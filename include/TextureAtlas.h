#pragma once

#include "BlockTypes.h"
#include <glm/glm.hpp>
#include <unordered_map>

class TextureAtlas
{
public:
    unsigned int ID;
    int atlasWidth;
    int atlasHeight;
    int tileSize;

    TextureAtlas();
    // returns the top, side, and bottom uv coords for each face of a block
    BlockTextureUVs getUVcoords(BlockType type);

private:
    std::unordered_map<BlockType, BlockTextureAtlasIndicies> blockIndicies;
    void initIndicies();
    UVCoords getTileUVs(int tileX, int tileY);
};