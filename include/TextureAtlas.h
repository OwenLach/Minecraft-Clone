#pragma once

#include "Block/BlockTypes.h"
#include "Block/BlockFaceData.h"

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>

class TextureAtlas
{
public:
    unsigned int ID;
    int atlasWidth;
    int atlasHeight;
    int tileSize;

    TextureAtlas();
    const std::array<glm::vec2, 4> &getBlockFaceUVs(BlockType type, BlockFaces face);

private:
    std::unordered_map<BlockType, BlockUVs> blockUVsMap;

    void initBlockUVs();
    std::array<glm::vec2, 4> getTileUVs(int tileX, int tileY);
};