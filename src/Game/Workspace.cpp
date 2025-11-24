// src/Game/Workspace.cpp
#include "Workspace.hpp"

Workspace::Workspace() 
    : Instance("Workspace", "Workspace"),
      player(nullptr),
      gravity(0, -98.0f, 0) 
{}

Workspace::~Workspace() {
    if (player) {
        delete player;
        player = nullptr;
    }
}

Workspace* global_workspace = nullptr;

void Workspace::initScene(unsigned int skyboxTexID) {
    global_workspace = this;
    cubes.clear();

    // 【重要修正】メモリ再確保によるポインタ無効化を防ぐため、十分な数を予約する
    // これをしないと、push_backした瞬間にHumanoidRootPartのポインタがゴミになります
    cubes.reserve(128); 

    // プレイヤーを作成
    player = new Player("Player");
    player->buildBody(cubes, Vector3(0, 10, 0));
    
    // Ground
    cubes.push_back(
        CubeBuilder()
            .size(512, 5, 512)
            .pos(0, -2.5, 0)
            .setName("Ground")
            .color(255, 255, 255)
            .setStatic()
            .texture("assets/textures/rblx_grass.jpg")
            .build()
    );
    
    // その他のオブジェクト
    cubes.push_back(
        CubeBuilder()
            .size(10, 10, 10)
            .pos(5, 10, 20)
            .setName("FloppaCube")
            .color(255, 255, 255)
            .texture("assets/textures/floppa_face_2048.jpg")
            .build()
    );

    cubes.push_back(
        CubeBuilder()
            .size(10, 10, 10)
            .pos(5, 15, 23)
            .setName("SaladCat")
            .color(255, 255, 255)
            .texture("assets/textures/salad-cat.jpg")
            .build()
    );

    cubes.push_back(
        CubeBuilder()
            .size(5, 5, 5)
            .pos(-10, 10, 0)
            .setName("GreenCube")
            .color(0, 150, 100)
            .build()
    );

    cubes.push_back(
        CubeBuilder()
            .size(6, 6, 6)
            .pos(10, 50, 10)
            .setName("RedCube")
            .color(255, 0, 0)
            .build()
    );
}

Cube* Workspace::getPlayer() {
    if (player && player->HumanoidRootPart) {
        return player->HumanoidRootPart;
    }
    return nullptr;
}