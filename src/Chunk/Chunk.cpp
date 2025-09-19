#include "Chunk/Chunk.h"
#include "Block/BlockTypes.h"
#include "Block/BlockFaceData.h"
#include "Performance/ScopedTimer.h"

#include <glm/glm.hpp>

#include <iostream>
#include <vector>

Chunk::Chunk(Shader &chunkShader, TextureAtlas &atlas, ChunkCoord pos)
    : mesh_(chunkShader), textureAtlas_(atlas), chunkCoord_(pos)
{
    const int chunkSize_X = Constants::CHUNK_SIZE_X;
    const int chunkSize_Y = Constants::CHUNK_SIZE_Y;
    const int chunkSize_Z = Constants::CHUNK_SIZE_Z;

    blocks_.resize(chunkSize_X * chunkSize_Y * chunkSize_Z);

    boundingBox_.min = glm::vec3(chunkCoord_.x * chunkSize_X, 0, chunkCoord_.z * chunkSize_Z);
    boundingBox_.max = glm::vec3(chunkCoord_.x * chunkSize_X + chunkSize_X, chunkSize_Y, chunkCoord_.z * chunkSize_Z + chunkSize_Z);
}

void Chunk::generateTerrain()
{
    using namespace Constants;

    for (int x = 0; x < CHUNK_SIZE_X; x++)
    {
        for (int z = 0; z < CHUNK_SIZE_Z; z++)
        {
            for (int y = 0; y < CHUNK_SIZE_Y; y++)
            {

                float worldX = chunkCoord_.x * CHUNK_SIZE_X + x;
                float worldZ = chunkCoord_.z * CHUNK_SIZE_Z + z;

                float terrainNoise = terrainGen_.getTerrainNoise(worldX, worldZ); // [0 - 1]
                float centeredNoise = terrainNoise - 0.5f;                        // -0.5 to 0.5
                int height = TERRAIN_BASE_HEIGHT + (int)(centeredNoise * TERRAIN_HEIGHT_VARIATION * 2.0f);

                BlockType type;
                if (y > height)
                {
                    type = BlockType::Air;
                }
                else if (y == height)
                {
                    type = BlockType::Grass;
                }
                else if (y < height && y > STONE_LEVEL)
                {
                    type = BlockType::Dirt;
                }
                else
                {
                    type = BlockType::Stone;
                }

                // Cave generation
                float caveVal = terrainGen_.getCaveNoise(worldX, (float)y, worldZ);
                if (y <= height && caveVal > CAVE_THRESHOLD && y > 0)
                    type = BlockType::Air;

                glm::ivec3 pos = {x, y, z};
                const size_t index = getBlockIndex(pos);
                blocks_[index] = Block(type, pos);
            }
        }
    }
}

void Chunk::removeBlockAt(glm::ivec3 pos)
{
    const size_t index = getBlockIndex(pos);
    blocks_[index] = Block(BlockType::Air, pos);
}

void Chunk::setBlockAt(glm::ivec3 pos, BlockType type)
{
    const size_t index = getBlockIndex(pos);
    blocks_[index] = Block(type, pos);
}

Block *Chunk::getBlockLocal(const glm::ivec3 &pos)
{
    if (!blockPosInChunkBounds(pos))
        return nullptr;

    const size_t index = getBlockIndex(pos);
    return &blocks_[index];
}

std::vector<Block> &Chunk::getBlocks()
{
    return blocks_;
}

ChunkMesh &Chunk::getMesh()
{
    return mesh_;
}

void Chunk::setMeshData(MeshData &newMeshData)
{
    mesh_.meshData_ = newMeshData;
    mesh_.setMeshValid();
}

const ChunkCoord Chunk::getCoord() const

{
    return chunkCoord_;
}

const BoundingBox Chunk::getBoundingBox() const
{
    return boundingBox_;
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
