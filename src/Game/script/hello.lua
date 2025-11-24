-- src/Game/script/hello.lua

print("=== Lua Script Started ===")

-- 1. workspace から FindFirstChild を使う
local player = workspace:FindFirstChild("Player")
if player then
    print("✓ Found Player from workspace!")
    print("  Player Name:", player.Name)
    print("  Player ClassName:", player.ClassName)
    local pos = player.Position
    if pos then
        print("  Player Position:", pos.X, pos.Y, pos.Z)
    end
    
    -- IsA メソッドのテスト
    print("  player:IsA('Part'):", player:IsA("Part"))
    print("  player:IsA('Instance'):", player:IsA("Instance"))
    print("  player:IsA('Workspace'):", player:IsA("Workspace"))
else
    print("✗ Player not found")
end

-- 2. workspace の子から別の子を探す（階層構造のテスト）
local floppa = workspace:FindFirstChild("FloppaCube")
if floppa then
    print("\n✓ Found FloppaCube from workspace!")
    print("  Floppa Name:", floppa.Name)
    print("  Floppa ClassName:", floppa.ClassName)
    
    -- FloppaCube から子を探す（今は子がいないはず）
    local child = floppa:FindFirstChild("NonExistent")
    if child then
        print("  Found child:", child.Name)
    else
        print("  No children in FloppaCube (expected)")
    end
    
    -- GetChildren のテスト
    local children = floppa:GetChildren()
    print("  FloppaCube children count:", #children)
end

-- 3. workspace の全ての子を取得して、それぞれからメソッドを呼ぶ
print("\n=== Testing Methods on All Children ===")
local allChildren = workspace:GetChildren()
for i, child in ipairs(allChildren) do
    if child then
        print(string.format("%d: %s (ClassName: %s, IsA('Part'): %s)", 
            i, 
            child.Name, 
            child.ClassName,
            tostring(child:IsA("Part"))))
        
        -- 各子の子を取得してみる
        local grandChildren = child:GetChildren()
        if #grandChildren > 0 then
            print("   Has " .. #grandChildren .. " children")
        end
    end
end

-- 4. Heartbeat イベントでプレイヤーの位置を監視
local frameCount = 0
RunService.Heartbeat:Connect(function(dt)
    frameCount = frameCount + 1
    
    -- 60フレームごとに情報を表示
    if frameCount % 60 == 0 then
        local p = workspace:FindFirstChild("Player")
        if p then
            local pos = p.Position
            if pos then
                print(string.format("[Frame %d] Player at (%.2f, %.2f, %.2f)", 
                    frameCount, pos.X, pos.Y, pos.Z))
            end
        end
    end
    
    -- 300フレーム後にFloppaCubeを動かす
    if frameCount == 300 then
        local floppa = workspace:FindFirstChild("FloppaCube")
        if floppa then
            print("\n🎯 Moving FloppaCube!")
            floppa.Position = {X = 0, Y = 20, Z = 0}
            print("   New position set to (0, 20, 0)")
        end
    end
end)

print("\n=== Lua Script Initialized ===")