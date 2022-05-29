local mod = {}
local g_counter = 0

function mod.getCounter()
    return g_counter
end

function mod.addCounter(arg)
    g_counter = g_counter + arg
end

return mod
