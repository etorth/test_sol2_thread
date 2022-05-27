#include<bits/stdc++.h>
#include "sol/sol.hpp"

#define INCLUA_BEGIN(script_byte_type) \
[]() \
{ \
    using _INCLUA_BYTE_TYPE = script_byte_type; \
    const char *_dummy_cstr = u8"?"; \
\
    const char *_consume_decr_op = _dummy_cstr + 1; \
    const char *_use_second_cstr[] \
    { \
        _consume_decr_op

#define INCLUA_END() \
    }; \
    return (const _INCLUA_BYTE_TYPE *)(_use_second_cstr[1]); \
}()

void check_error(const sol::protected_function_result &result)
{
    if(result.valid()){
        return;
    }

    const sol::error err = result;
    std::stringstream errStream(err.what());

    std::string errLine;
    while(std::getline(errStream, errLine, '\n')){
        std::cout << "co_callback error: " << errLine << std::endl;
    }
}

struct LuaThreadRunner
{
    sol::thread co_runner;
    sol::coroutine co_callback;

    LuaThreadRunner(sol::state &lua, std::string entry)
        : co_runner(sol::thread::create(lua.lua_state()))
        , co_callback(sol::state_view(co_runner.state())[entry])
    {}
};

int main()
{
    sol::state lua;
    lua.open_libraries();

    lua.script(INCLUA_BEGIN(char)
#include "luamodule_tlscfg.lua"
    INCLUA_END());

    sol::environment sandbox_env(lua, sol::create);
    sandbox_env[sol::metatable_key] = lua.create_table();
    sol::table sandbox_env_metatable = sandbox_env[sol::metatable_key];

    sandbox_env_metatable["__index"] = sol::function(lua["meta_index"]);
    sandbox_env_metatable["__newindex"] = sol::function(lua["meta_newindex"]);

    lua.script(INCLUA_BEGIN(char)
#include "npc.lua"
    INCLUA_END(), sandbox_env);

    lua["co_main"] = [&lua, &sandbox_env](sol::object arg)
    {
        char script_buf[2048];
        std::sprintf(script_buf, "npc_main(%d)", arg.as<int>());
        lua.script(script_buf, sandbox_env);
    };

    LuaThreadRunner runner(lua, "co_main");
    const auto result = runner.co_callback(120);
    check_error(result);
    return 0;
}
