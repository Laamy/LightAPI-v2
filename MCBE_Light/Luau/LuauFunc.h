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

// helper functions for calling game hooks
template<typename T>
void PushArgument(lua_State* L, T arg) {
    lua_pushnil(L);
}

template<>
void PushArgument(lua_State* L, const char* arg) {
    lua_pushstring(L, arg);
}

template<>
void PushArgument(lua_State* L, int arg) {
    lua_pushinteger(L, arg);
}

// function used in C++ to call the callhooks method so we can trigger game events
template<typename... Args>
void CallGameCallHooks(lua_State* L, Game::GameEvent event, Args... args) {
    lua_getglobal(L, "Game");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1); // Pop the nil value from the stack
        return;
    }

    lua_getfield(L, -1, "CallHooks");
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 2);
        return;
    }

    lua_pushinteger(L, (int)event);

    int numArgs = sizeof...(Args);
    (PushArgument(L, args), ...);

    lua_call(L, numArgs + 1, 0);

    lua_pop(L, 1);
}

// push a table of strings to the stack
static void pushStringVector(lua_State* L, const std::vector<std::string>& vec) {
    lua_newtable(L);
    for (size_t i = 0; i < vec.size(); ++i) {
        lua_pushstring(L, vec[i].c_str());
        lua_rawseti(L, -2, static_cast<int>(i + 1));
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
        return doPrint(L, MESSAGE_WARNING);
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
        // get arguments count
        int nargs = lua_gettop(L);

        //if less then 1 argument error
        if (nargs < 1)
        {
            return luaU_error(L, "atleast 1 argument required");
        }

        // if the first argument is not a string error
        if (!lua_isstring(L, 1))
        {
            return luaU_error(L, "arguments must be strings");
        }

        // get script and default chunkname
        const char* script = lua_tostring(L, 1);
        const char* chunkname = Instances::DefaultChunk.c_str();

        // if argumets more then 1 then use the chunkname provided
        if (nargs > 1)
        {
            if (!lua_isstring(L, 2))
            {
                return luaU_error(L, "arguments must be strings");
            }

            chunkname = lua_tostring(L, 2);
        }

        // queue script for execution
        LuauHelper::QueuedScripts.push(Instances::ScriptInstance(script, chunkname));

        return 0;
    }

    /// <summary>
    /// Cast luau errors
    /// </summary>
    static int env_error(lua_State* L)
    {
        // get arguments count
        int nargs = lua_gettop(L);

        // if less then 1 argument error
        if (nargs < 1)
        {
            return luaU_error(L, "luau error thrown");
        }

        // if the first argument is not a string error
        if (!lua_isstring(L, 1))
        {
            return luaU_error(L, "luau error thrown");
        }

        // get error
        const char* error = lua_tostring(L, 1);

        // throw error
        return luaU_error(L, error);
    }

    /// <summary>
    /// Taken the idea from roblox luau universal naming convention
    /// If you fork this and make your own luau implementation, please change this to include your API name
    /// </summary>
    static int env_identifyname(lua_State* L)
    {
        // return the name of the api
        lua_pushstring(L, "LightAPI");

        return 1;
    }

    /// <summary>
    /// Print the current identity of the lua thread
    /// </summary>
    static int env_printidentity(lua_State* L)
    {
        // get the current script context
        Instances::ScriptContext* context = Instances::ScriptContext::Get();

        // get the extra instance
        Instances::ExtraInstance* extra = context->Get(L);

        // get the identity
        int identity = extra->identity;

        // if there is an argument then print the argument instead
        if (lua_gettop(L) > 0) {

            // if the first argument is not a string error
            if (!lua_isstring(L, 1)) {
				return luaU_error(L, "arguments must be strings");
			}

            // print the argument
            LogMessage(MESSAGE_OUTPUT, "%s %d", lua_tostring(L, 1), identity);
        }
        else {
            // print the identity normally
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

        // if its over 65535 seconds just set it to 0
        if (timeout > 0xFFFF){
            return luaU_error(L, "max wait is 65535");
		}

        // get script context
        Instances::ScriptContext* context = Instances::ScriptContext::Get();

        // yield thread via script context
        context->YieldThread(L, timeout);

        // yield execution

        return lua_yield(L, 0);
    }

    /// <summary>
    /// Get the current time in seconds
    /// </summary>
    static int env_time(lua_State* L)
    {
        // push the current time in seconds (as we want lua to use seconds not ms)
        lua_pushnumber(L, LuauHelper::GetTime());

        return 1;
    }

    /// <summary>
    /// Load a string as a luau chunk
    /// </summary>
    static int env_loadstring(lua_State* L) 
    {
        // get script & chunkname as chunks
        size_t size;
        const char* script = luaL_checklstring(L, 1, &size);
        const char* chunkname = luaL_optstring(L, 2, script);

        // compile string to luau bytecode
        size_t bytecodeSize;
        char* bytecode = luau_compile(script, strlen(script), NULL, &bytecodeSize);

        // load bytecode then check status
        int status = luau_load(L, chunkname, bytecode, bytecodeSize, 0);

        if (status != 0) {
            const char* error = lua_tostring(L, -1);

            // if failed & error exists cast new luau error so parent thread can catch it
            if (error) {
                luaU_error(L, error);
            }
        }

        // we would call it but might aswell give the bytecode as the result (luau function)
        // so that they can call it when they want 2
        //lua_call(L, 0, 0);

        // return loaded bytecode
        return 1;
    }

    /// <summary>
    /// Get the lightapi version
    /// </summary>
    static int env_version(lua_State* L)
    {
        // return the version of the api
        lua_pushstring(L, "v0.1.1");

        // return the version string
        return 1;
    }

    /// <summary>
    /// return the current identity of the lua thread
    /// </summary>
    static int env_getidentity(lua_State* L)
    {
        // get the current script context
        Instances::ScriptContext* context = Instances::ScriptContext::Get();

        // get the extra instance
        Instances::ExtraInstance* extra = context->Get(L);

        // get the identity
        int identity = extra->identity;

        lua_pushinteger(L, identity);

        // return the version string
        return 1;
    }

    /// <summary>
    /// Get the list of objects in the garbage collector
    /// </summary>
    static int env_readfile(lua_State* L)
    {
        int nargs = lua_gettop(L);

        if (nargs < 1) {
            return luaU_error(L, "expected atleast 1 argument");
        }

        if (!lua_isstring(L, 1)) {
            return luaU_error(L, "expected string");
        }

        std::stringstream ss;
        ss << "workspace\\" << lua_tostring(L, 1);

        std::string content = FileIO::readFile(ss.str());

        lua_pushstring(L, content.c_str());

        return 1;
    }

    /// <summary>
    /// Get a list of files in a folder
    /// </summary>
    static int env_listfiles(lua_State* L)
    {
        int nargs = lua_gettop(L);

        std::string folder;
        std::string client = FileIO::getClientPath();

        if (nargs < 1) {
            return luaU_error(L, "expected atleast 1 argument");
        }

        if (!lua_isstring(L, 1)) {
            return luaU_error(L, "expected string");
        }

        std::stringstream ss;
        ss << client << "workspace\\" << std::string(lua_tostring(L, 1));

        folder = ss.str();

        if (!std::filesystem::is_directory(folder.c_str())) {
            return luaU_error(L, "directory not found '%s'", folder.c_str());
        }

        std::vector<std::string> file_list;

        for (const auto& entry : std::filesystem::directory_iterator(folder.c_str())) {
            if (entry.is_regular_file()) {
                file_list.push_back(entry.path().filename().string());
            }
        }

        pushStringVector(L, file_list);

        return 1;
    }

    /// <summary>
    /// write to a file (and create it if it doesnt exist)
    /// </summary>
    static int env_writefile(lua_State* L)
    {
        int nargs = lua_gettop(L);

        if (nargs < 2) {
            return luaU_error(L, "expected atleast 2 argument");
        }

        if (!lua_isstring(L, 1) || !lua_isstring(L, 2)) {
            return luaU_error(L, "expected string");
        }

        std::stringstream ss;
        ss << "workspace\\" << lua_tostring(L, 1);

        FileIO::writeFile(ss.str(), lua_tostring(L, 2));

        return 0;
    }

    /// <summary>
    /// Create a folder
    /// </summary>
    static int env_makefolder(lua_State* L)
    {
        int nargs = lua_gettop(L);

        if (nargs < 1) {
            return luaU_error(L, "expected atleast 1 argument");
        }

        if (!lua_isstring(L, 1)) {
            return luaU_error(L, "expected string");
        }

        std::stringstream ss;
        ss << "workspace\\" << lua_tostring(L, 1);

        FileIO::createPath(ss.str());

        return 0;
    }
};