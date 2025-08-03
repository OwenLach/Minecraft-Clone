#pragma once

#include "Block/BlockTypes.h"

class Block
{
public:
    BlockType type;
    glm::ivec3 position;
    uint8_t skylight; // 0-15  15 = max brightness

    Block();
    Block(BlockType type, glm::ivec3 pos);
};