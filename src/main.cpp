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
    float zoomDistance = 100.0f;
} mouseState;

Camera* g_camera = nullptr;

// オプション変数
bool Cursor_locked = true;
bool Cursor_unlock = true;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            mouseState.rightButtonPressed = true;
            mouseState.firstMouse = true; // ドラッグ開始時にジャンプを防ぐ
            // カーソルを非表示にして画面中央に固定（オプション）
            if (Cursor_locked) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else if (action == GLFW_RELEASE) {
            mouseState.rightButtonPressed = false;
            // カーソルを表示に戻す（オプション）
            if (Cursor_unlock) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!mouseState.rightButtonPressed || !g_camera) {
        return;
    }

    if (mouseState.firstMouse) {
        mouseState.lastX = xpos;
        mouseState.lastY = ypos;
        mouseState.firstMouse = false;
        return;
    }

    double xoffset = xpos - mouseState.lastX;
    double yoffset = mouseState.lastY - ypos;
    mouseState.lastX = xpos;
    mouseState.lastY = ypos;

    float sensitivity = 0.2f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    g_camera->rotation.y -= xoffset;
    g_camera->rotation.x += yoffset;

    g_camera->rotation.x = std::max(-89.0f, std::min(89.0f, g_camera->rotation.x));
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    mouseState.zoomDistance -= yoffset * 10.0f;
    mouseState.zoomDistance = std::max(10.0f, std::min(500.0f, mouseState.zoomDistance));
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

    // マウスコールバックの設定
    glfwSetMouseButtonCallback(win, mouse_button_callback);
    glfwSetCursorPosCallback(win, cursor_position_callback);
    glfwSetScrollCallback(win, scroll_callback);

    Renderer renderer;
    Workspace workspace;
    Physics physics;
    Camera mainCamera;
    
    g_camera = &mainCamera;
    
    mainCamera.pos = Vector3(0, 30, -80);
    mainCamera.rotation = Vector3(20, 0, 0);

    // 【修正】レンダラーとワークスペースを先に初期化
    renderer.init();
    workspace.initScene(renderer.getSkyboxTextureID());

    // 【修正】その後にLuaを初期化（この時点でcubesは存在する）
    std::cout << "\n=== Lua console ===" << std::endl;
    initLua();
    std::cout << "========================\n" << std::endl;

    float lastTime = glfwGetTime();
    bool isFreeCam = false;
    bool pKeyBlock = false; 

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

        physics.simulate(workspace, dt);
        RunService::Heartbeat.fire(dt);
        renderer.render(workspace, mainCamera, lookTarget);

        glfwSwapBuffers(win);
    }

    glfwTerminate();
    return 0;
}