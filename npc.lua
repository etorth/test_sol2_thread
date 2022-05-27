--, u8R"###(
--

function npc_main(uid)
    do
        local thId, inMainThread = coroutine.running()
        -- if inMainThread then
        --     error('npc_main() called in main thread', uid)
        -- end
        print(uid, thId, inMainThread)
    end

    coroutine.yield()

    do
        local thId, inMainThread = coroutine.running()
        -- if inMainThread then
        --     error('npc_main() called in main thread', uid)
        -- end
        print(uid, thId, inMainThread)
    end
end

--
-- )###"
