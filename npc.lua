--, u8R"###(
--

function npc_main(uid)
    local threadId, mainThread = coroutine.running()
    if mainThread then
        error('npc_main() called in main thread', uid)
    end

    if uid == 1 then
        counter = 0
        counterMax = 10
    else
        counter = 100
        counterMax = 110
    end

    while counter < counterMax do
        print(uid, counter)
        counter = counter + 1
        coroutine.yield()
    end

    if tab == nil then
        tab = {}
        tab[1] = uid
    end

    print(tab)
    clearTLSTable()
end

--
-- )###"
