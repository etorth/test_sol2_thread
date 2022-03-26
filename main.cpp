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

    st.script(u8R"###(
        function co_main(start)
            print(start)
            coroutine.yield()

            start = start + 1
            print(start)
        end
    )###");

    st.script(u8R"###(
        -- begin tls setup
        -- from https://stackoverflow.com/a/24358483/1490269
        -- setup all variable-access in thread as implicitly thread-local

        local _G, coroutine = _G, coroutine
        local ____g_tls_mainThreadId, ____g_tls_inMainThread = coroutine.running()

        local error = error
        local rawset = rawset
        local setmetatable = setmetatable

        if not ____g_tls_inMainThread then
            error(string.format('setup tls outside main thread: %s', tostring(____g_tls_mainThreadId)))
        end

        local ____g_tls_threadLocalTableList = setmetatable({[____g_tls_mainThreadId] = _G}, {__mode = "k"})
        local ____g_tls_threadLocalMetaTable = {}

        function ____g_tls_threadLocalMetaTable:__index(k)
            local currThreadId, currInMainThread = coroutine.running()
            if currInMainThread then
                error(string.format('setup tls in main thread: %s', tostring(currThreadId)))
            end

            local currThreadTable = ____g_tls_threadLocalTableList[currThreadId]
            if currThreadTable then
                if currThreadTable[k] == nil then
                    return _G[k]
                else
                    return currThreadTable[k]
                end
            else
                return _G[k]
            end
        end

        function ____g_tls_threadLocalMetaTable:__newindex(k, v)
            local currThreadId, currInMainThread = coroutine.running()
            if currInMainThread then
                error(string.format('setup tls in main thread: %s', tostring(currThreadId)))
            end

            local currThreadTable = ____g_tls_threadLocalTableList[currThreadId]
            if not currThreadTable then
                currThreadTable = setmetatable({_G = _G}, {__index = _G, __newindex = function(currThreadTable, key, value)
                    if _G[key] == nil then
                        rawset(currThreadTable, key, value)
                    else
                        _G[key] = value
                    end
                end})

                ____g_tls_threadLocalTableList[currThreadId] = currThreadTable
            end
            currThreadTable[k] = v
        end

        ____g_tls_TLENV = setmetatable({}, ____g_tls_threadLocalMetaTable)
        _ENV = ____g_tls_TLENV

        -- BUG here
        -- all function call after this line failed with stack overflow with sol2
        print('----: _ENV switched after this line')
    )###");

    sol::thread co_thread(sol::thread::create(st.lua_state()));
    sol::coroutine co_handler(sol::state_view(co_thread.state())["co_main"]);

    check_error(co_handler(12));
    if(!co_handler){
        throw std::runtime_error("coroutine exits unexpectedly");
    }

    check_error(co_handler());
    if(co_handler){
        throw std::runtime_error("coroutine doesn't finish as expected");
    }

    return 0;
}
