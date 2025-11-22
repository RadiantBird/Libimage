#ifndef GAMEDATA_HPP
#define GAMEDATA_HPP

#include <vector>
#include <tuple>
#include <cmath>
#include "src/Math/Vector3.hpp"
#include "src/Math/MathUtils.hpp" // 追加

// 定数
const float SCREEN_W = 800;
const float SCREEN_H = 600;
const float FOV_Y = 60.0f;
const float Z_NEAR = 0.1f;
const float Z_FAR = 2000.0f;

struct Cube {
    Vector3 size, pos, color;
    Vector3 rotation; // オイラー角 (度)
    
    // === 変更点 1: texturePath の配置 (既存のフィールドの間に移動) ===
    std::string texturePath;
    // =========================================================
    
    unsigned int textureID;
    bool useTexture;
    bool anchored; // 固定オブジェクトフラグ

    // --- 物理パラメータ ---
    Vector3 velocity;
    Vector3 angularVelocity; // 角速度
    
    float mass, invMass;
    Matrix3 invInertiaTensorLocal; // ローカル座標での慣性テンソル逆行列
    Matrix3 invInertiaTensorWorld; // ワールド座標での慣性テンソル逆行列 (毎フレーム更新)
    
    float restitution; // 反発係数 (0.0-1.0)
    float friction;    // 摩擦係数
    
    bool isPlayer;
    bool onGround;

    // === 変更点 2: コンストラクタ引数の更新 (texturePathを追加) ===
    Cube(Vector3 s, Vector3 p, Vector3 c, Vector3 r = Vector3(0,0,0),
         const std::string& texPath = "", // 新しい引数
         unsigned int texID = 0, bool useTex = false, bool an = false, bool isPl = false)
         : size(s), pos(p), color(c), rotation(r), 
           texturePath(texPath), // 初期化リストで設定
           textureID(texID), useTexture(useTex), anchored(an), 
           velocity(0,0,0), angularVelocity(0,0,0), 
           isPlayer(isPl), onGround(false), restitution(0.2f), friction(0.4f)
    {
        if (an) {
            mass = 0.0f;
            invMass = 0.0f;
            invInertiaTensorLocal.setZero(); // 固定物は回転しない
        } else {
            // 質量計算 (密度=1.0と仮定)
            mass = s.x * s.y * s.z * 1.0f; 
            if(mass < 0.001f) mass = 1.0f;
            invMass = 1.0f / mass;

            // 直方体の慣性モーメント計算
            Matrix3 I;
            I.setZero();
            I.m[0][0] = (1.0f/12.0f) * mass * (size.y*size.y + size.z*size.z);
            I.m[1][1] = (1.0f/12.0f) * mass * (size.x*size.x + size.z*size.z);
            I.m[2][2] = (1.0f/12.0f) * mass * (size.x*size.x + size.y*size.y);

            // 逆行列の計算 (対角行列なので逆数を取るだけ)
            invInertiaTensorLocal.setZero();
            invInertiaTensorLocal.m[0][0] = 1.0f / I.m[0][0];
            invInertiaTensorLocal.m[1][1] = 1.0f / I.m[1][1];
            invInertiaTensorLocal.m[2][2] = 1.0f / I.m[2][2];
        }
        updateInertiaWorld();
    }

    // ワールド空間の慣性テンソルを更新
    void updateInertiaWorld() {
        if(anchored) return;
        Matrix3 R = Matrix3::rotate(rotation);
        invInertiaTensorWorld = R * invInertiaTensorLocal * R.transpose();
    }

    // 任意のワールド座標点における速度を取得 (並進 + 回転)
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
    
    // === 追加点 1: _texturePath フィールドの追加 ===
    std::string _texturePath = "";
    // ===========================================
    
    unsigned int _texID = 0;
    bool _useTex = false;
    bool _anchored = false;
    bool _isPlayer = false;

    CubeBuilder& size(float x, float y, float z) { _size = Vector3(x,y,z); return *this; }
    CubeBuilder& pos(float x, float y, float z) { _pos = Vector3(x,y,z); return *this; }
    CubeBuilder& color(float r, float g, float b) { _color = Vector3(r,g,b); return *this; }
    CubeBuilder& rotation(float x, float y, float z) { _rotation = Vector3(x,y,z); return *this; }
    
    // Vector3を直接渡すオーバーロードもあると便利
    CubeBuilder& size(const Vector3& v) { _size = v; return *this; }
    CubeBuilder& pos(const Vector3& v) { _pos = v; return *this; }

    // === 追加点 2: texture(const std::string& path) メソッドの追加 ===
    CubeBuilder& texture(const std::string& path) {
        _texturePath = path;
        _useTex = true; // パスが指定されたらテクスチャ使用フラグを立てる
        return *this;
    }
    // =============================================================
    
    CubeBuilder& texture(unsigned int id) { _texID = id; _useTex = true; return *this; }
    CubeBuilder& setStatic() { _anchored = true; return *this; }
    CubeBuilder& setPlayer() { _isPlayer = true; return *this; }

    // === 変更点 3: build() の引数を更新 (texturePathを渡す) ===
    Cube build() {
        return Cube(_size, _pos, _color, _rotation, 
                    _texturePath, // パスを渡す
                    _texID, _useTex, _anchored, _isPlayer);
    }
    // ====================================================
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