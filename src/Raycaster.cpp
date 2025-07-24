#include "Raycaster.h"
#include "World.h"
#include "Constants.h"
#include <glm/glm.hpp>
#include <iostream>

Raycaster::Raycaster(World &world, Camera &camera)
    : maxDist_(Constants::MAX_RAYCAST_DIST),
      world_(world),
      camera_(camera),
      hitBlockPos_(glm::ivec3(0))
{
}

bool Raycaster::cast()
{
    // The DDA algorithm expects a grid aligned to integer coordinates, but blocks
    // are centered on them. We solve this by shifting the ray's origin into the
    // DDA's "grid space" before running the calculations.
    origin_ = (camera_.Position + glm::vec3(0.5f)) + camera_.Front * 1e-4f;
    dir_ = glm::normalize(camera_.Front);
    currVoxelPos_ = glm::ivec3(glm::floor(origin_));
    step_ = glm::ivec3(0);
    deltaDist_ = glm::vec3(0.0f);

    // Debugging
    // std::cout << "=== RAYCAST DEBUG ===" << std::endl;
    // std::cout << "Ray Origin: " << origin_.x << ", " << origin_.y << ", " << origin_.z << std::endl;
    // std::cout << "Ray Direction: " << dir_.x << ", " << dir_.y << ", " << dir_.z << std::endl;
    // std::cout << "Starting Voxel: " << currVoxelPos_.x << ", " << currVoxelPos_.y << ", " << currVoxelPos_.z << std::endl;
    // std::cout << "Max Distance: " << maxDist_ << std::endl;

    // Calculate step and deltaDist
    for (int i = 0; i < 3; i++)
    {
        if (dir_[i] < 0)
        {
            step_[i] = -1;
            deltaDist_[i] = (abs(dir_[i]) < 0.0001f) ? 1e30f : abs(1.0f / dir_[i]);
        }
        else if (dir_[i] > 0)
        {
            step_[i] = 1;
            deltaDist_[i] = (abs(dir_[i]) < 0.0001f) ? 1e30f : abs(1.0f / dir_[i]);
        }
        else
        {
            step_[i] = 0;
            deltaDist_[i] = 1e30f; // Effectively infinite
        }
    }

    // Calculate tMax
    glm::vec3 tMax;
    for (int i = 0; i < 3; i++)
    {
        if (dir_[i] < 0)
        {
            tMax[i] = (origin_[i] - float(currVoxelPos_[i])) * deltaDist_[i];
        }
        else if (dir_[i] > 0)
        {
            tMax[i] = (float(currVoxelPos_[i] + 1) - origin_[i]) * deltaDist_[i];
        }
        else
        {
            tMax[i] = 1e30f; // Ray parallel to this axis
        }
    }

    // Step thorugh voxels
    for (int i = 0; i < maxDist_; i++)
    {
        // std::cout << "Stepping to voxel "
        //           << currVoxelPos_.x << ", "
        //           << currVoxelPos_.y << ", "
        //           << currVoxelPos_.z << std::endl;

        // Find which axis to step along
        if (tMax.x < tMax.y && tMax.x < tMax.z)
        {
            // Step along X
            tMax.x += deltaDist_.x;
            currVoxelPos_.x += step_.x;
        }
        else if (tMax.y < tMax.z)
        {
            // Step along Y
            tMax.y += deltaDist_.y;
            currVoxelPos_.y += step_.y;
        }
        else
        {
            // Step along Z
            tMax.z += deltaDist_.z;
            currVoxelPos_.z += step_.z;
        }

        // Check current voxel
        if (world_.isBlockSolid(currVoxelPos_))
        {
            hitBlockPos_ = currVoxelPos_;
            return true;
        }
    }

    return false;
}

glm::ivec3 Raycaster::getHitBlockPosition() const
{
    return hitBlockPos_;
}
