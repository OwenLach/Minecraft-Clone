#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>

#include "Constants.h"
#include "BlockTypes.h"

class Block
{
public:
    BlockType block_type;
    /**
     * @brief Constructs a Block object that holds it's type and it's position
     *
     * @param type The type of the block (e.g., Grass, Stone, Air).
     * @param pos The world-space position of the block as a 3D vector.
     *
     */
    Block(BlockType type, glm::vec3 pos);
    std::vector<float> generateVerticies(BlockTextureUVs textureUVs);

private:
    glm::vec3 position;
};