#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include "src/Game/Workspace.hpp"
#include "src/Math/Vector3.hpp"
#include <vector>

// 接触情報構造体
struct Contact {
    Vector3 point;         // 衝突点（ワールド座標）
    Vector3 normal;        // 衝突法線（BからAへ、またはAからBへの押し出し方向）
    float penetration;     // 貫通深度
    
    Contact() : point(0,0,0), normal(0,1,0), penetration(0.0f) {}
};

class Physics {
public:
    // メインシミュレーション関数
    // dt: 経過時間（秒）
    void simulate(Workspace& ws, float dt);

private:
    // --- フェーズ1: 力の適用と積分 ---
    void integrateAcceleration(Workspace& ws, float dt);
    void integrateVelocity(Workspace& ws, float dt);

    // --- フェーズ2: 衝突検出 ---
    // 広域フェーズ（AABB）
    bool broadPhaseAABB(const Cube& a, const Cube& b);
    
    // 狭域フェーズ（OBB - 分離軸定理）
    bool detectOBBCollision(const Cube& a, const Cube& b, Contact& outContact);
    
    // サポート関数：接触点の特定
    Vector3 findContactPoint(const Cube& a, const Cube& b, const Vector3& normal);

    // --- フェーズ3: 衝突応答 ---
    // 衝突解決（インパルス法）
    void resolveCollision(Cube& a, Cube& b, const Contact& contact);
    
    // 位置補正（めり込み防止）
    void correctPosition(Cube& a, Cube& b, const Contact& contact);
};

#endif // PHYSICS_HPP