// quickly define these before everything else to avoid undefined errors
// these r main things
namespace LuauHelper {
    // security stuff
    namespace Security {
        enum Identities {
            Anonymous = 0,        // default identity
            DefaultScript,        // default lightapi script identity
            SystemScript,           // init script identity
            COUNT_Identities      // count of identities
        };
    }

    // get current time in seconds (lua util)
    double GetTime() {
        auto currentTime = std::chrono::high_resolution_clock::now();
        double seconds = std::chrono::duration<double>(currentTime.time_since_epoch()).count();

        return seconds;
    }
}

// game stuff
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
        lua_State* thread = GameState;

        if (identity != Security::Identities::SystemScript) {
            thread = lua_newthread(GameState); // create new thread if not a system script
        }

        Instances::ScriptContext* context = Instances::ScriptContext::Get();

        // assign identity extra to the thread
        context->Set(thread, new Instances::ExtraInstance(identity));

        // it should already be in a thread but it still errors when yielding so i'll do this hacky thing
        // TODO: find way to do wait method without directly modifying the lua scripts executed

        std::stringstream scriptStream;
        if (identity != Security::Identities::SystemScript) {
            // cant yield unless on a new thread
            scriptStream << "coroutine.wrap(function() " << _source << " end)()";
        }
		else {
			scriptStream << _source; // no new threads if system script (cant call wait methods & no security)
		}
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

        // Enum
        lua_newtable(LuauHelper::GameState);
        lua_setglobal(LuauHelper::GameState, "Enum");

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

        find more things to add idk i did my whole todo in 2 hours when it was meant to last 3 days bruh

        */

        LuauHelper::ExecuteLuau("-- define hookslist\nGame.HooksList = {};\n\n-- define game metatable\nlocal GameMetatable = { __index = Game };\n\n-- define games connect function used to hook events\n-- usage: Game:Connect('render', function(screen, renderctx) end)\nfunction Game:Connect(event, callback)\n	if type(callback) == 'function' then\n		if not Game.HooksList[event] then\n			Game.HooksList[event] = {}\n		end\n		table.insert(Game.HooksList[event], callback)\n	else \n		error('Invalid callback provided for event ' .. event)\n	end\nend\n\n-- game events enum\nEnum.GameEvent = {\n	String = {\n		[1] = 'update',\n		[2] = 'keydown',\n		[3] = 'keyup',\n		[4] = 'keypress'\n	},\n	Update = 1,\n	KeyDown = 2,\n	KeyUp = 3,\n	KeyPress = 4\n};\n\n-- used to trigger a set of hooks (if they exist)\nfunction Game:CallHooks(event, ...)\n	if type(event) == 'number' then\n		event = Enum.GameEvent.String[event];\n	end\n	\n    if Game.HooksList[event] then\n        for _, hook in ipairs(Game.HooksList[event]) do\n            hook(...)\n        end\n    end\nend\n\n-- no clue how roblox exactly does theirs\n-- im just gonna do coroutine wrap until i figure that out\nfunction spawn(func)\n	coroutine.wrap(func)() -- call it in a new wrap/thread\nend\n\n-- set game metatable then quit\nsetmetatable(Game, GameMetatable);",
            "InitScript", LuauHelper::Security::SystemScript);
    }
}
