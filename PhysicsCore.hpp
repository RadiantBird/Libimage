#ifndef PHYSICSCORE_HPP
#define PHYSICSCORE_HPP

#include "Vector3.hpp"
#include "MathUtils.hpp"
#include <vector>

// 形状の種類
enum ShapeType { SPHERE, BOX, TETRAHEDRON };

// 衝突形状の基底クラス
struct Collider {
    ShapeType type;
    Collider(ShapeType t) : type(t) {}
    virtual ~Collider() {}
    // 慣性テンソルの計算（質量とサイズから）
    virtual Matrix3 computeInertiaTensor(float mass) = 0;
};

struct SphereCollider : public Collider {
    float radius;
    SphereCollider(float r) : Collider(SPHERE), radius(r) {}
    Matrix3 computeInertiaTensor(float mass) override {
        Matrix3 I;
        float val = (2.0f / 5.0f) * mass * radius * radius;
        I.m[0][0] = I.m[1][1] = I.m[2][2] = val;
        return I;
    }
};

struct BoxCollider : public Collider {
    Vector3 size; // Width, Height, Depth
    BoxCollider(Vector3 s) : Collider(BOX), size(s) {}
    Matrix3 computeInertiaTensor(float mass) override {
        Matrix3 I;
        I.m[0][0] = (1.0f/12.0f) * mass * (size.y*size.y + size.z*size.z);
        I.m[1][1] = (1.0f/12.0f) * mass * (size.x*size.x + size.z*size.z);
        I.m[2][2] = (1.0f/12.0f) * mass * (size.x*size.x + size.y*size.y);
        return I;
    }
};

// 剛体クラス
struct RigidBody {
    Vector3 pos;
    Vector3 velocity;
    Vector3 angularVelocity; // 角速度
    
    Vector3 rotation; // オイラー角 (表示用)
    Matrix3 orientation; // 回転行列 (計算用)

    float mass;
    float invMass;
    Matrix3 invInertiaLocal; // ローカル空間での慣性テンソルの逆行列
    Matrix3 invInertiaWorld; // ワールド空間での慣性テンソルの逆行列

    bool isStatic;
    float restitution = 0.5f; // 反発係数
    float friction = 0.4f;    // 摩擦係数

    Collider* collider;

    RigidBody(Vector3 p, float m, Collider* col, bool stat = false) 
        : pos(p), mass(m), collider(col), isStatic(stat), 
          velocity(0,0,0), angularVelocity(0,0,0), rotation(0,0,0)
    {
        if (isStatic) {
            invMass = 0.0f;
            invInertiaLocal = Matrix3(); // ゼロ行列
            for(int i=0;i<3;i++) invInertiaLocal.m[i][i] = 0;
        } else {
            invMass = 1.0f / mass;
            // 慣性テンソルの逆行列を事前計算
            Matrix3 I = col->computeInertiaTensor(mass);
            // ※簡易的な逆行列計算（対角行列前提）
            invInertiaLocal.m[0][0] = 1.0f / I.m[0][0];
            invInertiaLocal.m[1][1] = 1.0f / I.m[1][1];
            invInertiaLocal.m[2][2] = 1.0f / I.m[2][2];
        }
        updateInertiaWorld();
    }

    // ワールド空間の慣性モーメントを更新 I_world = R * I_local * R^T
    void updateInertiaWorld() {
        if (isStatic) return;
        invInertiaWorld = orientation * invInertiaLocal * orientation.transpose();
    }

    // 指定した点（ワールド座標）での速度を取得（並進速度 + 回転速度）
    Vector3 getPointVelocity(const Vector3& worldPoint) {
        Vector3 r = worldPoint - pos;
        return velocity + angularVelocity.cross(r);
    }
};

struct Contact {
    Vector3 point;       // 衝突点 (ワールド座標)
    float penetration;   // 貫通深度
};

struct Manifold {
    RigidBody* a;
    RigidBody* b;
    Vector3 normal;      // AからBへ向かう法線
    std::vector<Contact> contacts;
};

#endif