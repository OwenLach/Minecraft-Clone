#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Chunk.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "BlockTypes.h"
#include <iostream>

Chunk::Chunk(Shader &shader, TextureAtlas *atlas)
    : shader(shader), textureAtlas(atlas)
{
    // Instead of creating individual Block objects, you'll iterate through the chunk's block positions.
    // For each block position, you'll still determine the BlockType and get the texture UVs.
    // Then, you'll generate the vertex data for that block,
    // but instead of storing it in a Block object, you'll append it to the chunkVertices vector.
    // Make sure to offset the vertex positions by the block's world position within the chunk.

    for (int x = 0; x < Constants::CHUNK_SIZE_X; x++)
    {
        for (int y = 0; y < Constants::CHUNK_SIZE_Y; y++)
        {
            for (int z = 0; z < Constants::CHUNK_SIZE_Z; z++)
            {
                // determine block type
                BlockType type;
                if (y > Constants::CHUNK_SIZE_Y / 2)
                {
                    type = BlockType::Grass;
                }
                else
                {
                    type = BlockType::Stone;
                }

                Block block(type, glm::vec3(x, y, z));
                BlockTextureUVs textureUVs = atlas->getUVcoords(type);
                std::vector<float> blockVerts = block.generateVerticies(textureUVs);
                blockVerticies.insert(blockVerticies.end(), blockVerts.begin(), blockVerts.end());
            }
        }
    }

    setupChunkMesh();
}

void Chunk::renderChunk()
{
    shader.use();
    vao.bind();

    // need to set model matrix
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);

    int numVerticies = blockVerticies.size() * sizeof(float);
    glDrawArrays(GL_TRIANGLES, 0, numVerticies);
}

void Chunk::setupChunkMesh()
{
    vao.bind();
    vbo.setData(blockVerticies.data(), blockVerticies.size() * sizeof(float));
    vbo.bind();

    VertexBufferLayout layout;
    layout.push<float>(3); // position
    layout.push<float>(2); // texture coords
    vao.addBuffer(vbo, layout);
}
