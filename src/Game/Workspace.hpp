#ifndef WORKSPACE_HPP
#define WORKSPACE_HPP

#include <vector>
#include "GameData.hpp"

class Workspace {
public:
    std::vector<Cube> cubes;
    size_t playerIndex;
    Vector3 gravity;

    Workspace();
    
    // シーンの初期化（テクスチャIDを受け取ってセットアップ）
    void initScene(unsigned int skyboxTexID);

    // プレイヤーオブジェクトへの参照を取得
    Cube* getPlayer();
};

extern Workspace* global_workspace;
#endif