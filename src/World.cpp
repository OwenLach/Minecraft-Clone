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
#include <iostream>
#include <algorithm>

World::World(Camera &camera, Shader &shader, TextureAtlas &textureAtlas)
    : camera_(camera),
      shader_(shader),
      textureAtlas_(textureAtlas),
      chunkManager_(shader, camera, textureAtlas),
      pipeline_(chunkManager_),
      lastPlayerChunk_(worldToChunkCoords(glm::ivec3(camera_.Position - glm::vec3(1)))),
      raycaster(*this, camera)
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

    updateChunkStates();
    pipeline_.processMeshes();
    pipeline_.processGPUUploads();
}

void World::render()
{
    chunkManager_.render();
}

void World::breakBlock()
{
    if (raycaster.cast())
    {
        glm::ivec3 blockHitPos = raycaster.getHitBlockPosition();
        auto chunk = chunkManager_.getChunk(worldToChunkCoords(blockHitPos));
        if (chunk)
        {
            chunk->removeBlockAt(getBlockLocalPosition(blockHitPos));
            chunk->setState(ChunkState::NEEDS_MESH_REGEN);
            chunkManager_.markNeighborsForMeshRegeneration(chunk->getCoord());
        }
    }
    else
    {
        std::cout << "BLOCK MISSED" << std::endl;
    }
}

void World::placeBlock()
{
    std::cout << "PLACING BLOCK" << std::endl;
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

    for (const auto &[pos, chunk] : chunkManager_.getLoadedChunksCopy())
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

void World::updateChunkStates()
{
    for (const auto &[pos, chunk] : chunkManager_.getLoadedChunksCopy())
    {
        if (!chunk)
            continue;

        ChunkState state = chunk->getState();

        if (state == ChunkState::TERRAIN_READY && chunkManager_.allNeighborsTerrainReady(pos))
        {
            // IMPORTANT: Set state immediately to prevent re-queueing
            chunk->setState(ChunkState::MESH_GENERATING);
            pipeline_.queueInitialMesh(chunk);
        }
        else if (state == ChunkState::NEEDS_MESH_REGEN && chunkManager_.allNeighborsTerrainReady(pos))
        {
            chunk->setState(ChunkState::MESH_GENERATING);
            pipeline_.queueRemesh(chunk);
        }
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

glm::ivec3 World::getBlockLocalPosition(glm::ivec3 worldPos) const
{
    // Convert World coordinates to local chunk coordinates
    // Modulo math ensures correct wrapping even with negative positions
    // int localX = (worldCoords.x % Constants::CHUNK_SIZE_X + Constants::CHUNK_SIZE_X) % Constants::CHUNK_SIZE_X;
    // int localY = worldCoords.y;
    // int localZ = (worldCoords.z % Constants::CHUNK_SIZE_Z + Constants::CHUNK_SIZE_Z) % Constants::CHUNK_SIZE_Z;
    int localX = (worldPos.x % Constants::CHUNK_SIZE_X + Constants::CHUNK_SIZE_X) % Constants::CHUNK_SIZE_X;
    int localY = worldPos.y;
    int localZ = (worldPos.z % Constants::CHUNK_SIZE_Z + Constants::CHUNK_SIZE_Z) % Constants::CHUNK_SIZE_Z;
    return glm::ivec3(localX, localY, localZ);
}

Block World::getBlockGlobal(glm::vec3 worldPos) const
{
    glm::ivec3 worldCoords = glm::ivec3(glm::floor(worldPos));

    // glm::ivec3 localBlockPos = glm::ivec3(localX, localY, localZ);
    auto localBlockPos = getBlockLocalPosition(worldCoords);
    ChunkCoord chunkCoord = worldToChunkCoords(worldCoords);
    auto chunkPtr = chunkManager_.getChunk(chunkCoord);
    if (chunkPtr)
    {
        return chunkPtr->getBlockLocal(localBlockPos);
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