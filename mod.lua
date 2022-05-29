local mod = {}
-- local g_counter = 0 -- global
-- g_counter = 0 -- error
_G.g_counter = 0 -- thread_local

function mod.getCounter()
    return g_counter
end

function mod.addCounter(arg)
    g_counter = g_counter + arg
end

return mod
