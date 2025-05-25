#pragma once

#include <vector>
#include <memory>

#include "Block.h"
#include "Constants.h"
#include "TextureAtlas.h"
#include "Shader.h"

class Chunk
{
public:
    Chunk(Shader &shader, TextureAtlas *atlas);
    void renderChunk();

private:
    Shader &shader;
    TextureAtlas *textureAtlas;
    std::unique_ptr<Block> blocksPtr[Constants::CHUNK_SIZE_X][Constants::CHUNK_SIZE_Y][Constants::CHUNK_SIZE_Z];
};