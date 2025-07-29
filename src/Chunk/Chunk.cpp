#include "Chunk/Chunk.h"
#include "Chunk/ChunkManager.h"
#include "Chunk/ChunkPipeline.h"
#include "Block/BlockTypes.h"
#include "FastNoiseLite.h"
#include "Performance/ScopedTimer.h"
#include "Block/BlockFaceData.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <array>
#include <vector>
#include <cmath>
#include <functional>

Chunk::Chunk(Shader &chunkShader, TextureAtlas &atlas, ChunkCoord pos)
    : chunkShader_(chunkShader), textureAtlas_(atlas), chunkCoord_(pos), indexCount_(0), vertexCount_(0)
{
    const int chunkSize_X = Constants::CHUNK_SIZE_X;
    const int chunkSize_Y = Constants::CHUNK_SIZE_Y;
    const int chunkSize_Z = Constants::CHUNK_SIZE_Z;

    blocks_.resize(chunkSize_X * chunkSize_Y * chunkSize_Z);

    vertices_.reserve(chunkSize_X * chunkSize_Y * chunkSize_Z * 4); // Max 4 unique vertices per face
    indices_.reserve(chunkSize_X * chunkSize_Y * chunkSize_Z * 6);  // Max 6 indices per face

    boundingBox_.min = glm::vec3(chunkCoord_.x * chunkSize_X, 0, chunkCoord_.z * chunkSize_Z);
    boundingBox_.max = glm::vec3(chunkCoord_.x * chunkSize_X + chunkSize_X, chunkSize_Y, chunkCoord_.z * chunkSize_Z + chunkSize_Z);

    configureVertexAttributes();
}

void Chunk::render()
{
    // make sure chunk is loaded
    if (!stateMachine_.isReady() || indexCount_ == 0)
        return;

    chunkShader_.use();

    vao_.bind();
    vbo_.bind();
    ebo_.bind();

    glm::mat4 model = glm::mat4(1.0f);
    modelMatrix_ = glm::translate(model, glm::vec3(chunkCoord_.x * Constants::CHUNK_SIZE_X, 0, chunkCoord_.z * Constants::CHUNK_SIZE_Z));

    chunkShader_.setMat4("model", modelMatrix_);

    glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, 0);
}

void Chunk::generateTerrain()
{
    // ScopedTimer timer("Chunk::generateTerrain");

    for (int x = 0; x < Constants::CHUNK_SIZE_X; x++)
    {
        for (int z = 0; z < Constants::CHUNK_SIZE_Z; z++)
        {
            for (int y = 0; y < Constants::CHUNK_SIZE_Y; y++)
            {

                float worldX = chunkCoord_.x * Constants::CHUNK_SIZE_X + x;
                float worldZ = chunkCoord_.z * Constants::CHUNK_SIZE_Z + z;

                float terrainNoise = terrainGen_.getTerrainNoise(worldX, worldZ); // [0 - 1]
                float centeredNoise = terrainNoise - 0.5f;                        // -0.5 to 0.5
                int height = Constants::TERRAIN_BASE_HEIGHT + (int)(centeredNoise * Constants::TERRAIN_HEIGHT_VARIATION * 2.0f);

                BlockType type;
                if (y > height)
                {
                    type = BlockType::Air;
                }
                else if (y == height)
                {
                    type = BlockType::Grass;
                }
                else if (y < height && y > Constants::STONE_LEVEL)
                {
                    type = BlockType::Dirt;
                }
                else
                {
                    type = BlockType::Stone;
                }

                // Cave generation
                float caveVal = terrainGen_.getCaveNoise(worldX, (float)y, worldZ);
                if (y < height - 5 &&
                    caveVal > Constants::CAVE_THRESHOLD &&
                    y > 0)
                {
                    type = BlockType::Air;
                }

                glm::ivec3 pos = {x, y, z};
                const size_t index = getBlockIndex(pos);
                blocks_[index] = Block(type, pos);
            }
        }
    }
}

void Chunk::generateMesh(std::array<std::shared_ptr<Chunk>, 4> neighborChunks)
{
    vertices_.clear();
    indices_.clear();

    // loop through chunk and generate each blocks mesh
    for (int x = 0; x < Constants::CHUNK_SIZE_X; x++)
    {
        for (int y = 0; y < Constants::CHUNK_SIZE_Y; y++)
        {
            for (int z = 0; z < Constants::CHUNK_SIZE_Z; z++)
            {
                glm::ivec3 pos = glm::ivec3(x, y, z);
                const size_t index = getBlockIndex(pos);
                const Block &block = blocks_[index];

                if (block.type == BlockType::Air)
                    continue;
                if (isBlockHidden(pos))
                    continue;

                generateBlockMesh(block, neighborChunks);
            }
        }
    }

    vertices_.shrink_to_fit();
    indices_.shrink_to_fit();
}

