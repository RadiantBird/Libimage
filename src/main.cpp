/* 
g++ -I. -std=c++17 \
    -Isrc/Game -Isrc/Math -Isrc/Physics -Isrc/Render \
    -I/opt/homebrew/include \
    -L/opt/homebrew/lib \
    -o engine \
    src/main.cpp \
    src/Game/Workspace.cpp \
    src/Game/GameData.cpp \
    src/Physics/Physics.cpp \
    src/Render/Renderer.cpp \
    src/Render/Shader.cpp \
    src/Game/ScriptRunner.cpp \
    -framework OpenGL -lglfw -lGLEW -lm -llua
*/

#define GL_SILENCE_DEPRECATION
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <tuple> 
#include <thread>
#include <atomic>

#include "src/Game/GameData.hpp"
#include "src/Game/Workspace.hpp"
#include "src/Physics/Physics.hpp" 
#include "src/Render/Renderer.hpp"
#include "src/Game/ScriptRunner.hpp"

// グローバル変数（マウス操作用）
struct MouseState {
    double lastX = 0.0;
    double lastY = 0.0;
    bool rightButtonPressed = false;
    bool firstMouse = true;
    float zoomDistance = 100.0f; // カメラとプレイヤーの距離
} mouseState;

// カメラへのポインタ（コールバックからアクセスするため）
Camera* g_camera = nullptr;

// マウスボタンコールバック
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            mouseState.rightButtonPressed = true;
            mouseState.firstMouse = true; // ドラッグ開始時にジャンプを防ぐ
            // カーソルを非表示にして画面中央に固定（オプション）
            // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else if (action == GLFW_RELEASE) {
            mouseState.rightButtonPressed = false;
            // カーソルを表示に戻す（オプション）
            // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

// マウス移動コールバック
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!mouseState.rightButtonPressed || !g_camera) {
        return;
    }

    // 初回のマウス移動でジャンプを防ぐ
    if (mouseState.firstMouse) {
        mouseState.lastX = xpos;
        mouseState.lastY = ypos;
        mouseState.firstMouse = false;
        return;
    }

    // マウスの移動量を計算
    double xoffset = xpos - mouseState.lastX;
    double yoffset = mouseState.lastY - ypos; // Y座標は逆（上が正）
    mouseState.lastX = xpos;
    mouseState.lastY = ypos;

    // マウス感度
    float sensitivity = 0.2f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    // カメラの回転を更新
    g_camera->rotation.y -= xoffset; // Yaw（左右）
    g_camera->rotation.x += yoffset; // Pitch（上下）

    // Pitchを制限（真上・真下を向きすぎないように）
    g_camera->rotation.x = std::max(-89.0f, std::min(89.0f, g_camera->rotation.x));
}

// マウスホイールコールバック
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    // yoffset: 上にスクロール = 正、下にスクロール = 負
    mouseState.zoomDistance -= yoffset * 10.0f; // 10.0fはズーム速度
    
    // ズーム距離を制限（近づきすぎ・離れすぎを防ぐ）
    mouseState.zoomDistance = std::max(10.0f, std::min(500.0f, mouseState.zoomDistance));
    
    //std::cout << "Zoom distance: " << mouseState.zoomDistance << std::endl;
}

