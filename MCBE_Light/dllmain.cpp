#pragma region Includes

#include <windows.h>
#include <vector>
#include <string>
#include <Psapi.h>
#include <sstream>
#include <thread>
#include <queue>
#include <map>
#include <iostream>

#include <lua.h>
#include <lualib.h>
#include <luacode.h>

#include "Libs/xorstr.h"
#include "Libs/minhook/minhook.h"

#include "Logger.h"

#include "FileIO.h"

#include "SDK/MemoryUtils.h"
#include "SDK/Game.h"

#include "Luau/LuauHelper.h"

#include "SDK/Job/JobBase.h"

#include "SDK/Hook/FuncHook.h"

#pragma endregion

FILE* __f;

void InitClient() {
    // setup debug console/output
    if (!GetConsoleWindow())
    {
        AllocConsole();

        freopen_s(&__f, "CONOUT$", "w", stdout);
        freopen_s(&__f, "CONIN$", "r", stdin);
        SetConsoleTitleA("Light");
    }

    // client path setup
    if (FileIO::setupClientPath()) {
        FileIO::createPath(xorstr_("AutoExecute"));
        FileIO::writeFile(xorstr_("script.tmp"), "print\"Setup script pipe\"");
    }

    // setup game hooks
    InitHooks();

    // setup game state
    LuauHelper::SetupEnvrionment();

    // main loop
    while (true) {
        // get contents of script.tmp
        std::string script = FileIO::readFile(xorstr_("script.tmp"));

        // if script is empty, return
        if (script != "")
        {
            // clear script.tmp
            FileIO::deletePath(xorstr_("script.tmp"));

            // create script.tmp again
            FileIO::writeFile(xorstr_("script.tmp"), "");

            // push script to game script queue
            // this is for later when i get the renderer working to avoid multiple scripts
            // being executed at once (which is extremely laggy & overall gay)
            LuauHelper::QueuedScripts.push(Instances::ScriptInstance(script));
        }
    }
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH) {

        // stop library from being injected more then once
        DisableThreadLibraryCalls(module);

        // initialize main C++ thread
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)InitClient, module, 0, 0);
    }

    return TRUE;
}