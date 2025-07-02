#include "World.h"
#include "Chunk/ChunkManager.h"
#include "Chunk/ChunkPipeline.h"
#include "Chunk/ChunkCoord.h"
#include "Block.h"
#include "Constants.h"
#include "Camera.h"
#include "Shader.h"
#include "TextureAtlas.h"

#include <glm/glm.hpp>

#include <algorithm>

World::World(Camera &camera, Shader &shader, TextureAtlas &textureAtlas)
    : camera_(camera),
      shader_(shader),
      textureAtlas_(textureAtlas),
      chunkManager_(shader, camera, textureAtlas),
      pipeline_(chunkManager_),
      lastPlayerChunk_(worldToChunkCoords(glm::ivec3(camera_.Position - glm::vec3(1))))
{
}

void World::update()
{
    auto currChunk = worldToChunkCoords(glm::ivec3(camera_.Position));
    if (currChunk != lastPlayerChunk_)
    {
        loadNewChunks(currChunk);
        unloadDistantChunks();
        lastPlayerChunk_ = currChunk;
    }

    pipeline_.processGPUUploads();
}

void World::render()
{
    chunkManager_.render();
}

void World::loadNewChunks(ChunkCoord center)
{
    const int R = Constants::RENDER_DISTANCE;
    std::vector<ChunkCoord> chunkCoordsToLoad;
    for (int dx = -R; dx < R; dx++)
    {
        for (int dz = -R; dz < R; dz++)
        {
            // Check cylindrical distance
            if (dx * dx + dz * dz > R * R)
                continue;

            ChunkCoord coord = {center.x + dx, center.z + dz};
            if (!chunkManager_.getChunk(coord))
                chunkCoordsToLoad.push_back(coord);
        }
    }

    std::sort(chunkCoordsToLoad.begin(), chunkCoordsToLoad.end(),
              [&](const ChunkCoord &a, const ChunkCoord &b)
              {
                  int da = std::abs(a.x - center.x) + std::abs(a.z - center.z);
                  int db = std::abs(b.x - center.x) + std::abs(b.z - center.z);
                  return da < db;
              });

    for (const auto &chunkCoord : chunkCoordsToLoad)
    {
        auto chunk = std::make_shared<Chunk>(shader_, textureAtlas_, chunkCoord, chunkManager_, pipeline_);
        chunkManager_.addChunk(chunkCoord, chunk);
        pipeline_.generateTerrain(chunk);
    }
}

void World::unloadDistantChunks()
{
    const ChunkCoord playerPos = worldToChunkCoords(glm::ivec3(camera_.Position));
    std::vector<ChunkCoord> chunkCoordsToRemove;

    for (const auto &[pos, chunk] : chunkManager_.getLoadedChunks())
    {
        // if chunk is in the middle of processing anything, don't unload
        if (!isInRenderDistance(pos.x, pos.z, playerPos.x, playerPos.z) && chunk->canUnload())
        {
            chunkCoordsToRemove.push_back(pos);
        }
    }

    for (const auto &coord : chunkCoordsToRemove)
    {
        chunkManager_.removeChunk(coord);
    }
}

glm::ivec3 World::chunkToWorldCoords(ChunkCoord chunkCoords, glm::ivec3 localPos) const
{
    int x = chunkCoords.x * Constants::CHUNK_SIZE_X + localPos.x;
    int y = localPos.y;
    int z = chunkCoords.z * Constants::CHUNK_SIZE_Z + localPos.z;
    return glm::ivec3(x, y, z);
}

ChunkCoord World::worldToChunkCoords(glm::ivec3 worldCoords) const
{
    int chunkX = glm::floor(worldCoords.x / (float)Constants::CHUNK_SIZE_X);
    int chunkZ = glm::floor(worldCoords.z / (float)Constants::CHUNK_SIZE_Z);
    return {chunkX, chunkZ};
}

Block World::getBlockLocal(ChunkCoord chunkCoords, glm::vec3 blockPos)
{
    auto chunkPtr = chunkManager_.getChunk(chunkCoords);
    if (!chunkPtr)
    {
        static Block airBlock;
        return airBlock;
    }
    return chunkPtr->getBlockLocal(blockPos);
}

Block World::getBlockGlobal(glm::vec3 worldPos) const
{
    // Round the World position down to the nearest integers to get block-aligned coordinates
    glm::ivec3 worldCoords = glm::floor(worldPos);
    // Convert World coordinates to chunk coordinates
    ChunkCoord chunkCoord = worldToChunkCoords(worldCoords);
    // Convert World coordinates to local chunk coordinates
    // Modulo math ensures correct wrapping even with negative positions
    int localX = (worldCoords.x % Constants::CHUNK_SIZE_X + Constants::CHUNK_SIZE_X) % Constants::CHUNK_SIZE_X;
    int localY = worldCoords.y;
    int localZ = (worldCoords.z % Constants::CHUNK_SIZE_Z + Constants::CHUNK_SIZE_Z) % Constants::CHUNK_SIZE_Z;

    glm::ivec3 localBlockPos = glm::ivec3(localX, localY, localZ);

    auto chunkPtr = chunkManager_.getChunk(chunkCoord);
    if (chunkPtr)
    {
        return chunkPtr->getBlockLocal(glm::ivec3(localX, localY, localZ));
    }
    else
    {
        // return a default air block if chunk isn't found
        return Block(BlockType::Air, glm::ivec3(0));
    }
}

bool World::isBlockSolid(glm::ivec3 blockWorldPos) const
{
    Block block = getBlockGlobal(glm::vec3(blockWorldPos));
    return block.type != BlockType::Air;
}

// void ChunkManager::markNeighborChunksForMeshRegeneration(const ChunkCoord &coord)
// {
//     // Return if the nei
//     if (!allNeighborsLoaded(coord))
//     {
//         return;
//     }

//     auto neighbors = getChunkNeighbors(coord);
//     for (const auto &n_chunkPtr : neighbors)
//     {
//         // The neighboring chunk exists and all its neighbors at least have their terrain ready && and the chunk can reme
//         if (n_chunkPtr && allNeighborsLoaded(n_chunkPtr->getCoord()) && n_chunkPtr->canRemesh())
//         {
//             // Mesh regeneration
//             bool expected = false;
//             if (n_chunkPtr->isDirty_.compare_exchange_strong(expected, true))
//             {
//                 n_chunkPtr->setState(ChunkState::NEEDS_MESH_REGEN);
//                 pipeline_->enqueueForRegen(n_chunkPtr);
//             }
//         }
//     }
// }

// bool ChunkManager::allNeighborsLoaded(const ChunkCoord &coord)
// {
//     auto neighbors = getChunkNeighbors(coord);

//     for (auto &n_chunkPtr : neighbors)
//     {
//         if (!n_chunkPtr || n_chunkPtr->getState() < ChunkState::TERRAIN_READY)
//         {
//             return false;
//         }
//     }
//     return true;
// }