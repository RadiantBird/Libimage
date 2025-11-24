// src/Game/Player.hpp
#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "GameData.hpp"
#include "Instance.hpp"
#include <memory>

// プレイヤークラス
class Player : public Instance {
public:
    Cube* HumanoidRootPart;
    Cube* Head;
    Cube* Torso;
    Cube* LeftArm;
    Cube* RightArm;
    Cube* LeftLeg;
    Cube* RightLeg;
    
    bool onGround;
    Vector3 moveDirection;
    
    Player(const std::string& name = "Player")
        : Instance(name, "Model"),
          HumanoidRootPart(nullptr),
          Head(nullptr), Torso(nullptr),
          LeftArm(nullptr), RightArm(nullptr),
          LeftLeg(nullptr), RightLeg(nullptr),
          onGround(false),
          moveDirection(0, 0, 0)
    {}
    
    void buildBody(std::vector<Cube>& cubesContainer, const Vector3& spawnPosition) {
        size_t startIndex = cubesContainer.size();
        
        // 0. HumanoidRootPart (物理演算の本体)
        cubesContainer.push_back(
            CubeBuilder()
                .size(4, 10, 2) 
                .pos(spawnPosition)
                .setName("HumanoidRootPart")
                .color(255, 100, 0) 
                .setPlayer()
                .setSimulated(true)
                .setCanCollide(true)
                .setTransparency(1.0f) // 透明化
                .build()
        );
        HumanoidRootPart = &cubesContainer[startIndex + 0];
        
        // --- 以下は装飾パーツ ---
        
        // 1. Torso
        cubesContainer.push_back(
            CubeBuilder()
                .size(4, 4, 2)
                .pos(spawnPosition)
                .setName("Torso")
                .color(0, 140, 255)
                .setSimulated(false).setCanCollide(false)
                .build()
        );
        Torso = &cubesContainer[startIndex + 1];
        
        // 2. Head
        cubesContainer.push_back(
            CubeBuilder()
                .size(4, 2, 2)
                .pos(spawnPosition)
                .setName("Head")
                .color(255, 211, 0)
                .texture("assets/textures/smile.png")
                .setSimulated(false).setCanCollide(false)
                .build()
        );
        Head = &cubesContainer[startIndex + 2];
        
        // 3. Right Arm
        cubesContainer.push_back(
            CubeBuilder()
                .size(2, 4, 2)
                .pos(spawnPosition)
                .setName("RightArm")
                .color(255, 211, 0)
                .setSimulated(false).setCanCollide(false)
                .build()
        );
        RightArm = &cubesContainer[startIndex + 3];
        
        // 4. Left Arm
        cubesContainer.push_back(
            CubeBuilder()
                .size(2, 4, 2)
                .pos(spawnPosition)
                .setName("LeftArm")
                .color(255, 211, 0)
                .setSimulated(false).setCanCollide(false)
                .build()
        );
        LeftArm = &cubesContainer[startIndex + 4];
        
        // 5. Right Leg
        cubesContainer.push_back(
            CubeBuilder()
                .size(2, 4, 2)
                .pos(spawnPosition)
                .setName("RightLeg")
                .color(109, 211, 0)
                .setSimulated(false).setCanCollide(false)
                .build()
        );
        RightLeg = &cubesContainer[startIndex + 5];
        
        // 6. Left Leg
        cubesContainer.push_back(
            CubeBuilder()
                .size(2, 4, 2)
                .pos(spawnPosition)
                .setName("LeftLeg")
                .color(109, 211, 0)
                .setSimulated(false).setCanCollide(false)
                .build()
        );
        LeftLeg = &cubesContainer[startIndex + 6];
    }
    
    void updateBodyParts() {
        if (!HumanoidRootPart) return;
        
        Vector3 rootPos = HumanoidRootPart->pos;
        Vector3 rootRot = HumanoidRootPart->rotation;
        Matrix3 R = Matrix3::rotate(rootRot);

        // 各パーツのオフセット
        Vector3 offsetTorso    = Vector3(0, 1.0f, 0);
        Vector3 offsetHead     = Vector3(0, 4.0f, 0);
        Vector3 offsetRightArm = Vector3(3.0f, 1.0f, 0);
        Vector3 offsetLeftArm  = Vector3(-3.0f, 1.0f, 0);
        Vector3 offsetRightLeg = Vector3(1.0f, -3.0f, 0);
        Vector3 offsetLeftLeg  = Vector3(-1.0f, -3.0f, 0);

        auto updatePart = [&](Cube* part, Vector3 localOffset) {
            if(!part) return;
            Vector3 worldOffset = R * localOffset;
            part->pos = rootPos + worldOffset;
            part->rotation = rootRot;
        };

        updatePart(Torso, offsetTorso);
        updatePart(Head, offsetHead);
        updatePart(RightArm, offsetRightArm);
        updatePart(LeftArm, offsetLeftArm);
        updatePart(RightLeg, offsetRightLeg);
        updatePart(LeftLeg, offsetLeftLeg);
    }

    void wakeUp() {
        if (HumanoidRootPart) HumanoidRootPart->wakeUp();
    }
    
    Vector3 getPosition() const {
        if (HumanoidRootPart) return HumanoidRootPart->pos;
        return Vector3(0, 0, 0);
    }
    
    void setPosition(const Vector3& newPos) {
        if (!HumanoidRootPart) return;
        HumanoidRootPart->pos = newPos;
        updateBodyParts();
    }
    
    void setVelocity(const Vector3& vel) {
        if (HumanoidRootPart) HumanoidRootPart->velocity = vel;
    }
    
    Vector3 getVelocity() const {
        if (HumanoidRootPart) return HumanoidRootPart->velocity;
        return Vector3(0, 0, 0);
    }

    bool IsA(const std::string& className) const override {
        return className == "Model" || className == "Player" || Instance::IsA(className);
    }
};

#endif // PLAYER_HPP