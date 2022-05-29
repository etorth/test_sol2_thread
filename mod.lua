local mod = {}

-- g_counter is global to all threads when applies local modifier
-- because it is upvalue to module functions
-- local g_counter = 0


-- error if use raw global variable
-- require(path) loads module and saves into package.path, means require() only load this file once
--
-- if we call require(path) in thread, g_counter get allocated as tls for that thread
-- this means if another thread calls module function in this file, accessing to it causes nil-access-error
-- g_counter = 0


-- right way to declare thread local variables
-- this line explicitly creates an global variable in default _G, because __index('_G') first find _G in _G, which succeeds
--
-- later if we read-access g_counter, it redirects to _G.g_counter
-- but when we write-access g_counter, we create a copy of g_counter to that thread, so for following line:
--
--     g_counter = g_counter + 1
--
-- when this line first time get executed, g_counter at right side of assignment operator actually reads _G.g_counter
-- and g_counter at left side of assignment operator creates a copy in that thread's TLS table
--
-- then all rest execution of this line always uses g_counter in that thread's TLS table
-- this make the code works in a copy-on-right way, to access the g_counter in _G, use explicit _G.g_counter
_G.g_counter = 0

function mod.getCounter()
    return g_counter
end

function mod.addCounter(arg)
    g_counter = g_counter + arg
end

return mod
