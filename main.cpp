#include<bits/stdc++.h>
#include "sol/sol.hpp"

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

int main()
{
    sol::state st;
    st.open_libraries();

    bool g_isDone = false;
    st.set_function("isDone", [&g_isDone]() -> bool
    {
        return g_isDone;
    });

    st.script(u8R"###(
        -- tls_main function
        -- entry point for all lua coroutine

        function main(start)
            while not isDone()
            do
                print(start)
                start = start + 1
                coroutine.yield()
            end
        end
    )###");

    // st.script(u8R"###(
    //     -- begin tls setup
    //     -- from https://stackoverflow.com/a/24358483/1490269
    //     -- setup all variable-access in thread as implicitly thread-local
    //     --
    //     -- this chunk is required to get sourced in main thread and before any call to main(uid)
    //     -- ____g_tls_mainThreadId is always the main thread id
    //
    //     local _G, coroutine = _G, coroutine
    //     local ____g_tls_mainThreadId, ____g_tls_inMainThread = coroutine.running()
    //
    //     -- put functions used in __index and __newindex into upvalue
    //     -- otherwise lua search _ENV for them
    //
    //     local error = error
    //     local rawset = rawset
    //     local setmetatable = setmetatable
    //
    //     if not ____g_tls_inMainThread then
    //         error(string.format('setup tls outside main thread: %s', tostring(____g_tls_mainThreadId)))
    //     end
    //
    //     -- tls table list has default _ENV as _G for main thread
    //     -- this requires current script should not sourced inside any function
    //
    //     local ____g_tls_threadLocalTableList = setmetatable({[____g_tls_mainThreadId] = _G}, {__mode = "k"})
    //     local ____g_tls_threadLocalMetaTable = {}
    //
    //     function ____g_tls_threadLocalMetaTable:__index(k)
    //         local currThreadId, currInMainThread = coroutine.running()
    //         if currInMainThread then
    //             error(string.format('setup tls in main thread: %s', tostring(currThreadId)))
    //         end
    //
    //         local currThreadTable = ____g_tls_threadLocalTableList[currThreadId]
    //         if currThreadTable then
    //             if currThreadTable[k] == nil then
    //                 return _G[k]
    //             else
    //                 return currThreadTable[k]
    //             end
    //         else
    //             -- current thread doesn't have associated tls table allocated yet
    //             -- means there is no new variable allocated in the new thread, can allocate tls table here
    //             -- but current implement postpone the allocation and search _G
    //             return _G[k]
    //         end
    //     end
    //
    //     function ____g_tls_threadLocalMetaTable:__newindex(k, v)
    //         local currThreadId, currInMainThread = coroutine.running()
    //         if currInMainThread then
    //             error(string.format('setup tls in main thread: %s', tostring(currThreadId)))
    //         end
    //
    //         local currThreadTable = ____g_tls_threadLocalTableList[currThreadId]
    //         if not currThreadTable then
    //             currThreadTable = setmetatable({_G = _G}, {__index = _G, __newindex = function(currThreadTable, key, value)
    //                 -- when reaching here
    //                 -- the thread tls table doesn't include key
    //                 -- we check if _G includes it, if yes assin to _G.key, otherwise create new table entry in tls table
    //                 if _G[key] == nil then
    //                     rawset(currThreadTable, key, value)
    //                 else
    //                     _G[key] = value
    //                 end
    //             end})
    //
    //             ____g_tls_threadLocalTableList[currThreadId] = currThreadTable
    //         end
    //         currThreadTable[k] = v
    //     end
    //
    //     -- convenient access to thread local variables via the `____g_tls_TLENV` table:
    //     -- user can use ____g_tls_TLENV.var to explicitly declare a thread local varible and access it
    //     ____g_tls_TLENV = setmetatable({}, ____g_tls_threadLocalMetaTable)
    //
    //     -- change default lua env
    //     -- makes all variables implicitly as thread-local-vars
    //
    //     -- this npchartlsconfig.lua get sourced as a chunk, but the _ENV is derived from main thread
    //     -- so here chane it to ____g_tls_TLENV is good to replace _ENV for main(uid)
    //     -- check this blog how upvalue/local/global works: https://luyuhuang.tech/2020/03/20/lua53-environment.html
    //     --
    //     -- TODO wired bug
    //     -- for lua interpreter the _ENV switching here works
    //     -- but with sol2 if switch _ENV here, function like print() is disabled and get C stack overflow
    //     --
    //     -- disable the _ENV switch here
    //     -- do _ENV switch in the first line of main(uid)
    //     -- _ENV = ____g_tls_TLENV
    //
    //     -- end tls setup
    //     -- following function variable access always goes into tls first
    // )###");

    sol::thread co_thread(sol::thread::create(st.lua_state()));
    sol::coroutine co_callback(sol::state_view(co_thread.state())["main"]);

    check_error(co_callback(12));
    if(!co_callback){
        throw std::runtime_error("coroutine exits unexpectedly");
    }

    check_error(co_callback());
    if(!co_callback){
        throw std::runtime_error("coroutine exits unexpectedly");
    }

    g_isDone = true;
    check_error(co_callback());
    if(co_callback){
        throw std::runtime_error("coroutine doesn't finish as expected");
    }

    return 0;
}
