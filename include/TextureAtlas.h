#pragma once

#include "BlockTypes.h"
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
    std::vector<glm::vec2> getFaceUVs(BlockType type, BlockFaces face);

private:
    std::unordered_map<BlockType, BlockTextureAtlasIndicies> blockIndicies;

    void initIndicies();
    std::vector<glm::vec2> getTileUVs(int tileX, int tileY);
};