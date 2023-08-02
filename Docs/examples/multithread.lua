-- spawn this thread then continue without waiting for execution to finish
spawn(function()
    print("Hello!")
    wait(0.1)
    print("Hello 2!")
end)

-- spawn this thread then continue without waiting for execution to finish
spawn(function()
    print("Hello!")
    wait(0.1)
    print("Hello 2!")
end)

-- results:
--Hello!
--Hello!
--Hello 2!
--Hello 2!
