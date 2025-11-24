// src/Render/Renderer.hpp
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

class Renderer {
public:
    Renderer();
    ~Renderer();

    void init();
    void render(const Workspace& ws, const Camera& cam, const Vector3& lookTarget);

    unsigned int getSkyboxTextureID() const { return skyboxTextureID; } 
    unsigned int getTextureID(const std::string& filename); 

private:
    unsigned int skyboxTextureID;
    
    void setViewMatrix(const Vector3& eye, const Vector3& f, const Vector3& r, const Vector3& u); 
    // 【修正】引数に transparency を追加
    void drawCube(const Vector3& pos, const Vector3& rot, const Vector3& scale, const Vector3& color, unsigned int textureID, float transparency);
    void setupLights() const;

    unsigned int loadTexture(const char* filename);
    unsigned int createWhiteTexture();
};

#endif // RENDERER_HPP