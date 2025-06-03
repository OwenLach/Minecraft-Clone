#pragma once
#include <glm/glm.hpp>

enum BlockType
{
    Air,
    Grass,
    Stone,
    Cobblestone,
    CryingObsidian,
    Dirt
};

enum class BlockFaces
{
    Right,
    Left,
    Top,
    Bottom,
    Front,
    Back
};

struct BlockTextureAtlasIndicies
{
    glm::vec2 top;
    glm::vec2 side;
    glm::vec2 bottom;
};