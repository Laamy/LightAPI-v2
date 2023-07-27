#pragma once

#define LUA_QL(x)	"'" x "'"
#define LUA_QS		LUA_QL("%s")

// custom util used to cast luau errors for custom functions
LUALIB_API int luaU_error(lua_State* L, const char* fmt, ...) {
    va_list argp;
    va_start(argp, fmt);
    luaL_where(L, 1);
    lua_pushvfstring(L, fmt, argp);
    va_end(argp);
    lua_concat(L, 2);
    lua_error(L);

    return 0;
}

// stole from roblox source
static int load_aux(lua_State* L, int status) {
    if (status == 0)  /* OK? */
        return 1;
    else {
        lua_pushnil(L);
        lua_insert(L, -2);  /* put before error message */
        return 2;  /* return nil plus error message */
    }
}

// doprint function exported directly from lua source
int doPrint(lua_State* L, MessageType type)
{
    std::string message;

    int n = lua_gettop(L);  /* number of arguments */
    int i;

    lua_getglobal(L, "tostring");
    for (i = 1; i <= n; i++) {
        if (i > 1)
        {
            message += ' ';
        }

        lua_pushvalue(L, -1);  /* function to be called */
        lua_pushvalue(L, i);   /* value to print */
        lua_call(L, 1, 1);
        const char* s = lua_tostring(L, -1);  /* get result */
        if (s == NULL)
            return luaU_error(L, LUA_QL("tostring") " must return a string to " LUA_QL("print"));

        message += s;
        if (i > 1) fputs("\t", stdout);
        lua_pop(L, 1);  /* pop result */
    }

    LogMessage(type, message.c_str());

    return 0;
}

class ScriptEnvrioment {
public:
    static ScriptEnvrioment* Get() {
        static ScriptEnvrioment single;
        return &single;
    }

public:
    /// <summary>
    /// Print to the console for debugging
    /// </summary>
    static int env_print(lua_State* L)
    {
        return doPrint(L, MESSAGE_OUTPUT);
    }

    /// <summary>
    /// Warn to the console for things like deprecated functions or classes
    /// </summary>
    static int env_warn(lua_State* L)
    {
        return doPrint(L, MESSAGE_OUTPUT);
    }

    /// <summary>
    /// stuff for system scripts
    /// </summary>
    static int env_info(lua_State* L)
    {
        if (Instances::ScriptContext::Requires(L, LuauHelper::Security::SystemScript))
        {
            return doPrint(L, MESSAGE_ERROR);
        }

        return luaU_error(L, "Identity of SystemScript required to run this function");
    }

    /// <summary>
    /// Create a new script instance then queue it for execution
    /// </summary>
    static int env_createscript(lua_State* L)
    {
        int nargs = lua_gettop(L);

        if (nargs < 1)
        {
            return luaU_error(L, "atleast 1 argument required");
        }

        if (!lua_isstring(L, 1))
        {
            return luaU_error(L, "arguments must be strings");
        }

        const char* script = lua_tostring(L, 1);
        const char* chunkname = Instances::DefaultChunk.c_str();

        if (nargs > 1)
        {
            if (!lua_isstring(L, 2))
            {
                return luaU_error(L, "arguments must be strings");
            }

            chunkname = lua_tostring(L, 2);
        }

        LuauHelper::QueuedScripts.push(Instances::ScriptInstance(script, chunkname));

        return 0;
    }

    /// <summary>
    /// Cast luau errors
    /// </summary>
    static int env_error(lua_State* L)
    {
        int nargs = lua_gettop(L);

        if (nargs < 1)
        {
            return luaU_error(L, "luau error thrown");
        }

        if (!lua_isstring(L, 1))
        {
            return luaU_error(L, "luau error thrown");
        }

        const char* error = lua_tostring(L, 1);

        return luaU_error(L, error);
    }

    /// <summary>
    /// Taken the idea from roblox luau universal naming convention
    /// If you fork this and make your own luau implementation, please change this to include your API name
    /// </summary>
    static int env_identifyname(lua_State* L)
    {
        lua_pushstring(L, "LightAPI");

        return 1;
    }

    /// <summary>
    /// Print the current identity of the lua thread
    /// </summary>
    static int env_printidentity(lua_State* L)
    {
        Instances::ScriptContext* context = Instances::ScriptContext::Get();

        Instances::ExtraInstance* extra = context->Get(L);

        int identity = extra->identity;

        if (lua_gettop(L) > 0) {
            if (!lua_isstring(L, 1)) {
				return luaU_error(L, "arguments must be strings");
			}

            LogMessage(MESSAGE_OUTPUT, "%s %d", lua_tostring(L, 1), identity);
        }
        else {
            LogMessage(MESSAGE_OUTPUT, "Current identity is %d", identity);
        }

        return 0;
    }

    /// <summary>
    /// Wait a specified amount of time in seconds
    /// </summary>
    static int env_wait(lua_State* L)
    {
        // TODO: implement wait on render function
        double timeout = lua_tonumber(L, 1);

        //clean timeout
        if (timeout < 0) {
            timeout = 0; // 0 isn't true 0 its just next frame when it gets around to resuming threads
        }

        if (timeout > 0xFFFF){ // if its over 65535 seconds just set it to 0
            timeout = 0;
		}

        // get script context
        Instances::ScriptContext* context = Instances::ScriptContext::Get();

        Instances::TimeoutInstance* timeoutInst = new Instances::TimeoutInstance();
        timeoutInst->end = LuauHelper::GetTime() + timeout;

        // tell script context that this thread is yielding
        context->yieldThreads.insert(std::pair<lua_State*, Instances::TimeoutInstance*>(L, timeoutInst));

        // yield execution

        return lua_yield(L, 0);
    }

    /// <summary>
    /// Get the current time in seconds
    /// </summary>
    static int env_time(lua_State* L)
    {
        lua_pushnumber(L, LuauHelper::GetTime());

        return 1;
    }

    /// <summary>
    /// Load a string as a luau chunk
    /// </summary>
    static int env_loadstring(lua_State* L) 
    {
        size_t size;
        const char* script = luaL_checklstring(L, 1, &size);
        const char* chunkname = luaL_optstring(L, 2, script);

        size_t bytecodeSize;
        char* bytecode = luau_compile(script, strlen(script), NULL, &bytecodeSize);

        int status = luau_load(L, chunkname, bytecode, bytecodeSize, 0);

        if (status != 0) {
            const char* error = lua_tostring(L, -1);

            if (error) {
                luaU_error(L, error);
            }
        }

        //lua_call(L, 0, 0);

        // return loaded bytecode
        return 1;
    }

    /// <summary>
    /// Get the lightapi version
    /// </summary>
    static int env_version(lua_State* L)
    {
        lua_pushstring(L, "v0.1.1");

        return 1;
    }
};