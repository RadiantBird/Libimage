#include "Physics.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>

// ====================================================================
// ヘルパー関数 (変更なし)
// ====================================================================

std::vector<Vector3> getOBBVertices(const Cube& cube) {
    std::vector<Vector3> vertices;
    Vector3 half = cube.size * 0.5f;
    Matrix3 R = Matrix3::rotate(cube.rotation);
    Vector3 localVerts[8] = {
        Vector3(-half.x, -half.y, -half.z), Vector3( half.x, -half.y, -half.z),
        Vector3( half.x,  half.y, -half.z), Vector3(-half.x,  half.y, -half.z),
        Vector3(-half.x, -half.y,  half.z), Vector3( half.x, -half.y,  half.z),
        Vector3( half.x,  half.y,  half.z), Vector3(-half.x,  half.y,  half.z)
    };
    for(int i = 0; i < 8; i++) vertices.push_back(cube.pos + R * localVerts[i]);
    return vertices;
}

void getOBBAxes(const Cube& cube, Vector3 axes[3]) {
    Matrix3 R = Matrix3::rotate(cube.rotation);
    axes[0] = R * Vector3(1, 0, 0);
    axes[1] = R * Vector3(0, 1, 0);
    axes[2] = R * Vector3(0, 0, 1);
}

void projectVertices(const std::vector<Vector3>& vertices, const Vector3& axis, float& min, float& max) {
    min = max = vertices[0].dot(axis);
    for(size_t i = 1; i < vertices.size(); i++) {
        float proj = vertices[i].dot(axis);
        if(proj < min) min = proj;
        if(proj > max) max = proj;
    }
}

bool testSeparatingAxis(const std::vector<Vector3>& vertsA, const std::vector<Vector3>& vertsB,
                        const Vector3& axis, float& outPenetration) {
    float minA, maxA, minB, maxB;
    projectVertices(vertsA, axis, minA, maxA);
    projectVertices(vertsB, axis, minB, maxB);
    if(maxA < minB || maxB < minA) return false;
    float overlap = std::min(maxA - minB, maxB - minA);
    outPenetration = overlap;
    return true;
}

// ====================================================================
// Physics クラス実装
// ====================================================================

void Physics::simulate(Workspace& ws, float dt) {
    const int subSteps = 8;
    float subDt = dt / subSteps;

    for (int step = 0; step < subSteps; ++step) {
        integrateAcceleration(ws, subDt);

        const int collisionIterations = 4;
        for (int iter = 0; iter < collisionIterations; ++iter) {
            for (size_t i = 0; i < ws.cubes.size(); ++i) {
                for (size_t j = i + 1; j < ws.cubes.size(); ++j) {
                    Cube& a = ws.cubes[i];
                    Cube& b = ws.cubes[j];

                    if (a.anchored && b.anchored) continue;
                    if (a.isSleeping && b.isSleeping) continue;

                    if (!broadPhaseAABB(a, b)) continue;

                    Contact contact;
                    if (detectOBBCollision(a, b, contact)) {
                        if(a.isSleeping) a.wakeUp();
                        if(b.isSleeping) b.wakeUp();

                        if(a.isPlayer && contact.normal.y < -0.7f) a.onGround = true;
                        if(b.isPlayer && contact.normal.y > 0.7f) b.onGround = true;

                        resolveCollision(a, b, contact);
                        correctPosition(a, b, contact);
                    }
                }
            }
        }
        integrateVelocity(ws, subDt);
    }
}