void Chunk::uploadMeshToGPU()
{
    if (vertices_.empty() || indices_.empty())
    {
        vertexCount_ = 0;
        indexCount_ = 0;
        return;
    }

    vao_.bind();

    vbo_.setData(reinterpret_cast<const float *>(vertices_.data()), vertices_.size() * sizeof(Vertex));
    vertexCount_ = vertices_.size();

    indexCount_ = indices_.size();
    ebo_.setData(indices_.data(), indexCount_ * sizeof(unsigned int));

    vertices_.clear();
    indices_.clear();
}

void Chunk::removeBlockAt(glm::ivec3 pos)
{
    const size_t index = getBlockIndex(pos);
    blocks_[index] = Block(BlockType::Air, pos);
}

void Chunk::configureVertexAttributes()
{
    vao_.bind();
    vbo_.bind();
    ebo_.bind();

    VertexBufferLayout layout;
    layout.push<float>(3); // position
    layout.push<float>(2); // texture coords
    layout.push<float>(1); // AO
    vao_.addBuffer(vbo_, layout);
}

void Chunk::generateBlockMesh(const Block &block, const std::array<std::shared_ptr<Chunk>, 4> neighborChunks)
{
    // ================NEIGHBOR CACHING===================================
    BlockType cache[27];
    for (int i = 0; i < 27; i++)
    {
        glm::ivec3 offset = glm::ivec3(i % 3 - 1, (i / 3) % 3 - 1, i / 9 - 1);
        cache[i] = getNeighborBlockType(block.position, offset, neighborChunks);
    }

    // Helper to get neighbor from cache
    auto getNeighborTypeFromCache = [&](int x, int y, int z)
    {
        // Convert offset from [-1, 1] to [0, 2] for array indexing
        return cache[(x + 1) + (y + 1) * 3 + (z + 1) * 9];
    };
    // ========================================================================

    for (int f = 0; f < 6; f++)
    {
        const auto offset = BlockFaceData::FACE_OFFSETS[f];
        if (isTransparent(getNeighborTypeFromCache(offset.x, offset.y, offset.z)))
            generateFaceVertices(block, static_cast<BlockFaces>(f), getNeighborTypeFromCache);
    }
}

void Chunk::generateFaceVertices(const Block &block, BlockFaces face, const std::function<BlockType(int, int, int)> &getNeighborTypeFromCache)
{
    const auto &faceUVs = textureAtlas_.getBlockFaceUVs(block.type, face);
    const auto &corners = BlockFaceData::faceCorners.at(face);
    const auto &aoData = BlockFaceData::aoOffsets.at(face);

    // ao helper function
    auto computeAO = [&](const std::array<glm::ivec3, 3> &offsets)
    {
        bool side1 = !isTransparent(getNeighborTypeFromCache(offsets[0].x, offsets[0].y, offsets[0].z));
        bool side2 = !isTransparent(getNeighborTypeFromCache(offsets[1].x, offsets[1].y, offsets[1].z));
        bool corner = !isTransparent(getNeighborTypeFromCache(offsets[2].x, offsets[2].y, offsets[2].z));

        if (side1 && side2)
        {
            return 0.3f; // Darkest
        }
        int occlusion = side1 + side2 + corner;
        return 1.0f - occlusion * 0.2f; // Simple mapping: 1.0, 0.8, 0.6, 0.4
    };

    unsigned int baseVertexIndex = static_cast<unsigned int>(vertices_.size());
    // Make vertex for each corner of face
    for (int i = 0; i < 4; i++)
    {
        Vertex v;
        v.position = corners[i] + glm::vec3(block.position);
        v.textureCoords = faceUVs[i];
        v.ao = computeAO(aoData[i]);
        vertices_.push_back(v);
    }

    for (size_t i = 0; i < 6; i++)
    {
        indices_.push_back(baseVertexIndex + BlockFaceData::quadIndices[i]);
    }
}