int main(){
    if(!glfwInit()) return -1;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE); 

    GLFWwindow* win = glfwCreateWindow((int)SCREEN_W, (int)SCREEN_H, "3D Engine", NULL, NULL);
    if(!win){ 
        glfwTerminate(); 
        return -1; 
    }
    glfwMakeContextCurrent(win); 
    glfwSwapInterval(1);
    if (glewInit() != GLEW_OK) return -1;

    //Luaの非同期実行
    initLua();

    // マウスコールバックの設定
    glfwSetMouseButtonCallback(win, mouse_button_callback);
    glfwSetCursorPosCallback(win, cursor_position_callback);
    glfwSetScrollCallback(win, scroll_callback);

    Renderer renderer;
    Workspace workspace;
    Physics physics;
    Camera mainCamera;
    
    // グローバルポインタに設定（コールバックからアクセスするため）
    g_camera = &mainCamera;
    
    // カメラの初期位置を調整
    mainCamera.pos = Vector3(0, 30, -80);
    mainCamera.rotation = Vector3(20, 0, 0);

    renderer.init();
    workspace.initScene(renderer.getSkyboxTextureID());

    float lastTime = glfwGetTime();
    bool isFreeCam = false;
    bool pKeyBlock = false; 

    bool firstFrame = true;

    while(!glfwWindowShouldClose(win)){
        float cur = glfwGetTime(); 
        float dt = cur - lastTime; 
        lastTime = cur;
        if(dt > 0.1f) dt = 0.1f; 

        glfwPollEvents();

        Vector3 lookTarget(0,0,0);
        Cube* player = workspace.getPlayer();
        if (player) {
            lookTarget = player->pos + Vector3(0, 5.0f, 0);
        }

        if(glfwGetKey(win, GLFW_KEY_P) == GLFW_PRESS) {
            if (!pKeyBlock) {
                isFreeCam = !isFreeCam;
                pKeyBlock = true;
                std::cout << "Free Cam: " << (isFreeCam ? "ON" : "OFF") << std::endl;
            }
        } else {
            pKeyBlock = false;
        }

        // キーボードでのカメラ回転（矢印キー）
        if(glfwGetKey(win,GLFW_KEY_UP)) mainCamera.rotation.x -= 1.5f; 
        if(glfwGetKey(win,GLFW_KEY_DOWN)) mainCamera.rotation.x += 1.5f;
        if(glfwGetKey(win,GLFW_KEY_LEFT)) mainCamera.rotation.y += 1.5f;
        if(glfwGetKey(win,GLFW_KEY_RIGHT)) mainCamera.rotation.y -= 1.5f;
        mainCamera.rotation.x = std::max(-89.0f, std::min(89.0f, mainCamera.rotation.x));

        Vector3 f, r, u; 
        std::tie(f, r, u) = mainCamera.get_directions();

        if (isFreeCam) {
            float s = 50.0f * dt;
            if(glfwGetKey(win,GLFW_KEY_W)) mainCamera.pos += f*s;
            if(glfwGetKey(win,GLFW_KEY_S)) mainCamera.pos -= f*s;
            if(glfwGetKey(win,GLFW_KEY_A)) mainCamera.pos -= r*s;
            if(glfwGetKey(win,GLFW_KEY_D)) mainCamera.pos += r*s;
            if(glfwGetKey(win,GLFW_KEY_Q)) mainCamera.pos -= u*s; 
            if(glfwGetKey(win,GLFW_KEY_E)) mainCamera.pos += u*s; 
        } else if (player) {
            Vector3 flatF = Vector3(f.x, 0, f.z).normalized();
            Vector3 flatR = Vector3(r.x, 0, r.z).normalized();
            
            Vector3 targetV(0, player->velocity.y, 0);
            float speed = 50.0f;

            //同時押しで速くなる。直すか直さないかは後で考えとく
            if(glfwGetKey(win,GLFW_KEY_W)) targetV += flatF * speed;
            if(glfwGetKey(win,GLFW_KEY_S)) targetV -= flatF * speed;
            if(glfwGetKey(win,GLFW_KEY_A)) targetV -= flatR * speed;
            if(glfwGetKey(win,GLFW_KEY_D)) targetV += flatR * speed;
            if(glfwGetKey(win,GLFW_KEY_SPACE) && player->onGround) targetV.y = 50.0f;

            player->velocity.x += (targetV.x - player->velocity.x) * 0.1f;
            player->velocity.z += (targetV.z - player->velocity.z) * 0.1f;
            player->velocity.y = targetV.y;

            // カメラ追従（ズーム距離を適用）
            mainCamera.pos = lookTarget - f * mouseState.zoomDistance;
            player->rotation.y = mainCamera.rotation.y;

            // X軸とZ軸の回転を強制的にリセット
            player->rotation.x = 0.0f; 
            player->rotation.z = 0.0f;
        }

        // if (firstFrame) {
        //     std::cout << "=== Debug Info ===" << std::endl;
        //     std::cout << "Camera pos: " << mainCamera.pos.x << ", " << mainCamera.pos.y << ", " << mainCamera.pos.z << std::endl;
        //     std::cout << "Camera rot: " << mainCamera.rotation.x << ", " << mainCamera.rotation.y << ", " << mainCamera.rotation.z << std::endl;
        //     std::cout << "Number of cubes: " << workspace.cubes.size() << std::endl;
        //     if (workspace.cubes.size() > 1) {
        //         const Cube& ground = workspace.cubes[1];
        //         std::cout << "Ground pos: " << ground.pos.x << ", " << ground.pos.y << ", " << ground.pos.z << std::endl;
        //         std::cout << "Ground size: " << ground.size.x << ", " << ground.size.y << ", " << ground.size.z << std::endl;
        //         std::cout << "Ground texture: " << (ground.texturePath.empty() ? "none" : ground.texturePath) << std::endl;
        //     }
        //     if (player) {
        //         std::cout << "Player pos: " << player->pos.x << ", " << player->pos.y << ", " << player->pos.z << std::endl;
        //     }
        //     std::cout << "\n=== Controls ===" << std::endl;
        //     std::cout << "P: Toggle Free Cam" << std::endl;
        //     std::cout << "WASD: Move" << std::endl;
        //     std::cout << "Q/E: Up/Down (Free Cam only)" << std::endl;
        //     std::cout << "Arrow Keys: Rotate camera" << std::endl;
        //     std::cout << "Right Click + Drag: Rotate camera" << std::endl;
        //     std::cout << "Mouse Wheel: Zoom in/out" << std::endl;
        //     std::cout << "==================" << std::endl;
        //     firstFrame = false;
        // }

        physics.simulate(workspace, dt);
        RunService::Heartbeat.fire(dt);
        renderer.render(workspace, mainCamera, lookTarget);

        glfwSwapBuffers(win);
    }

    glfwTerminate();
    return 0;
}