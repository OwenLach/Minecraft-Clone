#pragma once

#include "Block/BlockTypes.h"

class Block
{
public:
    BlockType type;
    glm::ivec3 position;

    Block();
    Block(BlockType type, glm::ivec3 pos);
};