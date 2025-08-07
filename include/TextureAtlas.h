#pragma once

#include "Block/BlockTypes.h"
#include "Block/BlockFaceData.h"

#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

class TextureAtlas
{
public:
    unsigned int ID_;

    TextureAtlas();
    void bindUnit(unsigned int unit);
    const std::array<glm::vec2, 4> &getBlockFaceUVs(BlockType type, BlockFaces face) const;

private:
    int atlasWidth_;
    int atlasHeight_;
    int tileSize_;
    std::unordered_map<BlockType, BlockUVs> blockUVsMap_;

    void initBlockUVs();
    std::array<glm::vec2, 4> getTileUVs(int tileX, int tileY);
};