#include "World.h"
#include "Chunk/ChunkManager.h"
#include "Chunk/ChunkCoord.h"
#include "Block/Block.h"
#include "Block/BlockFaceData.h"
#include "Constants.h"
#include "Camera.h"

#include <iostream>
#include <algorithm>

#include <glm/glm.hpp>

World::World(Camera &camera)
    : camera_(camera),
      pipeline_(),
      chunkManager_(camera),
      lightSystem_(this, &chunkManager_),
      lastPlayerChunk_(worldToChunkCoords(glm::ivec3(camera_.Position - glm::vec3(1)))),
      raycaster(*this, camera)
{
    pipeline_.init(&chunkManager_, &lightSystem_);
    chunkManager_.init(&pipeline_);
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

    chunkManager_.update();
    updateSelectedBlockOutline();
}

void World::render()
{
    chunkManager_.renderAllChunks();
    if (hasTargetBlock_)
    {
        blockOutline_.render(camera_.getViewMatrix(), camera_.getProjectionMatrix(), targetBlockPos_);
    }
}

void World::breakBlock()
{
    // if (raycaster.cast())
    // {
    //     glm::ivec3 blockHitPos = raycaster.getHitBlockPosition();
    //     auto chunk = chunkManager_.getChunk(worldToChunkCoords(blockHitPos));
    //     if (chunk)
    //     {
    //         chunk->removeBlockAt(getBlockLocalPosition(blockHitPos));
    //         chunk->setState(ChunkState::NEEDS_LIGHT_UPDATE);
    //     }
    // }
}

void World::placeBlock()
{
    // if (raycaster.cast())
    // {
    //     glm::ivec3 blockHitPos = raycaster.getHitBlockPosition();
    //     BlockFaces hitFace = raycaster.getHitBlockFace();
    //     glm::ivec3 dir;

    //     switch (hitFace)
    //     {
    //     case BlockFaces::Front:
    //         dir = glm::ivec3(0, 0, 1);
    //         break;
    //     case BlockFaces::Back:
    //         dir = glm::ivec3(0, 0, -1);
    //         break;
    //     case BlockFaces::Left:
    //         dir = glm::ivec3(-1, 0, 0);
    //         break;
    //     case BlockFaces::Right:
    //         dir = glm::ivec3(1, 0, 0);
    //         break;
    //     case BlockFaces::Bottom:
    //         dir = glm::ivec3(0, -1, 0);
    //         break;
    //     case BlockFaces::Top:
    //         dir = glm::ivec3(0, 1, 0);
    //         break;
    //     }

    //     glm::ivec3 posToPlace = blockHitPos + dir;
    //     auto chunk = chunkManager_.getChunk(worldToChunkCoords(posToPlace));
    //     if (chunk)
    //     {
    //         chunk->setBlockAt(getBlockLocalPosition(posToPlace), playerBlockType_);
    //         chunk->setState(ChunkState::NEEDS_LIGHT_UPDATE);
    //     }
    // }
}

void World::setPlayerBlockType(BlockType type)
{
    playerBlockType_ = type;
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
        chunkManager_.addChunk(chunkCoord);
    }
}

void World::unloadDistantChunks()
{
    // const ChunkCoord playerPos = worldToChunkCoords(glm::ivec3(camera_.Position));
    // std::vector<ChunkCoord> chunkCoordsToRemove;

    // // Pass to chunkManagers visitor function which checks if every coord/chunk pair
    // chunkManager_.forEachChunk([&](const ChunkCoord coord, std::shared_ptr<Chunk> chunk) { //
    //     // if (!isInRenderDistance(coord.x, coord.z, playerPos.x, playerPos.z) && chunk->canUnload())
    //     if (!isInRenderDistance(coord.x, coord.z, playerPos.x, playerPos.z))
    //         chunkCoordsToRemove.push_back(coord);
    // });

    // // Remove chunks outside of lock
    // for (const auto &coord : chunkCoordsToRemove)
    // {
    //     chunkManager_.removeChunk(coord);
    // }
}

void World::updateSelectedBlockOutline()
{
    if (raycaster.cast())
    {
        // Global hit position
        auto blockHitPos = raycaster.getHitBlockPosition();
        Block *blockPtr = getBlockGlobal(blockHitPos);

        if (!blockPtr)
        {
            throw std::runtime_error("Block not found at " +
                                     std::to_string(blockHitPos.x) + ", " +
                                     std::to_string(blockHitPos.y) + ", " +
                                     std::to_string(blockHitPos.z));
        }

        Block &block = *blockPtr;
        if (block.type != BlockType::Air)
        {
            targetBlockPos_ = blockHitPos;
            hasTargetBlock_ = true;
        }
    }
    // Only set it false if its true
    else if (hasTargetBlock_)
    {
        hasTargetBlock_ = false;
    }
}

const std::shared_ptr<Chunk> World::getChunk(const ChunkCoord &coord) const
{
    return chunkManager_.getChunk(coord);
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

glm::ivec3 World::localToGlobalPos(ChunkCoord chunkCoords, glm::ivec3 localPos) const
{
    return glm::ivec3(chunkCoords.x * Constants::CHUNK_SIZE_X + localPos.x,
                      localPos.y,
                      chunkCoords.z * Constants::CHUNK_SIZE_Z + localPos.z);
}

Block *World::getBlockGlobal(const glm::ivec3 worldPos) const
{

    auto localBlockPos = getBlockLocalPosition(worldPos);
    ChunkCoord chunkCoord = worldToChunkCoords(worldPos);
    auto chunkPtr = chunkManager_.getChunk(chunkCoord);
    return (chunkPtr) ? chunkPtr->getBlockLocal(localBlockPos) : nullptr;
}

bool World::isBlockSolid(glm::ivec3 blockWorldPos) const
{
    Block *block = getBlockGlobal(blockWorldPos);
    return block->type != BlockType::Air;
}