void Physics::integrateAcceleration(Workspace& ws, float dt) {
    const float sleepVelThreshold = 0.4f;
    const float sleepAngThreshold = 0.4f;
    const float sleepTimeThreshold = 0.5f;

    for (auto& c : ws.cubes) {
        if (c.anchored || c.isSleeping) continue;

        c.velocity += ws.gravity * dt;

        // 減衰処理 (Damping)
        c.velocity *= 0.999f; 
        
        // 【修正1】角運動の極限減衰 (0.98f -> 0.90f)
        // 外部から何も力が加わっていなくても、極めて積極的エネルギーを奪い、収束させる
        c.angularVelocity *= 0.90f; 

        // 【追加】ごく低速時の強制停止 (Jitter/微振動を即座に殺す)
        if (c.velocity.lengthSquared() < 0.01f) c.velocity = Vector3(0,0,0);
        if (c.angularVelocity.lengthSquared() < 0.01f) c.angularVelocity = Vector3(0,0,0);


        // 角速度のハードリミット (Max Angular Velocity)
        const float maxAngVel = 10.0f; 
        if (c.angularVelocity.lengthSquared() > maxAngVel * maxAngVel) {
            c.angularVelocity = c.angularVelocity.normalized() * maxAngVel;
        }

        // スリープ判定
        if (!c.isPlayer) {
            if (c.velocity.lengthSquared() < sleepVelThreshold * sleepVelThreshold &&
                c.angularVelocity.lengthSquared() < sleepAngThreshold * sleepAngThreshold) {
                c.sleepTimer += dt;
                if (c.sleepTimer > sleepTimeThreshold) {
                    c.isSleeping = true;
                    c.velocity = Vector3(0,0,0);
                    c.angularVelocity = Vector3(0,0,0);
                }
            } else {
                c.sleepTimer = 0.0f;
            }
        }

        c.updateInertiaWorld();
        
        if (c.isPlayer) {
            c.angularVelocity = Vector3(0,0,0);
            c.rotation.x = 0; c.rotation.z = 0;
            c.onGround = false;
        }
    }
}

void Physics::integrateVelocity(Workspace& ws, float dt) {
    for (auto& c : ws.cubes) {
        if (c.anchored || c.isSleeping) continue;

        c.pos += c.velocity * dt;

        if (!c.isPlayer && c.angularVelocity.lengthSquared() > 1e-8f) {
            Matrix3 R = Matrix3::rotate(c.rotation);
            Matrix3 omegaStar;
            omegaStar.setZero();
            omegaStar.m[0][1] = -c.angularVelocity.z; omegaStar.m[0][2] = c.angularVelocity.y;
            omegaStar.m[1][0] = c.angularVelocity.z;  omegaStar.m[1][2] = -c.angularVelocity.x;
            omegaStar.m[2][0] = -c.angularVelocity.y; omegaStar.m[2][1] = c.angularVelocity.x;

            Matrix3 dR = omegaStar * R;
            R = R + (dR * dt);
            R.orthonormalize();
            c.rotation = R.toEuler();
        }
    }
}

bool Physics::broadPhaseAABB(const Cube& a, const Cube& b) {
    float scale = 1.732f;
    Vector3 sizeA = a.size * scale;
    Vector3 sizeB = b.size * scale;
    return (std::abs(a.pos.x - b.pos.x) < (sizeA.x + sizeB.x) * 0.5f) &&
           (std::abs(a.pos.y - b.pos.y) < (sizeA.y + sizeB.y) * 0.5f) &&
           (std::abs(a.pos.z - b.pos.z) < (sizeA.z + sizeB.z) * 0.5f);
}

bool Physics::detectOBBCollision(const Cube& a, const Cube& b, Contact& outContact) {
    std::vector<Vector3> vertsA = getOBBVertices(a);
    std::vector<Vector3> vertsB = getOBBVertices(b);
    Vector3 axesA[3], axesB[3];
    getOBBAxes(a, axesA);
    getOBBAxes(b, axesB);

    float minPen = 1e10f;
    Vector3 bestAxis;
    bool found = false;

    std::vector<Vector3> axes;
    for(int i=0; i<3; i++) axes.push_back(axesA[i]);
    for(int i=0; i<3; i++) axes.push_back(axesB[i]);

    const float crossThreshold = 1e-4f;
    for(int i=0; i<3; i++) {
        for(int j=0; j<3; j++) {
            Vector3 cross = axesA[i].cross(axesB[j]);
            if(cross.lengthSquared() > crossThreshold) {
                axes.push_back(cross.normalized());
            }
        }
    }

    for(size_t i=0; i<axes.size(); i++) {
        float pen;
        if(!testSeparatingAxis(vertsA, vertsB, axes[i], pen)) return false; 
        if (i >= 6) pen *= 1.05f; // バイアス

        if(pen < minPen) {
            minPen = pen;
            bestAxis = axes[i];
            found = true;
        }
    }

    if (!found) return false;

    Vector3 dir = b.pos - a.pos;
    if(bestAxis.dot(dir) < 0) bestAxis = bestAxis * -1.0f;

    outContact.normal = bestAxis;
    outContact.penetration = minPen;
    outContact.point = findContactPoint(a, b, bestAxis);

    return true;
}

