#pragma once
#include <glm/glm.hpp>

enum BlockType
{
    Air,
    Grass,
    Dirt,
    Stone,
    Cobblestone,
    Log,
    Plank,
    Brick

};

struct BlockTextureAtlasIndicies
{
    glm::vec2 top;
    glm::vec2 side;
    glm::vec2 bottom;
};