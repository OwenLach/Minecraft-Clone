#pragma once

#include "Block/BlockTypes.h"

#include <array>
#include <unordered_map>

#include <glm/glm.hpp>

enum class BlockFaces
{
    Right,
    Left,
    Top,
    Bottom,
    Front,
    Back
};

struct BlockUVs
{
    std::array<glm::vec2, 4> top;
    std::array<glm::vec2, 4> side;
    std::array<glm::vec2, 4> bottom;
};

namespace BlockFaceData
{
    using Vec3 = glm::vec3;
    using Vec2 = glm::vec2;
    using IVec3 = glm::ivec3;
    using AOTriplet = std::array<IVec3, 3>;

    inline const std::unordered_map<BlockFaces, std::array<Vec3, 4>> faceCorners = {
        {BlockFaces::Front, {Vec3(-0.5f, -0.5f, 0.5f), Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f)}},
        {BlockFaces::Back, {Vec3(0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f)}},
        {BlockFaces::Left, {Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(-0.5f, 0.5f, -0.5f)}},
        {BlockFaces::Right, {Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, -0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, 0.5f)}},
        {BlockFaces::Top, {Vec3(-0.5f, 0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Vec3(0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f)}},
        {BlockFaces::Bottom, {Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.5f, -0.5f, -0.5f), Vec3(0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f)}},
    };

    inline const std::unordered_map<BlockFaces, std::array<AOTriplet, 4>> aoOffsets = {
        {BlockFaces::Front, {
                                AOTriplet{IVec3(-1, 0, 1), IVec3(0, -1, 1), IVec3(-1, -1, 1)}, // Bottom-left vertex
                                AOTriplet{IVec3(1, 0, 1), IVec3(0, -1, 1), IVec3(1, -1, 1)},   // Bottom-right vertex
                                AOTriplet{IVec3(1, 0, 1), IVec3(0, 1, 1), IVec3(1, 1, 1)},     // Top-right vertex
                                AOTriplet{IVec3(-1, 0, 1), IVec3(0, 1, 1), IVec3(-1, 1, 1)}    // Top-left vertex
                            }},
        {BlockFaces::Back, {
                               AOTriplet{IVec3(1, 0, -1), IVec3(0, -1, -1), IVec3(1, -1, -1)},   // Bottom-left vertex (from back face perspective)
                               AOTriplet{IVec3(-1, 0, -1), IVec3(0, -1, -1), IVec3(-1, -1, -1)}, // Bottom-right vertex
                               AOTriplet{IVec3(-1, 0, -1), IVec3(0, 1, -1), IVec3(-1, 1, -1)},   // Top-right vertex
                               AOTriplet{IVec3(1, 0, -1), IVec3(0, 1, -1), IVec3(1, 1, -1)}      // Top-left vertex
                           }},
        {BlockFaces::Left, {
                               AOTriplet{IVec3(-1, 0, -1), IVec3(-1, -1, 0), IVec3(-1, -1, -1)}, // Bottom-left vertex
                               AOTriplet{IVec3(-1, 0, 1), IVec3(-1, -1, 0), IVec3(-1, -1, 1)},   // Bottom-right vertex
                               AOTriplet{IVec3(-1, 0, 1), IVec3(-1, 1, 0), IVec3(-1, 1, 1)},     // Top-right vertex
                               AOTriplet{IVec3(-1, 0, -1), IVec3(-1, 1, 0), IVec3(-1, 1, -1)}    // Top-left vertex
                           }},
        {BlockFaces::Right, {
                                AOTriplet{IVec3(1, 0, 1), IVec3(1, -1, 0), IVec3(1, -1, 1)},   // Bottom-left vertex
                                AOTriplet{IVec3(1, 0, -1), IVec3(1, -1, 0), IVec3(1, -1, -1)}, // Bottom-right vertex
                                AOTriplet{IVec3(1, 0, -1), IVec3(1, 1, 0), IVec3(1, 1, -1)},   // Top-right vertex
                                AOTriplet{IVec3(1, 0, 1), IVec3(1, 1, 0), IVec3(1, 1, 1)}      // Top-left vertex
                            }},
        {BlockFaces::Top, {
                              AOTriplet{IVec3(-1, 1, 0), IVec3(0, 1, 1), IVec3(-1, 1, 1)},  // Bottom-left vertex (front-left from top view)
                              AOTriplet{IVec3(1, 1, 0), IVec3(0, 1, 1), IVec3(1, 1, 1)},    // Bottom-right vertex (front-right from top view)
                              AOTriplet{IVec3(1, 1, 0), IVec3(0, 1, -1), IVec3(1, 1, -1)},  // Top-right vertex (back-right from top view)
                              AOTriplet{IVec3(-1, 1, 0), IVec3(0, 1, -1), IVec3(-1, 1, -1)} // Top-left vertex (back-left from top view)
                          }},
        {BlockFaces::Bottom, {
                                 AOTriplet{IVec3(-1, -1, 0), IVec3(0, -1, -1), IVec3(-1, -1, -1)}, // Bottom-left vertex (back-left from bottom view)
                                 AOTriplet{IVec3(1, -1, 0), IVec3(0, -1, -1), IVec3(1, -1, -1)},   // Bottom-right vertex (back-right from bottom view)
                                 AOTriplet{IVec3(1, -1, 0), IVec3(0, -1, 1), IVec3(1, -1, 1)},     // Top-right vertex (front-right from bottom view)
                                 AOTriplet{IVec3(-1, -1, 0), IVec3(0, -1, 1), IVec3(-1, -1, 1)}    // Top-left vertex (front-left from bottom view)
                             }},
    };

    inline const std::array<unsigned int, 6> quadIndices = {0, 1, 2, 2, 3, 0};

    inline constexpr std::array<glm::ivec3, 6> FACE_OFFSETS = {{
        {1, 0, 0},  // Right
        {-1, 0, 0}, // Left
        {0, 1, 0},  // Top
        {0, -1, 0}, // Bottom
        {0, 0, 1},  // Front
        {0, 0, -1}  // Back
    }};
};