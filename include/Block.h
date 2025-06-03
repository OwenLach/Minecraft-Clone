#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>

#include "Constants.h"
#include "BlockTypes.h"

class Block
{
public:
    BlockType type;
    glm::ivec3 position;

    Block();
    Block(BlockType type, glm::ivec3 pos);
    std::vector<float> generateFacevertices(BlockFaces face, const std::vector<glm::vec2> &faceUVs) const;

private:
};