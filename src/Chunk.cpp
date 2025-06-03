#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "Chunk.h"
#include "BlockTypes.h"
#include "World.h"

Chunk::Chunk(Shader &shader, TextureAtlas *atlas, ChunkCoord pos, World *world)
    : shader(shader), textureAtlas(atlas), worldPos(pos), world(world)
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

    model = glm::translate(model, glm::vec3(worldPos.x * Constants::CHUNK_SIZE_X, 0, worldPos.z * Constants::CHUNK_SIZE_Z));
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
    if (!blockInBounds(pos))
    {
        // Out of bounds — return a static air block
        static Block airBlock(BlockType::Air, glm::vec3(0));
        return airBlock;
        std::cout << "BLOCK NOT IN BOUND" << std::endl;
    }

    size_t index = pos.x + (pos.y * Constants::CHUNK_SIZE_X) + (pos.z * Constants::CHUNK_SIZE_X * Constants::CHUNK_SIZE_Y);
    return blocks[index];
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
                Block currBlock(type, glm::ivec3(x, y, z));
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
        glm::ivec3 offset = getFaceOffset(face);
        glm::ivec3 neighborLocalPos = block.position + getFaceOffset(face);

        BlockType neighborType = BlockType::Air; // Assume air by default

        if (neighborLocalPos.x < 0 || neighborLocalPos.x >= Constants::CHUNK_SIZE_X ||
            neighborLocalPos.y < 0 || neighborLocalPos.y >= Constants::CHUNK_SIZE_Y ||
            neighborLocalPos.z < 0 || neighborLocalPos.z >= Constants::CHUNK_SIZE_Z)
        {
            // Neighbor is in an adjacent chunk
            glm::ivec3 currBlockWorldCoords = world->chunkToWorldCoords(worldPos, block.position);
            glm::ivec3 neighborWorldPos = currBlockWorldCoords + offset;
            neighborType = world->getBlockAt(neighborWorldPos).type;
        }
        else
        {
            // Neighbor is in the same chunk
            neighborType = getBlockAt(neighborLocalPos).type;
        }

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

glm::ivec3 Chunk::getFaceOffset(BlockFaces face) const
{
    switch (face)
    {
    case BlockFaces::Front:
        return glm::ivec3(0, 0, 1);
    case BlockFaces::Back:
        return glm::ivec3(0, 0, -1);
    case BlockFaces::Left:
        return glm::ivec3(-1, 0, 0);
    case BlockFaces::Right:
        return glm::ivec3(1, 0, 0);
    case BlockFaces::Top:
        return glm::ivec3(0, 1, 0);
    case BlockFaces::Bottom:
        return glm::ivec3(0, -1, 0);
    default:
        throw std::runtime_error("Invalid BlockFace");
    }
}
