#pragma once
#include <glm/glm.hpp>

enum BlockType
{
    Air,
    Grass,
    Stone,
    Cobblestone,
    CryingObsidian
};

struct UVCoords
{
    glm::vec2 topLeft;
    glm::vec2 topRight;
    glm::vec2 bottomLeft;
    glm::vec2 bottomRight;
};

struct BlockTextureUVs
{
    UVCoords top;
    UVCoords side;
    UVCoords bottom;
};

struct BlockTextureAtlasIndicies
{
    glm::vec2 top;
    glm::vec2 side;
    glm::vec2 bottom;
};