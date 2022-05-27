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
