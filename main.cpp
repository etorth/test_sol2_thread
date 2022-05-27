#include<bits/stdc++.h>
#include "sol/sol.hpp"

void checkError(const sol::protected_function_result &pfr)
{
    if(pfr.valid()){
        return;
    }

    const sol::error err = pfr;
    std::stringstream errStream(err.what());

    std::string errLine;
    while(std::getline(errStream, errLine, '\n')){
        std::cout << "callback error: " << errLine << std::endl;
    }
}

struct LuaThreadRunner
{
    sol::thread runner;
    sol::coroutine callback;

    LuaThreadRunner(sol::state &lua, const std::string &entry)
        : runner(sol::thread::create(lua.lua_state()))
        , callback(sol::state_view(runner.state())[entry])
    {}
};

int main()
{
    sol::state lua;
    lua.open_libraries();

    lua.script_file("luamodule_tlscfg.lua");
    sol::environment sandbox_env(lua, sol::create);
    sandbox_env[sol::metatable_key] = lua.create_table();
    sol::table sandbox_env_metatable = sandbox_env[sol::metatable_key];

    sandbox_env_metatable["__index"] = sol::function(lua["sandBox_meta_index"]);
    sandbox_env_metatable["__newindex"] = sol::function(lua["sandBox_meta_newindex"]);

    // idea from: https://blog.rubenwardy.com/2020/07/26/sol3-script-sandbox/
    // important hack to set sandbox_env as default environment
    //
    // otherwise I didn't know how to make LuaThreadRunner call with sandbox env
    // if we let npc_main call back to C, I get error: attempt to yield from outside a coroutine

    lua_rawgeti(lua.lua_state(), LUA_REGISTRYINDEX, sandbox_env.registry_index());
    lua_rawseti(lua.lua_state(), LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);

    lua.script_file("npc.lua");

    LuaThreadRunner runner1(lua, "npc_main");
    checkError(runner1.callback(1));

    LuaThreadRunner runner2(lua, "npc_main");
    checkError(runner2.callback(2));

    while(runner1.callback || runner2.callback){
        if(runner1.callback) checkError(runner1.callback());
        if(runner2.callback) checkError(runner2.callback());
    }
    return 0;
}
