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

    // idea from: https://blog.rubenwardy.com/2020/07/26/sol3-script-sandbox/
    // important hack to set sandbox_env as default environment
    //
    // otherwise I didn't know how to make LuaThreadRunner call with sandbox env
    // if we let npc_main call back to C, I get error: attempt to yield from outside a coroutine

    lua_rawgeti(lua.lua_state(), LUA_REGISTRYINDEX, sandbox_env.registry_index());
    lua_rawseti(lua.lua_state(), LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);

    lua.script(INCLUA_BEGIN(char)
#include "npc.lua"
    INCLUA_END());

    LuaThreadRunner runner1(lua, "npc_main");
    runner1.co_callback(1);

    LuaThreadRunner runner2(lua, "npc_main");
    runner2.co_callback(2);

    while(runner1.co_callback){
        const auto result1 = runner1.co_callback();
        check_error(result1);

        const auto result2 = runner2.co_callback();
        check_error(result2);
    }
    return 0;
}
