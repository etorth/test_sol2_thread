--, u8R"###(
--

-- function co_main(start)
--     print(start)
--     coroutine.yield()
--
--     start = start + 1
--     print(start)
-- end
--
-- -- begin tls setup
-- -- from https://stackoverflow.com/a/24358483/1490269
-- -- setup all variable-access in thread as implicitly thread-local
--
-- local _G, coroutine = _G, coroutine
-- local ____g_tls_mainThreadId, ____g_tls_inMainThread = coroutine.running()
--
-- local error = error
-- local rawset = rawset
-- local setmetatable = setmetatable
--
-- if not ____g_tls_inMainThread then
--     error(string.format('setup tls outside main thread: %s', tostring(____g_tls_mainThreadId)))
-- end
--
-- local ____g_tls_threadLocalTableList = setmetatable({[____g_tls_mainThreadId] = _G}, {__mode = "k"})
-- local ____g_tls_threadLocalMetaTable = {}
--
-- function ____g_tls_threadLocalMetaTable:__index(k)
--     local currThreadId, currInMainThread = coroutine.running()
--     if currInMainThread then
--         error(string.format('setup tls in main thread: %s', tostring(currThreadId)))
--     end
--
--     local currThreadTable = ____g_tls_threadLocalTableList[currThreadId]
--     if currThreadTable then
--         if currThreadTable[k] == nil then
--             return _G[k]
--         else
--             return currThreadTable[k]
--         end
--     else
--         return _G[k]
--     end
-- end
--
-- function ____g_tls_threadLocalMetaTable:__newindex(k, v)
--     local currThreadId, currInMainThread = coroutine.running()
--     if currInMainThread then
--         error(string.format('setup tls in main thread: %s', tostring(currThreadId)))
--     end
--
--     local currThreadTable = ____g_tls_threadLocalTableList[currThreadId]
--     if not currThreadTable then
--         currThreadTable = setmetatable({_G = _G}, {__index = _G, __newindex = function(currThreadTable, key, value)
--             if _G[key] == nil then
--                 rawset(currThreadTable, key, value)
--             else
--                 _G[key] = value
--             end
--         end})
--
--         ____g_tls_threadLocalTableList[currThreadId] = currThreadTable
--     end
--     currThreadTable[k] = v
-- end
--
-- ____g_tls_TLENV = setmetatable({}, ____g_tls_threadLocalMetaTable)
-- _ENV = ____g_tls_TLENV
--
-- -- BUG here
-- -- all function call after this line failed with stack overflow with sol2
-- print('----: _ENV switched after this line')
--
-- local co_handler = coroutine.create(co_main, 12)
--
-- coroutine.resume(co_handler)
-- coroutine.resume(co_handler)

local _G = _G
local error = error
local coroutine = coroutine

local g_tlsTable = {}
function meta_index(table, key)
    local threadId, inMainThread = coroutine.running()
    if not inMainThread then
        if g_tlsTable[threadId] ~= nil and g_tlsTable[threadId][key] ~= nil then
            return g_tlsTable[threadId][key]
        end
    end
    return _G[key]
end

local rawset = rawset
function meta_newindex(table, key, value)
    local threadId, inMainThread = coroutine.running()
    if inMainThread then
        rawset(table, key, value)
    else
        if g_tlsTable[threadId] == nil then
            g_tlsTable[threadId] = {}
        end
        g_tlsTable[threadId][key] = value
    end
end

function clearTLSTable()
    local threadId, inMainThread = coroutine.running()
    if inMainThread then
        error('call clearTLSTable in main thread')
    else
        g_tlsTable[threadId] = nil
    end
end

--
-- )###"
