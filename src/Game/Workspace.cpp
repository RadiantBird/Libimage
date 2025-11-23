#include "Workspace.hpp"

Workspace::Workspace() : playerIndex(0), gravity(0, -98.0f, 0) {}
Workspace* global_workspace = nullptr;

void Workspace::initScene(unsigned int skyboxTexID) {
    global_workspace = this;
    cubes.clear();

    // 0: Player
    cubes.push_back(
        CubeBuilder()
            .size(4, 6, 2)
            .pos(0, 10, 0) // 地面より上に配置
            .texture("assets/textures/noob.jpg")
            .setPlayer()
            .build()
    );
    playerIndex = 0;

    // 1: Ground（サイズを小さくしてテスト）
    cubes.push_back(
        CubeBuilder()
            .size(200, 5, 200) // 2048 -> 200に縮小
            .pos(0, -2.5, 0) // Y=-2.5（地面の上面がY=0になる）
            .color(255, 255, 255) // 【重要】テクスチャの色をそのまま表示するため白
            .setStatic()
            .texture("assets/textures/rblx_grass.jpg")
            .build()
    );
    
    // Others - テクスチャがある場合は色を白に設定
    cubes.push_back(
        CubeBuilder()
            .size(10, 10, 10)
            .pos(5, 10, 20)
            .color(255, 255, 255) // 【修正】テクスチャがある場合は白
            .texture("assets/textures/floppa_face_2048.jpg") // テクスチャパスを指定
            .build()
    );

    cubes.push_back(
        CubeBuilder()
            .size(10, 10, 10)
            .pos(5, 15, 23)
            .color(255, 255, 255) // 【修正】テクスチャがある場合は白
            .texture("assets/textures/salad-cat.jpg") // テクスチャパスを指定
            .build()
    );

    cubes.push_back(
        CubeBuilder()
            .size(5, 5, 5)
            .pos(-10, 10, 0)
            .color(0, 150, 100) // テクスチャなしなので色のまま
            .build()
    );

    cubes.push_back(
        CubeBuilder()
            .size(6, 6, 6)
            .pos(10, 50, 10)
            .color(255, 0, 0) // テクスチャなしなので色のまま
            .build()
    );

    // Skybox（一旦コメントアウト）
    /*
    cubes.push_back(
        CubeBuilder()
            .size(Z_FAR * 2.0f, Z_FAR * 2.0f, Z_FAR * 2.0f)
            .pos(0, 0, 0)
            .color(255, 255, 255)
            .setStatic()
            .build()
    );
    */
}

Cube* Workspace::getPlayer() {
    if (playerIndex < cubes.size()) {
        return &cubes[playerIndex];
    }
    return nullptr;
}