Vector3 Physics::findContactPoint(const Cube& a, const Cube& b, const Vector3& normal) {
    auto getAverageContact = [&](const Cube& c, const Vector3& n) {
        std::vector<Vector3> verts = getOBBVertices(c);
        float maxDist = -1e20f;
        for(const auto& v : verts) {
            float d = v.dot(n);
            if(d > maxDist) maxDist = d;
        }
        // 接触点閾値を0.15fに維持
        const float threshold = 0.15f; 
        Vector3 sum(0,0,0);
        int count = 0;
        for(const auto& v : verts) {
            if(v.dot(n) >= maxDist - threshold) {
                sum += v; count++;
            }
        }
        return (count > 0) ? (sum / (float)count) : verts[0];
    };
    Vector3 pA = getAverageContact(a, normal);
    Vector3 pB = getAverageContact(b, normal * -1.0f);
    return (pA + pB) * 0.5f;
}

void Physics::resolveCollision(Cube& a, Cube& b, const Contact& contact) {
    Vector3 n = contact.normal;
    Vector3 rA = contact.point - a.pos;
    Vector3 rB = contact.point - b.pos;

    Vector3 vA = a.velocity + a.angularVelocity.cross(rA);
    Vector3 vB = b.velocity + b.angularVelocity.cross(rB);
    Vector3 relVel = vB - vA;

    float velAlongNormal = relVel.dot(n);
    if (velAlongNormal > 0) return;

    Vector3 rAxN = rA.cross(n);
    Vector3 rBxN = rB.cross(n);
    
    float angA = (a.invInertiaTensorWorld * rAxN).cross(rA).dot(n);
    float angB = (b.invInertiaTensorWorld * rBxN).cross(rB).dot(n);

    float invMassSum = a.invMass + b.invMass;
    
    if(!a.anchored) invMassSum += angA;
    if(!b.anchored) invMassSum += angB;

    if (invMassSum < 1e-6f) return;

    float e = std::min(a.restitution, b.restitution);
    
    // 低速時の反発抑制 (Stabilization)
    bool isStabilizing = false;
    if (std::abs(velAlongNormal) < 1.0f) { 
        e = 0.0f;
        isStabilizing = true;
    }

    float j = -(1.0f + e) * velAlongNormal / invMassSum;
    Vector3 impulse = n * j;

    // 速度更新
    if (!a.anchored) {
        a.velocity -= impulse * a.invMass;
        // ここを微調整して発散を防ぐよ
        if (!isStabilizing) a.angularVelocity -= a.invInertiaTensorWorld * rA.cross(impulse);
        else a.angularVelocity *= 0.95f; 
    }
    if (!b.anchored) {
        b.velocity += impulse * b.invMass;
        if (!isStabilizing) b.angularVelocity += b.invInertiaTensorWorld * rB.cross(impulse);
        else b.angularVelocity *= 0.95f;
    }

    // 摩擦 (Friction)
    Vector3 t = relVel - n * velAlongNormal;
    if (t.lengthSquared() > 1e-6f) {
        t = t.normalized();
        
        // 摩擦インパルスの簡易計算（安定性のための近似）
        float jt = -relVel.dot(t) / invMassSum; 
        
        float mu = std::sqrt(a.friction * b.friction);
        float maxFriction = std::abs(j) * mu;
        if (std::abs(jt) > maxFriction) jt = (jt > 0) ? maxFriction : -maxFriction;

        Vector3 frictionImpulse = t * jt;
        
        if (!a.anchored) {
            a.velocity -= frictionImpulse * a.invMass;
            // 【修正2-2】安定化時は摩擦トルクを完全に無視する (0.1fを維持)
            if(!isStabilizing) {
                // 衝突が激しい時だけ、回転を大きく抑制しながら適用
                a.angularVelocity -= (a.invInertiaTensorWorld * rA.cross(frictionImpulse)) * 0.1f; 
            }
        }
        if (!b.anchored) {
            b.velocity += frictionImpulse * b.invMass;
            if(!isStabilizing) {
                b.angularVelocity += (b.invInertiaTensorWorld * rB.cross(frictionImpulse)) * 0.1f;
            }
        }
    }
}

// --- 位置補正 (沈み込み防止) ---
void Physics::correctPosition(Cube& a, Cube& b, const Contact& contact) {
    // 補正率をマイルドに維持 (0.2f)
    const float percent = 0.2f; 
    const float slop = 0.01f;

    float correctionMag = std::max(contact.penetration - slop, 0.0f) * percent / (a.invMass + b.invMass);
    Vector3 correction = contact.normal * correctionMag;
    
    if(!a.anchored) a.pos -= correction * a.invMass;
    if(!b.anchored) b.pos += correction * b.invMass;
}