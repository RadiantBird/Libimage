// src/Render/Renderer.cpp

#include "Renderer.hpp"
#include <iostream>
#include <map> 
#include <tuple> 
#include <cmath> 
#include <algorithm>
#include <cstring> // strstr用に追加

#define GL_SILENCE_DEPRECATION
#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include "src/Math/Matrix4x4.hpp"
#include "src/Game/GameData.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace {
    std::map<std::string, unsigned int> textureCache; 
    unsigned int cachedWhiteTextureID = 0;

    // 修正: 法線の向きを外向きに統一
    float cubeVertices[] = {
        // 後ろ面 (-Z) - 法線は外向き (0, 0, -1)
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

        // 前面 (+Z) - 法線は外向き (0, 0, 1)
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 0.0f,  1.0f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 0.0f, 0.0f,  1.0f, 1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 0.0f,  1.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 0.0f,  1.0f, 1.0f, 1.0f,

        // 左面 (-X) - 法線は外向き (-1, 0, 0)
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,

        // 右面 (+X) - 法線は外向き (1, 0, 0)
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,

        // 下面 (-Y) - 法線は外向き (0, -1, 0)
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,

        // 上面 (+Y) - 法線は外向き (0, 1, 0)
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f
    };
}

Renderer::Renderer() : skyboxTextureID(0) {
}

Renderer::~Renderer() {
    for (auto const& [key, val] : textureCache) {
        glDeleteTextures(1, (GLuint*)&val);
    }
    if (cachedWhiteTextureID != 0) {
        glDeleteTextures(1, (GLuint*)&cachedWhiteTextureID);
    }
    textureCache.clear();
}

unsigned int Renderer::createWhiteTexture() {
    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    unsigned char data[] = { 255, 255, 255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    
    return tex;
}

unsigned int Renderer::loadTexture(const char* filename) {
    unsigned int tex;
    glGenTextures(1,&tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    
    int w,h,nc;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filename,&w,&h,&nc,0);
    
    if(data){
        std::cout << "✓ Texture loaded: " << filename << " (" << w << "x" << h << ", " << nc << " channels)";
        
        // 2の累乗かチェック
        bool isPowerOfTwo = ((w & (w - 1)) == 0) && ((h & (h - 1)) == 0);
        
        // 警告表示
        if (!isPowerOfTwo) {
            std::cout << " [WARNING: Not power-of-two]";
        }
        if (w > 2048 || h > 2048) {
            std::cout << " [WARNING: Very large texture, consider resizing to 2048x2048 or smaller]";
        }
        std::cout << std::endl;
        
        GLenum fmt = (nc==1?GL_RED:(nc==3?GL_RGB:GL_RGBA));
        
        // テクスチャパラメータの設定（NPOT対応）
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "✗ Failed to load texture: " << filename << std::endl;
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        unsigned char magenta[] = { 255, 0, 255 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, magenta);
    }
    stbi_image_free(data);
    return tex;
}

void Renderer::init() {
    // OpenGL情報を出力
    // std::cout << "\n=== OpenGL Info ===" << std::endl;
    // std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    // std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    // std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    
    // GLint maxTextureSize;
    // glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    // std::cout << "Max Texture Size: " << maxTextureSize << "x" << maxTextureSize << std::endl;
    
    // const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
    // bool supportsNPOT = (strstr(extensions, "GL_ARB_texture_non_power_of_two") != nullptr);
    // std::cout << "Non-Power-of-Two Textures: " << (supportsNPOT ? "SUPPORTED" : "NOT SUPPORTED") << std::endl;
    // std::cout << "===================\n" << std::endl;
    
    glEnable(GL_DEPTH_TEST); 
    glDisable(GL_CULL_FACE);
    
    // ライティングを有効化（テクスチャに陰影がつく）
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);

    cachedWhiteTextureID = createWhiteTexture();
    setupLights();
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
}

unsigned int Renderer::getTextureID(const std::string& filename) {
    if (filename.empty()) {
        return cachedWhiteTextureID;
    }

    if (textureCache.count(filename)) {
        return textureCache[filename];
    }
    
    unsigned int id = loadTexture(filename.c_str());
    textureCache[filename] = id;
    return id;
}

void Renderer::setupLights() const {
    glEnable(GL_LIGHT0);
    
    // 環境光を明るく（テクスチャの色が見えるように）
    GLfloat ambient[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    
    // 拡散光も明るく
    GLfloat diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    
    // スペキュラは控えめに
    GLfloat specular[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void Renderer::setViewMatrix(const Vector3& eye, const Vector3& f, const Vector3& r, const Vector3& u) {
    float m[16] = {
         r.x,  u.x, -f.x,  0,
         r.y,  u.y, -f.y,  0,
         r.z,  u.z, -f.z,  0,
           0,    0,    0,  1
    };
    glMultMatrixf(m);
    glTranslatef(-eye.x, -eye.y, -eye.z);
}

void Renderer::drawCube(const Vector3& pos, const Vector3& rot, const Vector3& scale, const Vector3& color, unsigned int textureID) {
    glPushMatrix();

    glTranslatef(pos.x, pos.y, pos.z);
    
    glRotatef(rot.y, 0.0f, 1.0f, 0.0f);
    glRotatef(rot.x, 1.0f, 0.0f, 0.0f);
    glRotatef(rot.z, 0.0f, 0.0f, 1.0f);

    glScalef(scale.x, scale.y, scale.z);

    // 色の設定（テクスチャと乗算される）
    glColor3f(color.x / 255.0f, color.y / 255.0f, color.z / 255.0f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // テクスチャ環境モードを GL_MODULATE に設定
    // これにより、テクスチャ色 * マテリアル色 * ライト色 になる
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    const int stride = 8 * sizeof(float);

    glVertexPointer(3, GL_FLOAT, stride, ::cubeVertices + 0);
    glNormalPointer(GL_FLOAT, stride, ::cubeVertices + 3);
    glTexCoordPointer(2, GL_FLOAT, stride, ::cubeVertices + 6);
    
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    
    glDisable(GL_TEXTURE_2D);
    
    glPopMatrix();
}

void Renderer::render(const Workspace& ws, const Camera& cam, const Vector3& lookTarget) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    float aspect = SCREEN_W / SCREEN_H; 
    Matrix4x4 projMatrix = Matrix4x4::perspective(FOV_Y, aspect, Z_NEAR, Z_FAR);
    glMultMatrixf(&projMatrix.m[0][0]); 

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    Vector3 f, r, u;
    std::tie(f, r, u) = cam.get_directions(); 
    setViewMatrix(cam.pos, f, r, u); 
    
    GLfloat lightPos[] = {100.0f, 200.0f, 100.0f, 1.0f}; 
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    // 【デバッグ】最初の数フレームでテクスチャIDを出力
    static int debugFrameCount = 0;
    if (debugFrameCount < 1) {
        std::cout << "\n=== Texture Debug Info ===" << std::endl;
        for (size_t i = 0; i < ws.cubes.size(); i++) {
            const auto& block = ws.cubes[i];
            unsigned int texID = getTextureID(block.texturePath);
            std::cout << "Cube " << i << ": ";
            if (block.texturePath.empty()) {
                std::cout << "No texture (using white)" << std::endl;
            } else {
                std::cout << block.texturePath << " -> ID: " << texID << std::endl;
            }
        }
        std::cout << "========================\n" << std::endl;
        debugFrameCount++;
    }

    for (const auto& block : ws.cubes) {
        unsigned int texID = getTextureID(block.texturePath);
        drawCube(block.pos, block.rotation, block.size, block.color, texID);
    }
    
    glFlush();
}