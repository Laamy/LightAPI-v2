-- hook keydown game event
Game:Connect('keydown', function(key)
    -- print key that triggered keydown event
    print("Key " .. key .. " held down")
end)

-- hook keyup game event
Game:Connect('keyup', function(key)
    -- print key that triggered keyup event
    print("Key " .. key .. " let go")
end)
