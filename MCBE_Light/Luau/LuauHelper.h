// quickly define these before everything else to avoid undefined errors
// these r main things
namespace LuauHelper {
    // security stuff
    namespace Security {
        enum Identities {
            Anonymous = 0,        // default identity
            DefaultScript,        // default lightapi script identity
            SystemScript,         // init script identity
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

    // used to store extra information about a lua state/thread
    class ExtraInstance {
    public:
        LuauHelper::Security::Identities identity;

    public:
        ExtraInstance(LuauHelper::Security::Identities identity) {
			this->identity = identity;
		}
    };

    // timeout instance used for resumejob
    class TimeoutInstance {
    public:
        double end;
    };

    // timeout instance used for consoleinputjobs
    class InputInstance {
    public:
        lua_State* thread;
    };

    // context for scripts (lua states/threads)
    class ScriptContext {
    public:
        // lua states/threads with an identity
        std::map<lua_State*, ExtraInstance*> luauThreads;

        // lua states/threads that are yielding (resumed by resumejob)
        std::map<lua_State*, TimeoutInstance*> yieldThreads;

        // threads that want input (resumed by consoleinputjob)
        std::queue<InputInstance*> inputThreads;

        // singleton function
        static ScriptContext* Get() {
            static ScriptContext* instance = new ScriptContext();
            return instance;
        }

        // resume thread
        void ResumeThread(lua_State* thread) {
            // get script context
            ScriptContext* context = ScriptContext::Get();

            // erase from yield threads map
            context->yieldThreads.erase(thread);

            // resume thread execution
            lua_resume(thread, 0, 0);

            lua_pop(thread, 1);
        }

        // yield thread execution
        void YieldThread(lua_State* thread, double timeout) {
            // get script context
            Instances::ScriptContext* context = Instances::ScriptContext::Get();

            // make timeout instance
            Instances::TimeoutInstance* timeoutInst = new Instances::TimeoutInstance();

            // set timeout to current time plus timeout
            timeoutInst->end = LuauHelper::GetTime() + timeout;

            // tell script context that this thread is yielding by inserting into yield threads map
            context->yieldThreads.insert(std::pair<lua_State*, Instances::TimeoutInstance*>(thread, timeoutInst));
		}

        // used to yield for input
        void YieldForInput(lua_State* thread) {
            // get script context
            Instances::ScriptContext* context = Instances::ScriptContext::Get();

            // make input instance
            Instances::InputInstance* input = new Instances::InputInstance();
            input->thread = thread;

            // tell script context that this thread is yielding by inserting into input threads map
            context->inputThreads.push(input);
        }

        // assign extrainstance to luau state/thread
        void Set(lua_State* L, ExtraInstance* extra) {
            // insert extra instance pointer into luau threads map
			luauThreads.insert(std::pair<lua_State*, ExtraInstance*>(L, extra));
		}

        // get extrainstance from luau state/thread
        ExtraInstance* Get(lua_State* L) {
            // check if extra instance exists
            if (luauThreads[L] == nullptr) {
                // if not, create a new one with default identity
				luauThreads[L] = new ExtraInstance(LuauHelper::Security::DefaultScript);
			}

            // return extra instance pointer
            return luauThreads[L];
        }

        // used to check if a luau state/thread has permissions to do something
        static bool Requires(lua_State* thread, LuauHelper::Security::Identities identity) {
            // get script context
            ScriptContext* context = Get();

            // get extra instance from state/thread map
            ExtraInstance* extra = context->Get(thread);

            // check if identity is greater than or equal to required identity
            if (extra->identity >= identity)
                return true; // identity check passed

            return false; // failed identity check
        }
    };
}

// quickly define these before everything else to avoid undefined errors
// these r main things
namespace LuauHelper {
    // queued scripts queue
    std::queue<Instances::ScriptInstance> QueuedScripts{};
}

// funcs for global setup
#include "Luau/LuauFunc.h"
#include <functional>

namespace LuauHelper {
    // global lua state all scripts inherit from
    lua_State* GameState = luaL_newstate();

