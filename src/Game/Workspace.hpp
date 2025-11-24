// src/Game/Workspace.hpp
#ifndef WORKSPACE_HPP
#define WORKSPACE_HPP

#include <vector>
#include "GameData.hpp"
#include "Instance.hpp"

class Workspace : public Instance {
public:
    std::vector<Cube> cubes;
    size_t playerIndex;
    Vector3 gravity;

    Workspace();
    
    // シーンの初期化
    void initScene(unsigned int skyboxTexID);

    // プレイヤーオブジェクトへの参照を取得
    Cube* getPlayer();

    // IsA のオーバーライド
    bool IsA(const std::string& className) const override {
        return className == "Workspace" || Instance::IsA(className);
    }

    // FindFirstChild のオーバーライド（cubes配列も検索）
    Instance* FindFirstChild(const std::string& name, bool recursive = false) override {
        // cubes 配列から検索
        for (auto& cube : cubes) {
            if (cube.Name == name) {
                return &cube;
            }
        }
        
        // 通常のChildrenからも検索
        return Instance::FindFirstChild(name, recursive);
    }
};

extern Workspace* global_workspace;

#endif // WORKSPACE_HPP