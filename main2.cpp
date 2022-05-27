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

    lua.script(R"(
        local _G = _G
        local error = error
        local rawset = rawset
        local coroutine = coroutine

        local _G_sandbox = {}
        function sandBox_meta_index(table, key)
            local threadId, inMainThread = coroutine.running()
            if not inMainThread then
                if _G_sandbox[threadId] ~= nil and _G_sandbox[threadId][key] ~= nil then
                    return _G_sandbox[threadId][key]
                end
            end
            return _G[key]
        end

        function sandBox_meta_newindex(table, key, value)
            local threadId, inMainThread = coroutine.running()
            if inMainThread then
                rawset(table, key, value)
            else
                if _G_sandbox[threadId] == nil then
                    _G_sandbox[threadId] = {}
                end
                _G_sandbox[threadId][key] = value
            end
        end

        function clearTLSTable()
            local threadId, inMainThread = coroutine.running()
            if inMainThread then
                error('call clearTLSTable in main thread')
            else
                _G_sandbox[threadId] = nil
            end
        end
    )");

    sol::environment hack_env(lua, sol::create);
    hack_env[sol::metatable_key] = lua.create_table();
    sol::table hack_env_metatable = hack_env[sol::metatable_key];

    hack_env_metatable["__index"] = sol::function(lua["sandBox_meta_index"]);
    hack_env_metatable["__newindex"] = sol::function(lua["sandBox_meta_newindex"]);

    // idea from: https://blog.rubenwardy.com/2020/07/26/sol3-script-sandbox/
    // set hack_env as default environment, otherwise I don't know how to setup hack_env to thread/coroutine

    lua_rawgeti(lua.lua_state(), LUA_REGISTRYINDEX, hack_env.registry_index());
    lua_rawseti(lua.lua_state(), LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);

    lua.script(R"(
        function coth_main(runner)
            local threadId, mainThread = coroutine.running()
            if mainThread then
                error('coth_main(runner) called in main thread', runner)
            end

            coroutine.yield()

            counter = 0     -- localized global varible tested
            counterMax = 10 --

            while counter < counterMax do
                print(string.format('runner %d counter %d, you can resume %d more times', runner, counter, counterMax - counter - 1))
                counter = counter + 1
                coroutine.yield()
            end

            clearTLSTable()
        end
    )");

    LuaThreadRunner runner1(lua, "coth_main");
    checkError(runner1.callback(1));

    LuaThreadRunner runner2(lua, "coth_main");
    checkError(runner2.callback(2));

    const auto fnResume = [](auto &callback, int index)
    {
        if(callback){
            checkError(callback());
        }
        else{
            std::cout << "runner " << index << " has existed" << std::endl;
        }
    };

    while(runner1.callback || runner2.callback){
        int event_from = 0;
        std::cout << "wait event from: ";
        std::cin >> event_from;

        switch(event_from){
            case 1: fnResume(runner1.callback, 1); break;
            case 2: fnResume(runner2.callback, 2); break;
            default: std::cout << "no runner " << event_from << std::endl;
        }
    }
    return 0;
}