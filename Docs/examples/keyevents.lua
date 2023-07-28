Game:Connect('keydown', function(key)
    print("Key " .. key .. " held down")
end)

Game:Connect('keyup', function(key)
    print("Key " .. key .. " let go")
end)
