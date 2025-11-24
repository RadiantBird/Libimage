// src/Game/Workspace.hpp
#ifndef WORKSPACE_HPP
#define WORKSPACE_HPP

#include <vector>
#include <memory>
#include "GameData.hpp"
#include "Instance.hpp"
#include "Player.hpp"

class Workspace : public Instance {
public:
    std::vector<Cube> cubes;
    Player* player;  // プレイヤーオブジェクト
    Vector3 gravity;

    Workspace();
    ~Workspace();
    
    // シーンの初期化
    void initScene(unsigned int skyboxTexID);

    // プレイヤーのルートパーツを取得（後方互換性のため）
    Cube* getPlayer();
    
    // Playerオブジェクトを取得
    Player* getPlayerObject() {
        return player;
    }

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
        
        // Player オブジェクトも検索
        if (player && player->Name == name) {
            return player;
        }
        
        // 通常のChildrenからも検索
        return Instance::FindFirstChild(name, recursive);
    }
};

extern Workspace* global_workspace;

#endif // WORKSPACE_HPP