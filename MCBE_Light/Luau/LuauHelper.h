// quickly define these before everything else to avoid undefined errors
// these r main things
namespace LuauHelper {
    namespace Security {
        enum Identities {
            Anonymous = 0,        // default identity
            DefaultScript,        // default lightapi script identity
            SystemScript,           // init script identity
            COUNT_Identities      // count of identities
        };
    }

    double GetTime() {
        auto currentTime = std::chrono::high_resolution_clock::now();
        double seconds = std::chrono::duration<double>(currentTime.time_since_epoch()).count();

        return seconds;
    }
}

namespace Instances {
    static std::string DefaultChunk = xorstr_("LightScript");

    class ScriptInstance {
    public:
        std::string source;
        std::string chunkname;

        ScriptInstance(std::string source, std::string chunkname) {
            this->source = source;
            this->chunkname = chunkname;
        }

        ScriptInstance(std::string source) {
            this->source = source;
            this->chunkname = Instances::DefaultChunk; // default script name is Instances::DefaultChunk
        }
    };

    class ExtraInstance {
    public:
        LuauHelper::Security::Identities identity;

    public:
        ExtraInstance(LuauHelper::Security::Identities identity) {
			this->identity = identity;
		}
    };

    class TimeoutInstance {
    public:
        double end;
    };

    class ScriptContext {
    public:
        std::map<lua_State*, ExtraInstance*> luauThreads;
        std::map<lua_State*, TimeoutInstance*> yieldThreads;

        static ScriptContext* Get() {
            static ScriptContext* instance = new ScriptContext();
            return instance;
        }

        void ResumeThread(lua_State* thread) {
            ScriptContext* context = ScriptContext::Get();

            context->yieldThreads.erase(thread);
            lua_resume(thread, 0, 0);
        }

        void YieldThread(lua_State* thread, double timeout) {
            Instances::ScriptContext* context = Instances::ScriptContext::Get();

            Instances::TimeoutInstance* timeoutInst = new Instances::TimeoutInstance();
            timeoutInst->end = LuauHelper::GetTime() + timeout;

            // tell script context that this thread is yielding
            context->yieldThreads.insert(std::pair<lua_State*, Instances::TimeoutInstance*>(thread, timeoutInst));
		}

        void Set(lua_State* L, ExtraInstance* extra) {
			luauThreads.insert(std::pair<lua_State*, ExtraInstance*>(L, extra));
		}

        ExtraInstance* Get(lua_State* L) {
            if (luauThreads[L] == nullptr) {
				luauThreads[L] = new ExtraInstance(LuauHelper::Security::DefaultScript);
			}

            return luauThreads[L];
        }

        static bool Requires(lua_State* thread, LuauHelper::Security::Identities identity) {
            ScriptContext* context = Get();

            ExtraInstance* extra = context->Get(thread);

            if (extra->identity >= identity)
                return true; // identity check passed

            return false; // failed identity check
        }
    };
}

// quickly define these before everything else to avoid undefined errors
// these r main things
namespace LuauHelper {
    std::queue<Instances::ScriptInstance> QueuedScripts{};
}

// funcs for global setup
#include "Luau/LuauFunc.h"
#include <functional>

namespace LuauHelper {
    lua_State* GameState = luaL_newstate();
    lua_State* SharedState = luaL_newstate();

    // this should be called by any scripts that want an identity, else it'll be treated as DefaultScript
    inline int ExecuteLuau(const char* _source, const char* chunkname, Security::Identities identity) {
        lua_State* thread = lua_newthread(GameState);

        Instances::ScriptContext* context = Instances::ScriptContext::Get();

        // assign identity extra to the thread
        context->Set(thread, new Instances::ExtraInstance(identity));

        // it should already be in a thread but it still errors when yielding so i'll do this hacky thing
        // TODO: find way to do wait method without directly modifying the lua scripts executed

        std::stringstream scriptStream;
        scriptStream << "coroutine.wrap(function() " << _source << " end)()";
        const char* source = scriptStream.str().c_str();

        size_t bytecodeSize = 0;
        char* bytecode = luau_compile(source, strlen(source), NULL, &bytecodeSize);

        int result = luau_load(thread, chunkname, bytecode, bytecodeSize, 0);

        free(bytecode);

        if (result != 0) {
            const char* errorMsg = lua_tostring(thread, -1);
            LogMessage(MESSAGE_ERROR, errorMsg);

            lua_pop(thread, 1);
            return 1;
        }
        else {
            try {
                lua_call(thread, 0, 0);
            }
            catch (const std::exception& e) {
                LogMessage(MESSAGE_ERROR, e.what());
                return 1;
            }
        }

        return 0;
    }

