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
    : mesh_(chunkShader), textureAtlas_(atlas), chunkCoord_(pos)
{
    const int chunkSize_X = Constants::CHUNK_SIZE_X;
    const int chunkSize_Y = Constants::CHUNK_SIZE_Y;
    const int chunkSize_Z = Constants::CHUNK_SIZE_Z;

    blocks_.resize(chunkSize_X * chunkSize_Y * chunkSize_Z);

    boundingBox_.min = glm::vec3(chunkCoord_.x * chunkSize_X, 0, chunkCoord_.z * chunkSize_Z);
    boundingBox_.max = glm::vec3(chunkCoord_.x * chunkSize_X + chunkSize_X, chunkSize_Y, chunkCoord_.z * chunkSize_Z + chunkSize_Z);
}

void Chunk::render()
{
    if (mesh_.hasValidMesh_)
        mesh_.render(chunkCoord_);
}

void Chunk::generateTerrain()
{
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

Block &Chunk::getBlockLocal(const glm::ivec3 &pos)
{
    if (!blockPosInChunkBounds(pos))
    {
        // Out of bounds — return a static air block
        static Block airBlock(BlockType::Air, glm::vec3(0));
        return airBlock;
    }

    const size_t index = getBlockIndex(pos);
    return blocks_[index];
}

inline size_t Chunk::getBlockIndex(const glm::ivec3 &pos) const
{
    return pos.x + (pos.y * Constants::CHUNK_SIZE_X) + (pos.z * Constants::CHUNK_SIZE_X * Constants::CHUNK_SIZE_Y);
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
