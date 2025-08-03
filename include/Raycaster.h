#pragma once

#include "Block/BlockFaceData.h"

#include <glm/glm.hpp>

class World;
class Camera;

class Raycaster
{
public:
    Raycaster(World &world, Camera &camera);
    bool cast();

    glm::ivec3 getHitBlockPosition() const;
    BlockFaces getHitBlockFace() const;

private:
    const int maxDist_;
    glm::vec3 origin_;
    glm::vec3 dir_;
    glm::ivec3 step_;
    glm::ivec3 currVoxelPos_;
    glm::vec3 deltaDist_;

    glm::ivec3 hitBlockPos_;
    BlockFaces hitBlockFace_;

    World &world_;
    Camera &camera_;
};