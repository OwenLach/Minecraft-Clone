#include <glad/glad.h>
#include "Block.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <unordered_map>
#include <array>

Block::Block() : type(BlockType::Air), position(glm::ivec3(0.0f))
{
}

Block::Block(BlockType type, glm::ivec3 pos) : type(type), position(pos)
{
}

std::vector<float> Block::generateFacevertices(BlockFaces face, const std::vector<glm::vec2> &faceUVs) const
{
    using Vec3 = glm::vec3;
    using Vec2 = glm::vec2;

    static const std::unordered_map<BlockFaces, std::array<Vec3, 4>> faceCorners = {
        {BlockFaces::Front, {Vec3(-0.5f, -0.5f, 0.5f), Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f)}},
        {BlockFaces::Back, {Vec3(0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f)}},
        {BlockFaces::Left, {Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(-0.5f, 0.5f, -0.5f)}},
        {BlockFaces::Right, {Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, -0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, 0.5f)}},
        {BlockFaces::Top, {Vec3(-0.5f, 0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Vec3(0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f)}},
        {BlockFaces::Bottom, {Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.5f, -0.5f, -0.5f), Vec3(0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f)}},
    };

    static const std::array<unsigned int, 6> quadIndices = {0, 1, 2, 2, 3, 0};

    // Error if invalid UV count
    if (faceUVs.size() != 6)
        throw std::runtime_error("faceUVs must contain exactly 6 elements (6 vertices)");

    const auto &corners = faceCorners.at(face);

    std::vector<float> vertices;
    vertices.reserve(6 * (3 + 2)); // 6 vertices, each with 3 position + 2 UV

    for (size_t i = 0; i < 6; ++i)
    {
        const Vec3 &pos = corners[quadIndices[i]] + glm::vec3(position);
        const Vec2 &uv = faceUVs[i];

        vertices.insert(vertices.end(), {pos.x, pos.y, pos.z, uv.x, uv.y});
    }

    return vertices;
}
