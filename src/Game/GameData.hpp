#ifndef GAMEDATA_HPP
#define GAMEDATA_HPP

#include <vector>
#include <tuple>
#include <cmath>
#include "src/Math/Vector3.hpp"
#include "src/Math/MathUtils.hpp"

// 定数
const float SCREEN_W = 800;
const float SCREEN_H = 600;
const float FOV_Y = 60.0f;
const float Z_NEAR = 0.1f;
const float Z_FAR = 2000.0f;

struct Cube {
    Vector3 size, pos, color;
    Vector3 rotation; // オイラー角 (度)
    
    std::string texturePath;
    unsigned int textureID;
    bool useTexture;
    bool anchored; // 固定オブジェクトフラグ

    // --- 物理パラメータ ---
    Vector3 velocity;
    Vector3 angularVelocity; // 角速度 (rad/s)
    
    float mass, invMass;
    Matrix3 invInertiaTensorLocal;
    Matrix3 invInertiaTensorWorld;
    
    float restitution;
    float friction;
    
    bool isPlayer;
    bool onGround;

    // --- 追加: スリープ（休止）システム用 ---
    bool isSleeping;
    float sleepTimer; 
    // ------------------------------------

    Cube(Vector3 s, Vector3 p, Vector3 c, Vector3 r = Vector3(0,0,0),
         const std::string& texPath = "",
         unsigned int texID = 0, bool useTex = false, bool an = false, bool isPl = false)
         : size(s), pos(p), color(c), rotation(r), 
           texturePath(texPath), textureID(texID), useTexture(useTex), anchored(an), 
           velocity(0,0,0), angularVelocity(0,0,0), 
           isPlayer(isPl), onGround(false), 
           isSleeping(false), sleepTimer(0.0f), // 初期化
           restitution(0.2f), friction(0.5f)
    {
        if (an) {
            mass = 0.0f;
            invMass = 0.0f;
            invInertiaTensorLocal.setZero();
        } else {
            mass = s.x * s.y * s.z * 1.0f; 
            if(mass < 0.001f) mass = 1.0f;
            invMass = 1.0f / mass;

            if (isPlayer) {
                invInertiaTensorLocal.setZero();
            } else {
                // 直方体の慣性モーメント
                Matrix3 I;
                I.setZero();
                
                // 【重要修正】慣性スケーリング係数 (Inertia Scaling)
                // 物理的には1.0が正しいですが、ゲーム挙動安定のため 5.0〜10.0 倍にして
                // 「回転に対する重さ」を水増しします。これで発散を防ぎます。
                const float inertiaScale = 10.0f; 

                I.m[0][0] = (1.0f/12.0f) * mass * (size.y*size.y + size.z*size.z) * inertiaScale;
                I.m[1][1] = (1.0f/12.0f) * mass * (size.x*size.x + size.z*size.z) * inertiaScale;
                I.m[2][2] = (1.0f/12.0f) * mass * (size.x*size.x + size.y*size.y) * inertiaScale;

                invInertiaTensorLocal.setZero();
                // ゼロ除算防止
                if(I.m[0][0] > 1e-6f) invInertiaTensorLocal.m[0][0] = 1.0f / I.m[0][0];
                if(I.m[1][1] > 1e-6f) invInertiaTensorLocal.m[1][1] = 1.0f / I.m[1][1];
                if(I.m[2][2] > 1e-6f) invInertiaTensorLocal.m[2][2] = 1.0f / I.m[2][2];
            }
        }
        updateInertiaWorld();
    }

    void updateInertiaWorld() {
        if(anchored) return;
        Matrix3 R = Matrix3::rotate(rotation);
        invInertiaTensorWorld = R * invInertiaTensorLocal * R.transpose();
    }
    
    // スリープからの復帰
    void wakeUp() {
        isSleeping = false;
        sleepTimer = 0.0f;
    }

    Vector3 getPointVelocity(const Vector3& worldPoint) const {
        Vector3 r = worldPoint - pos;
        return velocity + angularVelocity.cross(r);
    }
};

struct CubeBuilder {
    Vector3 _size = Vector3(1,1,1);
    Vector3 _pos = Vector3(0,0,0);
    Vector3 _color = Vector3(255,255,255);
    Vector3 _rotation = Vector3(0,0,0);
    std::string _texturePath = "";
    unsigned int _texID = 0;
    bool _useTex = false;
    bool _anchored = false;
    bool _isPlayer = false;

    CubeBuilder& size(float x, float y, float z) { _size = Vector3(x,y,z); return *this; }
    CubeBuilder& pos(float x, float y, float z) { _pos = Vector3(x,y,z); return *this; }
    CubeBuilder& color(float r, float g, float b) { _color = Vector3(r,g,b); return *this; }
    CubeBuilder& rotation(float x, float y, float z) { _rotation = Vector3(x,y,z); return *this; }
    CubeBuilder& texture(const std::string& path) { _texturePath = path; _useTex = true; return *this; }
    CubeBuilder& texture(unsigned int id) { _texID = id; _useTex = true; return *this; }
    CubeBuilder& setStatic() { _anchored = true; return *this; }
    CubeBuilder& setPlayer() { _isPlayer = true; return *this; }

    Cube build() {
        return Cube(_size, _pos, _color, _rotation, _texturePath, _texID, _useTex, _anchored, _isPlayer);
    }
};

struct Camera {
    Vector3 pos;
    Vector3 rotation;
    Camera() : pos(0,0,0), rotation(0,0,0) {}
    std::tuple<Vector3,Vector3,Vector3> get_directions() const {
        float yaw = rotation.y*M_PI/180, pitch = rotation.x*M_PI/180;
        Vector3 forward(-sin(yaw)*cos(pitch), -sin(pitch), -cos(yaw)*cos(pitch));
        Vector3 right(cos(yaw), 0, -sin(yaw));
        Vector3 up = right.cross(forward); 
        return {forward.normalized(), right.normalized(), up.normalized()};
    }
};

#endif