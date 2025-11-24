-- src/Game/script/hello.lua

print("=== Lua Script Started ===")

-- -- 1. プレイヤーのパーツを探す
-- local player = workspace:FindFirstChild("Player")
-- if player then
--     print("✓ Found Player Model!")
--     print("  Player ClassName:", player.ClassName)
-- end

-- -- 2. HumanoidRootPart を探す
-- local rootPart = workspace:FindFirstChild("HumanoidRootPart")
-- if rootPart then
--     print("\n✓ Found HumanoidRootPart!")
--     print("  Name:", rootPart.Name)
--     local pos = rootPart.Position
--     if pos then
--         print("  Position:", pos.X, pos.Y, pos.Z)
--     end
-- end

-- -- 3. 全ての体パーツを探す
-- local bodyParts = {"Head", "Torso", "LeftArm", "RightArm", "LeftLeg", "RightLeg"}
-- print("\n=== Body Parts ===")
-- for _, partName in ipairs(bodyParts) do
--     local part = workspace:FindFirstChild(partName)
--     if part then
--         print("✓", partName, "found at", part.Position.X, part.Position.Y, part.Position.Z)
--     else
--         print("✗", partName, "not found")
--     end
-- end

-- -- 4. workspace の全ての子を表示
-- print("\n=== All Children in Workspace ===")
-- local allChildren = workspace:GetChildren()
-- for i, child in ipairs(allChildren) do
--     if child then
--         print(string.format("%d: %s (ClassName: %s)", 
--             i, 
--             child.Name, 
--             child.ClassName))
--     end
-- end

-- -- 5. Heartbeat イベント
-- local frameCount = 0
-- RunService.Heartbeat:Connect(function(dt)
--     frameCount = frameCount + 1
    
--     -- 60フレームごとに情報を表示
--     if frameCount % 60 == 0 then
--         local root = workspace:FindFirstChild("HumanoidRootPart")
--         if root then
--             local pos = root.Position
--             if pos then
--                 print(string.format("[Frame %d] Player at (%.2f, %.2f, %.2f)", 
--                     frameCount, pos.X, pos.Y, pos.Z))
--             end
--         end
--     end
    
--     -- 300フレーム後にFloppaCubeを動かす
--     if frameCount == 300 then
--         local floppa = workspace:FindFirstChild("FloppaCube")
--         if floppa then
--             print("\n🎯 Moving FloppaCube!")
--             floppa.Position = {X = 0, Y = 20, Z = 0}
--         end
--     end
-- end)

print("\n=== Lua Script Initialized ===")