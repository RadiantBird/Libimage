// src/Game/Workspace.cpp
#include "Workspace.hpp"

Workspace::Workspace() 
    : Instance("Workspace", "Workspace"), // Instance コンストラクタ呼び出し
      playerIndex(0), 
      gravity(0, -98.0f, 0) 
{}

Workspace* global_workspace = nullptr;

void Workspace::initScene(unsigned int skyboxTexID) {
    global_workspace = this;
    cubes.clear();

    // 0: Player
    cubes.push_back(
        CubeBuilder()
            .size(4, 6, 2)
            .pos(0, 10, 0)
            .setName("Player") // 名前を設定
            .texture("assets/textures/noob.jpg")
            .setPlayer()
            .build()
    );
    playerIndex = 0;

    // 1: Ground
    cubes.push_back(
        CubeBuilder()
            .size(512, 5, 512)
            .pos(0, -2.5, 0)
            .setName("Ground") // 名前を設定
            .color(255, 255, 255)
            .setStatic()
            .texture("assets/textures/rblx_grass.jpg")
            .build()
    );
    
    // Others
    cubes.push_back(
        CubeBuilder()
            .size(10, 10, 10)
            .pos(5, 10, 20)
            .setName("FloppaCube") // 名前を設定
            .color(255, 255, 255)
            .texture("assets/textures/floppa_face_2048.jpg")
            .build()
    );

    cubes.push_back(
        CubeBuilder()
            .size(10, 10, 10)
            .pos(5, 15, 23)
            .setName("SaladCat") // 名前を設定
            .color(255, 255, 255)
            .texture("assets/textures/salad-cat.jpg")
            .build()
    );

    cubes.push_back(
        CubeBuilder()
            .size(5, 5, 5)
            .pos(-10, 10, 0)
            .setName("GreenCube") // 名前を設定
            .color(0, 150, 100)
            .build()
    );

    cubes.push_back(
        CubeBuilder()
            .size(6, 6, 6)
            .pos(10, 50, 10)
            .setName("RedCube") // 名前を設定
            .color(255, 0, 0)
            .build()
    );
}

Cube* Workspace::getPlayer() {
    if (playerIndex < cubes.size()) {
        return &cubes[playerIndex];
    }
    return nullptr;
}