    inline void SetupEnvrionment() {
        luaL_openlibs(LuauHelper::GameState);

        // setup shared environment between scripts (& game stuff)
        lua_newtable(LuauHelper::GameState);
        lua_setglobal(LuauHelper::GameState, "Game");

        // other
        lua_newtable(LuauHelper::GameState);
        lua_setglobal(LuauHelper::GameState, "_G");
        lua_newtable(LuauHelper::GameState);
        lua_setglobal(LuauHelper::GameState, "shared");

        // define print
        lua_pushcfunction(LuauHelper::GameState, ScriptEnvrioment::env_print, "print");
        lua_setglobal(LuauHelper::GameState, "print");

        // define warn
        lua_pushcfunction(LuauHelper::GameState, ScriptEnvrioment::env_warn, "warn");
        lua_setglobal(LuauHelper::GameState, "warn");

        // define error
        lua_pushcfunction(LuauHelper::GameState, ScriptEnvrioment::env_error, "error");
        lua_setglobal(LuauHelper::GameState, "error");

        // define identity
        lua_pushcfunction(LuauHelper::GameState, ScriptEnvrioment::env_identifyname, "identify");
        lua_setglobal(LuauHelper::GameState, "identify");

        // define queuescript
        lua_pushcfunction(LuauHelper::GameState, ScriptEnvrioment::env_createscript, "createscript");
        lua_setglobal(LuauHelper::GameState, "createscript");

        // define time
        lua_pushcfunction(LuauHelper::GameState, ScriptEnvrioment::env_time, "time");
        lua_setglobal(LuauHelper::GameState, "time");

        // define printidentity
        lua_pushcfunction(LuauHelper::GameState, ScriptEnvrioment::env_printidentity, "printidentity");
        lua_setglobal(LuauHelper::GameState, "printidentity");

        // define loadstring
        lua_pushcfunction(LuauHelper::GameState, ScriptEnvrioment::env_loadstring, "loadstring");
        lua_setglobal(LuauHelper::GameState, "loadstring");

        // define version
        lua_pushcfunction(LuauHelper::GameState, ScriptEnvrioment::env_version, "version");
        lua_setglobal(LuauHelper::GameState, "version");

        // define wait
        lua_pushcfunction(LuauHelper::GameState, ScriptEnvrioment::env_wait, "wait");
        lua_setglobal(LuauHelper::GameState, "wait");

        // define info
        lua_pushcfunction(LuauHelper::GameState, ScriptEnvrioment::env_info, "info");
        lua_setglobal(LuauHelper::GameState, "info");

        /*

        /--- TODO ---\

		spawn function - spawn a new luau thread taking in a function as the new thread code

        */

        LuauHelper::ExecuteLuau("Game.HooksList = {};\n\nlocal GameMetatable = { __index = Game }\n\nfunction Game:Connect(event, callback)\n	local str = event:match('^%s*(.-)%s*$')\n	\n	if type(callback) == 'function' then\n		if not Game.HooksList[str] then\n			Game.HooksList[str] = {}\n		end\n		table.insert(Game.HooksList[str], callback)\n	else \n		error('Invalid callback provided for event ' .. str)\n	end\nend\n\nfunction Game:CallHooks(event, ...)\n	local str = event:match('^%s*(.-)%s*$')\n	\n    if Game.HooksList[str] then\n        for _, hook in ipairs(Game.HooksList[str]) do\n            hook(...)\n        end\n    end\nend\n\nsetmetatable(Game, GameMetatable);",
            "InitScript", LuauHelper::Security::SystemScript);
    }
}