BlockType Chunk::getNeighborBlockType(const glm::ivec3 blockPos, const glm::ivec3 offset, std::array<std::shared_ptr<Chunk>, 4> neighborChunks)
{
    const auto northChunk = neighborChunks[0];
    const auto southChunk = neighborChunks[1];
    const auto eastChunk = neighborChunks[2];
    const auto westChunk = neighborChunks[3];

    const glm::ivec3 neighborLocalPos = blockPos + offset;

    // if it's in the chunk, just get it
    if (blockInChunkBounds(neighborLocalPos))
    {
        return blocks_[getBlockIndex(neighborLocalPos)].type;
    }
    else
    {
        // Check which chunk block is in
        // Determine which neighboring chunk the block falls into and get its local coordinates

        if (neighborLocalPos.x < 0 &&
            neighborLocalPos.z >= 0 && neighborLocalPos.z < Constants::CHUNK_SIZE_Z &&
            neighborLocalPos.y >= 0 && neighborLocalPos.y < Constants::CHUNK_SIZE_Y)
        {
            if (westChunk)
            {
                glm::ivec3 localPosInNeighbor = {neighborLocalPos.x + Constants::CHUNK_SIZE_X, neighborLocalPos.y, neighborLocalPos.z};
                return westChunk->blocks_[getBlockIndex(localPosInNeighbor)].type;
            }
        }
        // Check East neighbor (+X direction)
        else if (neighborLocalPos.x >= Constants::CHUNK_SIZE_X &&
                 neighborLocalPos.z >= 0 && neighborLocalPos.z < Constants::CHUNK_SIZE_Z &&
                 neighborLocalPos.y >= 0 && neighborLocalPos.y < Constants::CHUNK_SIZE_Y)
        {
            if (eastChunk)
            {
                glm::ivec3 localPosInNeighbor = {neighborLocalPos.x - Constants::CHUNK_SIZE_X, neighborLocalPos.y, neighborLocalPos.z};
                return eastChunk->blocks_[getBlockIndex(localPosInNeighbor)].type;
            }
        }
        // Check South neighbor (-Z direction)
        else if (neighborLocalPos.z < 0 &&
                 neighborLocalPos.x >= 0 && neighborLocalPos.x < Constants::CHUNK_SIZE_X &&
                 neighborLocalPos.y >= 0 && neighborLocalPos.y < Constants::CHUNK_SIZE_Y)
        {
            if (southChunk)
            {
                glm::ivec3 localPosInNeighbor = {neighborLocalPos.x, neighborLocalPos.y, neighborLocalPos.z + Constants::CHUNK_SIZE_Z};
                return southChunk->blocks_[getBlockIndex(localPosInNeighbor)].type;
            }
        }
        // Check North neighbor (+Z direction)
        else if (neighborLocalPos.z >= Constants::CHUNK_SIZE_Z &&
                 neighborLocalPos.x >= 0 && neighborLocalPos.x < Constants::CHUNK_SIZE_X &&
                 neighborLocalPos.y >= 0 && neighborLocalPos.y < Constants::CHUNK_SIZE_Y)
        {
            if (northChunk)
            {
                glm::ivec3 localPosInNeighbor = {neighborLocalPos.x, neighborLocalPos.y, neighborLocalPos.z - Constants::CHUNK_SIZE_Z};
                return northChunk->blocks_[getBlockIndex(localPosInNeighbor)].type;
            }
        }

        // Fallback
        return BlockType::Air;
    }
}

const Block &Chunk::getBlockLocal(const glm::ivec3 &pos) const
{
    if (!blockInChunkBounds(pos))
    {
        // Out of bounds — return a static air block
        static Block airBlock(BlockType::Air, glm::vec3(0));
        return airBlock;
    }

    const size_t index = getBlockIndex(pos);
    return blocks_[index];
}

inline bool Chunk::isTransparent(BlockType type) const
{
    return type == BlockType::Air;
}

bool Chunk::isBlockHidden(const glm::ivec3 &pos)
{
    // Quick check if all 6 neighbors are solid blocks
    for (const auto &offset : BlockFaceData::FACE_OFFSETS)
    {
        glm::ivec3 neighborPos = pos + offset;
        if (blockInChunkBounds(neighborPos))
        {
            if (isTransparent(blocks_[getBlockIndex(neighborPos)].type))
            {
                return false;
            }
        }
        else
        {
            return false; // Edge blocks are never completely hidden
        }
    }
    return true;
}

inline size_t Chunk::getBlockIndex(const glm::ivec3 &pos) const
{
    return pos.x + (pos.y * Constants::CHUNK_SIZE_X) + (pos.z * Constants::CHUNK_SIZE_X * Constants::CHUNK_SIZE_Y);
}

inline bool Chunk::blockInChunkBounds(const glm::ivec3 &pos) const
{
    return pos.x >= 0 && pos.x < Constants::CHUNK_SIZE_X &&
           pos.y >= 0 && pos.y < Constants::CHUNK_SIZE_Y &&
           pos.z >= 0 && pos.z < Constants::CHUNK_SIZE_Z;
}

ChunkState Chunk::getState() const
{
    return stateMachine_.getState();
}

void Chunk::setState(ChunkState newState)
{
    stateMachine_.setState(newState);
}

bool Chunk::canUnload() const
{
    return stateMachine_.canUnload();
}

bool Chunk::isProcessing()
{
    return stateMachine_.isProcessing();
}

bool Chunk::canRemesh()
{
    return stateMachine_.canRemesh();
}

const ChunkCoord Chunk::getCoord() const
{
    return chunkCoord_;
}

const BoundingBox Chunk::getBoundingBox() const
{
    return boundingBox_;
}
