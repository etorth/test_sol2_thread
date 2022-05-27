--, u8R"###(
--

function npc_main(uid)
    local threadId, mainThread = coroutine.running()
    if mainThread then
        error('npc_main() called in main thread', uid)
    end

    local counter = 0
    while counter < 10 do
        print(counter)
        counter = counter + 1
        coroutine.yield()
    end
end

--
-- )###"
