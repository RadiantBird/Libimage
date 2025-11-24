// src/Game/GameData.hpp
#ifndef GAMEDATA_HPP
#define GAMEDATA_HPP

#include <vector>
#include <tuple>
#include <cmath>
#include <functional>
#include <string>
#include <any>

#include "src/Math/Vector3.hpp"
#include "src/Math/MathUtils.hpp"
#include "src/Game/Instance.hpp"

// 定数
const float SCREEN_W = 800;
const float SCREEN_H = 600;
const float FOV_Y = 60.0f;
const float Z_NEAR = 0.1f;
const float Z_FAR = 2000.0f;

class Connection {
public:
    bool connected = true;
    void disconnect() { connected = false; }
};

class EventBase {
public:
    using Callback = std::function<void(std::any)>;
    std::vector<std::pair<Callback, Connection*>> listeners;

    Connection* connect(Callback cb) {
        auto* conn = new Connection();
        listeners.emplace_back(cb, conn);
        return conn;
    }

    void fire(std::any value = std::any()) {
        for (auto it = listeners.begin(); it != listeners.end(); ) {
            if (it->second->connected) {
                it->first(value);
                ++it;
            } else {
                it = listeners.erase(it);
            }
        }
    }
};

struct RunService {
    static EventBase Heartbeat;
};

// Cube は Instance を継承
struct Cube : public Instance {
    Vector3 size, pos, color;
    Vector3 rotation;
    
    std::string texturePath;
    unsigned int textureID;
    bool useTexture;
    bool anchored;

    // --- 物理/衝突フラグ ---
    bool canCollide; 
    bool simulated;  

    // --- 追加: 透明度 (0.0=不透明, 1.0=透明) ---
    float transparency;
    // ---------------------------------------

    // 物理パラメータ
    Vector3 velocity;
    Vector3 angularVelocity;
    
    float mass, invMass;
    Matrix3 invInertiaTensorLocal;
    Matrix3 invInertiaTensorWorld;
    
    float restitution;
    float friction;
    
    bool isPlayer;
    bool onGround;
    bool isSleeping;
    float sleepTimer;

    Cube(Vector3 s, Vector3 p, Vector3 c, Vector3 r = Vector3(0,0,0),
         const std::string& texPath = "",
         unsigned int texID = 0, bool useTex = false, bool an = false, bool isPl = false,
         std::string name = "Part",
         bool canCol = true, bool sim = true,
         float trans = 0.0f) // 引数追加
         : Instance(name, "Part"),
           size(s), pos(p), color(c), rotation(r), 
           texturePath(texPath), textureID(texID), useTexture(useTex), anchored(an), 
           canCollide(canCol), simulated(sim), transparency(trans), // 初期化
           velocity(0,0,0), angularVelocity(0,0,0), 
           isPlayer(isPl), onGround(false), 
           restitution(0.2f), friction(0.5f),
           isSleeping(false), sleepTimer(0.0f)
    {
        if (an || !simulated) { 
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
                Matrix3 I;
                I.setZero();
                
                const float inertiaScale = 5.0f; 

                I.m[0][0] = (1.0f/12.0f) * mass * (size.y*size.y + size.z*size.z) * inertiaScale;
                I.m[1][1] = (1.0f/12.0f) * mass * (size.x*size.x + size.z*size.z) * inertiaScale;
                I.m[2][2] = (1.0f/12.0f) * mass * (size.x*size.x + size.y*size.y) * inertiaScale;

                invInertiaTensorLocal.setZero();
                if(I.m[0][0] > 1e-6f) invInertiaTensorLocal.m[0][0] = 1.0f / I.m[0][0];
                if(I.m[1][1] > 1e-6f) invInertiaTensorLocal.m[1][1] = 1.0f / I.m[1][1];
                if(I.m[2][2] > 1e-6f) invInertiaTensorLocal.m[2][2] = 1.0f / I.m[2][2];
            }
        }
        updateInertiaWorld();
    }

    void updateInertiaWorld() {
        if(anchored || !simulated) return;
        Matrix3 R = Matrix3::rotate(rotation);
        invInertiaTensorWorld = R * invInertiaTensorLocal * R.transpose();
    }
    
    void wakeUp() {
        isSleeping = false;
        sleepTimer = 0.0f;
    }

    Vector3 getPointVelocity(const Vector3& worldPoint) const {
        Vector3 r = worldPoint - pos;
        return velocity + angularVelocity.cross(r);
    }

    bool IsA(const std::string& className) const override {
        return className == "Part" || className == "BasePart" || Instance::IsA(className);
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
    std::string _name = "Part";
    bool _canCollide = true;
    bool _simulated = true;
    
    // 追加
    float _transparency = 0.0f;

    CubeBuilder& size(float x, float y, float z) { _size = Vector3(x,y,z); return *this; }
    CubeBuilder& size(const Vector3& v) { _size = v; return *this; }
    CubeBuilder& pos(float x, float y, float z) { _pos = Vector3(x,y,z); return *this; }
    CubeBuilder& pos(const Vector3& v) { _pos = v; return *this; }
    CubeBuilder& color(float r, float g, float b) { _color = Vector3(r,g,b); return *this; }
    CubeBuilder& color(const Vector3& v) { _color = v; return *this; }
    CubeBuilder& rotation(float x, float y, float z) { _rotation = Vector3(x,y,z); return *this; }
    CubeBuilder& rotation(const Vector3& v) { _rotation = v; return *this; }

    CubeBuilder& texture(const std::string& path) { _texturePath = path; _useTex = true; return *this; }
    CubeBuilder& texture(unsigned int id) { _texID = id; _useTex = true; return *this; }
    CubeBuilder& setStatic() { _anchored = true; return *this; }
    CubeBuilder& setPlayer() { _isPlayer = true; return *this; }
    CubeBuilder& setName(const std::string& s) { _name = s; return *this; }
    CubeBuilder& setCanCollide(bool enable) { _canCollide = enable; return *this; }
    CubeBuilder& setSimulated(bool enable) { _simulated = enable; return *this; }

    // 透明度設定メソッド
    CubeBuilder& setTransparency(float t) { _transparency = t; return *this; }

    Cube build() {
        return Cube(
            _size, _pos, _color, _rotation,
            _texturePath, _texID, _useTex,
            _anchored, _isPlayer, _name,
            _canCollide, _simulated,
            _transparency // 追加
        );
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

#endif // GAMEDATA_HPP