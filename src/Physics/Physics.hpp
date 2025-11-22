#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include "src/Game/Workspace.hpp"
#include "src/Game/GameData.hpp"

struct Contact {
    Vector3 point;
    Vector3 normal;
    float penetration;
};

class Physics {
public:
    void simulate(Workspace& workspace, float dt);

private:
    void resolveCollision(Cube& a, Cube& b, const Contact& contact);
    bool detectCollision(const Cube& a, const Cube& b, Contact& outContact);
};

#endif