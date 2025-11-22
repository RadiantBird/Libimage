#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <vector>
#include <tuple>
#include <cmath>
#include <string> 
#include <map>    

#include "src/Math/Vector3.hpp"
#include "src/Game/GameData.hpp"
#include "src/Game/Workspace.hpp"

// class Shader; // 【削除】

class Renderer {
public:
    Renderer();
    ~Renderer();

    void init();
    void render(const Workspace& ws, const Camera& cam, const Vector3& lookTarget);

    unsigned int getSkyboxTextureID() const { return skyboxTextureID; } 
    unsigned int getTextureID(const std::string& filename); 

private:
    // Shader* lightingShader; // 【削除】
    unsigned int skyboxTextureID;
    
    // unsigned int cubeVAO, cubeVBO, cubeEBO; // 【削除】
    
    // レガシーな描画関数
    void setViewMatrix(const Vector3& eye, const Vector3& f, const Vector3& r, const Vector3& u); 
    void drawCube(const Vector3& pos, const Vector3& rot, const Vector3& scale, const Vector3& color, unsigned int textureID); // 【追加/修正】
    void setupLights() const; // 【追加】固定機能のライト設定

    unsigned int loadTexture(const char* filename);
    unsigned int createWhiteTexture();
    
    // void setupCubeGeometry(); // 【削除】
};

#endif // RENDERER_HPP