    // this should be called by any scripts that want an identity, else it'll be treated as DefaultScript
    inline int ExecuteLuau(const char* _source, const char* chunkname, Security::Identities identity) {
        // get lua state/thread
        lua_State* thread = GameState;

        // create new thread if not a system script
        if (identity != Security::Identities::SystemScript) {
            thread = lua_newthread(GameState);
        }

        // get script context
        Instances::ScriptContext* context = Instances::ScriptContext::Get();

        // assign identity extra to the thread
        context->Set(thread, new Instances::ExtraInstance(identity));

        // it should already be in a thread but it still errors when yielding so i'll do this hacky thing
        // TODO: find way to do wait method without directly modifying the lua scripts executed

        // create script stream
        std::stringstream scriptStream;

        // wrap script in coroutine if not system script
        if (identity != Security::Identities::SystemScript) {
            // cant yield unless on a new thread
            scriptStream << "coroutine.wrap(function() " << _source << " end)()";
        }
		else {
			scriptStream << _source; // no new threads if system script (cant call wait methods & no security)
		}

        // get stream result
        const char* source = scriptStream.str().c_str();

        // compile script to luau bytecode
        size_t bytecodeSize = 0;
        char* bytecode = luau_compile(source, strlen(source), NULL, &bytecodeSize);

        // load luau bytecode into the VM
        int result = luau_load(thread, chunkname, bytecode, bytecodeSize, 0);

        // free the bytecode
        free(bytecode);

        // check if there was an error
        if (result != 0) {
            // get error message
            const char* errorMsg = lua_tostring(thread, -1);

            // log error message
            LogMessage(MESSAGE_ERROR, errorMsg);

            // pop error message from stack
            lua_pop(thread, 1);
            return 1;
        }
        else {
            // try catch cuz i cause std::exceptions in my script functions (cuz luau does that too for some reason)
            try {
                // call the luau thread from loaded bytecode
                lua_call(thread, 0, 0);
            }
            catch (const std::exception& e) {
                // log error message
                LogMessage(MESSAGE_ERROR, e.what());
                return 1;
            }
        }

        return 0;
    }

    // lua function structure for quick assignment
    struct LuaFunction {
        const char* name;
        lua_CFunction func;
    };

    LuaFunction functions[] = {
        // default console stuff
        { "print", ScriptEnvrioment::env_print },
        { "warn", ScriptEnvrioment::env_warn },
        { "error", ScriptEnvrioment::env_error },
        { "info", ScriptEnvrioment::env_info }, // this is for systemscripts not for default scripts

        // identification
        { "identify", ScriptEnvrioment::env_identifyname },
        { "printidentity", ScriptEnvrioment::env_printidentity },
        { "version", ScriptEnvrioment::env_version },
        { "getidentity", ScriptEnvrioment::env_getidentity },

        // luau bytecode stuff
        { "createscript", ScriptEnvrioment::env_createscript },
        { "loadstring", ScriptEnvrioment::env_loadstring },

        // roblox mains
        { "wait", ScriptEnvrioment::env_wait },

        // filesystem
        { "readfile", ScriptEnvrioment::env_readfile },
        { "listfiles", ScriptEnvrioment::env_listfiles },
        { "writefile", ScriptEnvrioment::env_writefile },
        { "makefolder", ScriptEnvrioment::env_makefolder },
        { "appendfile", ScriptEnvrioment::env_appendfile },
        { "isfile", ScriptEnvrioment::env_isfile },
        { "isfolder", ScriptEnvrioment::env_isfolder },
        { "delfile", ScriptEnvrioment::env_delfile },
        { "delfolder", ScriptEnvrioment::env_delfile },
        { "dofile", ScriptEnvrioment::env_dofile },

        // console api
        { "rconsoleclear", ScriptEnvrioment::env_rconsoleclear },
        { "rconsolecreate", ScriptEnvrioment::env_rconsolecreate },
        { "rconsoledestroy", ScriptEnvrioment::env_rconsoledestroy },
        { "rconsoleprint", ScriptEnvrioment::env_rconsoleprint },
        { "rconsoleinput", ScriptEnvrioment::env_rconsoleinput },
        { "rconsoletitle", ScriptEnvrioment::env_rconsoletitle },

        // misc
        { "time", ScriptEnvrioment::env_time },
        { "setclipboard", ScriptEnvrioment::env_setclipboard },
    };

