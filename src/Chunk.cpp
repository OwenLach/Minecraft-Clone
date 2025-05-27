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
    blocks.reserve(Constants::CHUNK_SIZE_X * Constants::CHUNK_SIZE_Y * Constants::CHUNK_SIZE_Z);
    rebuildMesh();
    setupChunkMesh();
}

void Chunk::renderChunk()
{
    shader.use();
    vao.bind();

    // need to set model matrix
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);

    // 5 is for the 3 pos floats and 2 texture coord floats
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
}

bool Chunk::blockInBounds(glm::ivec3 pos) const
{
    return !(pos.x < 0 || pos.x >= Constants::CHUNK_SIZE_X || pos.y < 0 || pos.y >= Constants::CHUNK_SIZE_Y || pos.z < 0 || pos.z >= Constants::CHUNK_SIZE_Z);
}

const Block &Chunk::getBlockAt(glm::ivec3 pos) const
{
    static Block airBlock;
    if (!blockInBounds(pos))
        // return a default air block if its out of bounds
        return airBlock;

    // return blocks[pos.x + Constants::CHUNK_SIZE_X * (pos.y + Constants::CHUNK_SIZE_Y * pos.z)];
    return blocks[pos.x + (pos.y * Constants::CHUNK_SIZE_X) + (pos.z * Constants::CHUNK_SIZE_X * Constants::CHUNK_SIZE_Y)];
}

void Chunk::rebuildMesh()
{
    vertices.clear();
    std::vector<float> vertexData;
    std::vector<Vertex> meshvertices = generateChunkMesh();
    vertices = meshvertices;
    for (const auto &vertex : meshvertices)
    {
        vertexData.push_back(vertex.pos.x);
        vertexData.push_back(vertex.pos.y);
        vertexData.push_back(vertex.pos.z);
        vertexData.push_back(vertex.uvs.x);
        vertexData.push_back(vertex.uvs.y);
    }
    vbo.setData(vertexData.data(), vertexData.size() * sizeof(float));
}

void Chunk::setupChunkMesh()
{
    vao.bind();

    VertexBufferLayout layout;
    layout.push<float>(3); // position
    layout.push<float>(2); // texture coords
    vao.addBuffer(vbo, layout);
}

std::vector<Vertex> Chunk::generateChunkMesh()
{
    // clear blocks and push all blocks back
    blocks.clear();
    for (int x = 0; x < Constants::CHUNK_SIZE_X; x++)
    {
        for (int y = 0; y < Constants::CHUNK_SIZE_Y; y++)
        {
            for (int z = 0; z < Constants::CHUNK_SIZE_Z; z++)
            {
                BlockType type = (y < Constants::CHUNK_SIZE_Y / 3) ? BlockType::Stone : BlockType::Grass;
                Block currBlock(type, glm::vec3(x, y, z));
                blocks.push_back(currBlock);
            }
        }
    }

    // generates the meshes for the blocks using face culling
    std::vector<Vertex> meshVerts;
    for (int x = 0; x < Constants::CHUNK_SIZE_X; x++)
    {
        for (int y = 0; y < Constants::CHUNK_SIZE_Y; y++)
        {
            for (int z = 0; z < Constants::CHUNK_SIZE_Z; z++)
            {
                generateBlockMesh(getBlockAt(glm::ivec3(x, y, z)), meshVerts);
            }
        }
    }
    return meshVerts;
}

void Chunk::generateBlockMesh(const Block &block, std::vector<Vertex> &meshVerts)
{
    for (int faceIdx = 0; faceIdx < 6; faceIdx++)
    {
        BlockFaces face = static_cast<BlockFaces>(faceIdx);
        // check if the neighbor block touching the current face idx is air
        glm::ivec3 neighborPos = getNeighborPosition(block.position, face);
        BlockType neighborType = getBlockTypeAt(neighborPos);

        // if the neighbor type is air, render the current face
        if (neighborType == BlockType::Air)
        {
            std::vector<glm::vec2> faceUVs = textureAtlas->getFaceUVs(block.type, face);
            std::vector<float> rawvertices = block.generateFacevertices(face, faceUVs);

            for (unsigned int i = 0; i < rawvertices.size(); i += 5)
            {
                meshVerts.push_back({
                    {rawvertices[i], rawvertices[i + 1], rawvertices[i + 2]}, // pos
                    {rawvertices[i + 3], rawvertices[i + 4]}                  // uvs
                });
            }
        }
    }
}

BlockType Chunk::getBlockTypeAt(glm::ivec3 pos) const
{
    return getBlockAt(pos).type;
}

glm::ivec3 Chunk::getNeighborPosition(glm::ivec3 pos, BlockFaces face) const
{
    switch (face)
    {
    case BlockFaces::Front:
        return pos + glm::ivec3(0, 0, 1);
    case BlockFaces::Back:
        return pos + glm::ivec3(0, 0, -1);
    case BlockFaces::Left:
        return pos + glm::ivec3(-1, 0, 0);
    case BlockFaces::Right:
        return pos + glm::ivec3(1, 0, 0);
    case BlockFaces::Top:
        return pos + glm::ivec3(0, 1, 0);
    case BlockFaces::Bottom:
        return pos + glm::ivec3(0, -1, 0);
    default:
        throw std::runtime_error("Invalid BlockFace passed to getNeighborPosition.");
    }
}
