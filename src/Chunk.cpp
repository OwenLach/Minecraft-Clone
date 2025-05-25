#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Chunk.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

#include <iostream>

Chunk::Chunk(Shader &shader, TextureAtlas *atlas)
    : shader(shader), textureAtlas(atlas)
{

    for (int x = 0; x < Constants::CHUNK_SIZE_X; x++)
    {
        for (int y = 0; y < Constants::CHUNK_SIZE_Y; y++)
        {
            for (int z = 0; z < Constants::CHUNK_SIZE_Z; z++)
            {
                blocksPtr[x][y][z] = std::make_unique<Block>(BlockType::Grass, textureAtlas, glm::vec3(x, y, z), &shader);
            }
        }
    }
}

void Chunk::renderChunk()
{
    for (int x = 0; x < Constants::CHUNK_SIZE_X; x++)
    {
        for (int y = 0; y < Constants::CHUNK_SIZE_Y; y++)
        {
            for (int z = 0; z < Constants::CHUNK_SIZE_Z; z++)
            {
                blocksPtr[x][y][z]->render();
            }
        }
    }
}

void Chunk::setupChunkMesh()
{
}
