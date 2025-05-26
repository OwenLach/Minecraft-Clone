#pragma once

#include <vector>
#include <memory>

#include "Block.h"
#include "Constants.h"
#include "TextureAtlas.h"
#include "Shader.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

class Chunk
{
public:
    Chunk(Shader &shader, TextureAtlas *atlas);
    void renderChunk();

private:
    VertexArray vao;
    VertexBuffer vbo;
    Shader &shader;
    TextureAtlas *textureAtlas;
    std::vector<float> blockVerticies;

    void setupChunkMesh();
};