    inline void SetupEnvrionment() {
        luaL_openlibs(LuauHelper::GameState);

#pragma region Global Fields

        // setup shared environment between scripts (& game stuff)
        lua_newtable(LuauHelper::GameState);
        lua_setglobal(LuauHelper::GameState, "Game");

        // Enum
        lua_newtable(LuauHelper::GameState);
        lua_setglobal(LuauHelper::GameState, "Enum");

        // other
        lua_newtable(LuauHelper::GameState);
        lua_setglobal(LuauHelper::GameState, "_G");

#pragma endregion

        // get function count
        int numFunctions = sizeof(functions) / sizeof(functions[0]);

        // loop over functions then register them
        for (int i = 0; i < numFunctions; i++) {
            // get luafunction
            LuaFunction function = functions[i];

            // push then setglobal
            lua_pushcfunction(LuauHelper::GameState, function.func, function.name);
            lua_setglobal(LuauHelper::GameState, function.name);
        }

        /*
        
        /--- TODO ---\

        setfps - set fps
        getfps - get fps
        isactive - check if window is currently active or not

        mouse1click - fake mouse1btn keydown
        mouse1press - fake mouse1btn keypress
        mouse1release - fake mouse1btn keyup
        mouse2click - fake mouse2btn keydown
        mouse2press - fake mouse2btn keypress
        mouse2release - fake mouse2btn keyup
        mousemoveabs - move mouse to absolute position
        mousemoverel - move mouse to relative position
        mousescroll - scroll mouse wheel

        Drawing {
            new - create new drawing object
            isrenderobj - check if an instance is a drawing instance
            clearcache - clear all drawings from the screen (does not destroy them)
        }

		request - make a http request
        WebSocket{
		    connect - create new websocket
		    close - close websocket
            send - send data to websocket
            onmessage - hook websocket message event
            onclose - hook websocket close event
        }

        */

        LuauHelper::ExecuteLuau("-- define hookslist\nGame.HooksList = {};\n\n-- define game metatable\nlocal GameMetatable = { __index = Game };\n\n-- define games connect function used to hook events\n-- usage: Game:Connect('render', function(screen, renderctx) end)\nfunction Game:Connect(event, callback)\n	if type(callback) == 'function' then\n		if not Game.HooksList[event] then\n			Game.HooksList[event] = {}\n		end\n		table.insert(Game.HooksList[event], callback)\n	else \n		error('Invalid callback provided for event ' .. event)\n	end\nend\n\n-- game events enum\nEnum.GameEvent = {\n	String = {\n		[1] = 'update',\n		[2] = 'keydown',\n		[3] = 'keyup',\n		[4] = 'keypress'\n	},\n	Update = 1,\n	KeyDown = 2,\n	KeyUp = 3,\n	KeyPress = 4\n};\n\n-- used to trigger a set of hooks (if they exist)\nfunction Game:CallHooks(event, ...)\n	if type(event) == 'number' then\n		event = Enum.GameEvent.String[event];\n	end\n	\n    if Game.HooksList[event] then\n        for _, hook in ipairs(Game.HooksList[event]) do\n            hook(...)\n        end\n    end\nend\n\n-- no clue how roblox exactly does theirs\n-- im just gonna do coroutine wrap until i figure that out\nfunction spawn(func)\n	coroutine.wrap(func)() -- call it in a new wrap/thread\nend\n\n-- set game metatable then quit\nsetmetatable(Game, GameMetatable);",
            "InitScript", LuauHelper::Security::SystemScript);
    }
}
