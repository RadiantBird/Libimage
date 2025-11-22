#include "Physics.hpp"
#include <iostream>
#include <algorithm>
#include <cmath> // std::abs, std::max, M_PI のために追加/確認

void Physics::simulate(Workspace& ws, float dt) {
    const int subSteps = 16; // 精度向上のためステップ数を増やす
    float subDt = dt / subSteps;
    
    Cube* player = ws.getPlayer();
    if(player) player->onGround = false; // 毎フレームリセット

    for(int step=0; step<subSteps; ++step){
        // 1. 力の適用と積分 (位置・回転の更新)
        for(auto &c : ws.cubes){
            if(c.anchored) continue;
            
            // 重力
            c.velocity += ws.gravity * subDt;
            
            // 空気抵抗 (減衰)
            c.velocity *= 0.99f;
            c.angularVelocity *= 0.98f; 

            // 位置更新
            c.pos += c.velocity * subDt;
            
            // 回転更新 (オイラー角への加算)
            Vector3 deltaRot = c.angularVelocity * (subDt * 180.0f / M_PI);
            c.rotation += deltaRot;

            if (c.isPlayer) {
                // プレイヤーはY軸以外回転しない
                c.angularVelocity.x = 0.0f;
                c.angularVelocity.z = 0.0f;
            }
            // 慣性テンソルの更新
            c.updateInertiaWorld();
        }

        // 2. 衝突検出と解決
        for(size_t i=0; i<ws.cubes.size(); ++i){
            Cube &a = ws.cubes[i];
            for(size_t j=i+1; j<ws.cubes.size(); ++j){
                Cube &b = ws.cubes[j];
                if(a.anchored && b.anchored) continue;

                Contact contact;
                if(detectCollision(a, b, contact)){
                    
                    // === 修正点 1: 接地判定ロジックの修正 ===
                    // Aがプレイヤーの場合、法線が下向き(-Y方向)であれば接地。
                    if(a.isPlayer && contact.normal.y < -0.7f) a.onGround = true;
                    // Bがプレイヤーの場合、法線が上向き(+Y方向)であれば接地。
                    if(b.isPlayer && contact.normal.y > 0.7f) b.onGround = true;
                    // ===================================

                    resolveCollision(a, b, contact);
                }
            }
        }
    }
}

// AABB同士の衝突判定 (戻り値: 衝突したかどうか)
bool Physics::detectCollision(const Cube& a, const Cube& b, Contact& outContact) {
    Vector3 delta = b.pos - a.pos;
    Vector3 overlap(
        (a.size.x + b.size.x)/2.0f - std::abs(delta.x),
        (a.size.y + b.size.y)/2.0f - std::abs(delta.y),
        (a.size.z + b.size.z)/2.0f - std::abs(delta.z)
    );

    if (overlap.x > 0 && overlap.y > 0 && overlap.z > 0) {
        // 最も浅い軸を法線とする
        if (overlap.x < overlap.y && overlap.x < overlap.z) {
            outContact.normal = Vector3(delta.x > 0 ? 1 : -1, 0, 0);
            outContact.penetration = overlap.x;
        } else if (overlap.y < overlap.z) {
            outContact.normal = Vector3(0, delta.y > 0 ? 1 : -1, 0);
            outContact.penetration = overlap.y;
        } else {
            outContact.normal = Vector3(0, 0, delta.z > 0 ? 1 : -1);
            outContact.penetration = overlap.z;
        }
        
        // 衝突点の簡易推定
        Vector3 halfA = a.size * 0.5f;
        Vector3 minA = a.pos - halfA;
        Vector3 maxA = a.pos + halfA;
        Vector3 halfB = b.size * 0.5f;
        Vector3 minB = b.pos - halfB;
        Vector3 maxB = b.pos + halfB;

        Vector3 intersectMin(
            std::max(minA.x, minB.x),
            std::max(minA.y, minB.y),
            std::max(minA.z, minB.z)
        );
        Vector3 intersectMax(
            std::min(maxA.x, maxB.x),
            std::min(maxA.y, maxB.y),
            std::min(maxA.z, maxB.z)
        );
        outContact.point = (intersectMin + intersectMax) * 0.5f;

        return true;
    }
    return false;
}

// インパルスベースの衝突応答
void Physics::resolveCollision(Cube& a, Cube& b, const Contact& contact) {
    Vector3 n = contact.normal;
    
    // 重心から衝突点へのベクトル
    Vector3 ra = contact.point - a.pos;
    Vector3 rb = contact.point - b.pos;

    // 1. 相対速度の計算
    Vector3 va = a.getPointVelocity(contact.point);
    Vector3 vb = b.getPointVelocity(contact.point);
    Vector3 relVel = vb - va;

    float velAlongNormal = relVel.dot(n);

    // 既に離れようとしているなら何もしない
    if (velAlongNormal > 0) return;

    // 2. インパルス(撃力)の大きさ j の計算
    float raLn = ra.cross(n).dot(a.invInertiaTensorWorld * ra.cross(n));
    float rbLn = rb.cross(n).dot(b.invInertiaTensorWorld * rb.cross(n));
    
    float invMassSum = a.invMass + b.invMass + raLn + rbLn;
    float e = std::min(a.restitution, b.restitution);

    float j = -(1.0f + e) * velAlongNormal / invMassSum;

    Vector3 impulse = n * j;

    // 3. 速度と角速度の更新
    if (!a.anchored) {
        a.velocity -= impulse * a.invMass;
        a.angularVelocity -= a.invInertiaTensorWorld * ra.cross(impulse);
    }
    if (!b.anchored) {
        b.velocity += impulse * b.invMass;
        b.angularVelocity += b.invInertiaTensorWorld * rb.cross(impulse);
    }

    // 4. 位置補正 (めり込み解消)
    float percent = 0.4f; 
    float slop = 0.01f;   
    Vector3 correction = n * (std::max(contact.penetration - slop, 0.0f) / (a.invMass + b.invMass) * percent);
    
    if (!a.anchored) a.pos -= correction * a.invMass;
    if (!b.anchored) b.pos += correction * b.invMass;
}