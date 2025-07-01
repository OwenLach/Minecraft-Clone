#include "Chunk/ChunkManager.h"
#include "Chunk/Chunk.h"
#include "Chunk/ChunkStateMachine.h"
#include "Chunk/ChunkPipeline.h"
#include "ThreadPool.h"
#include "FastNoiseLite.h"

#include <iostream>
#include <thread>
#include <algorithm>
#include <iostream>

ChunkManager::ChunkManager(Shader &shader, TextureAtlas *atlas, Camera &camera)
    : shader_(shader),
      textureAtlas_(atlas),
      camera_(camera),
      pipeline_(std::make_unique<ChunkPipeline>(*this))
{
    loadInitialChunks();
}

void ChunkManager::render()
{
    std::shared_lock<std::shared_mutex> lock(loadedChunksMutex_);
    for (auto &[pos, chunk] : loadedChunks_)
    {
        if (camera_.isAABBInFrustum(chunk->getBoundingBox()))
            chunk->render();
    }
}

void ChunkManager::update()
{
    int renderDistance = Constants::RENDER_DISTANCE;
    ChunkCoord playerPos = worldToChunkCoords(glm::ivec3(camera_.Position));

    // Load new chunks
    loadVisibleChunks(playerPos, renderDistance);

    // Processes the chunk pipeline
    pipeline_->processAll();

    // Unload chunks
    unloadDistantChunks(playerPos);
}

void ChunkManager::loadChunk(ChunkCoord coord)
{
    auto chunk = std::make_shared<Chunk>(shader_, textureAtlas_, coord, this);
    loadedChunks_.emplace(coord, chunk);

    // Chunk automatically starts out in EMPTY state;
    pipeline_->enqueueForTerrain(chunk);
}

void ChunkManager::unloadChunk(ChunkCoord coord)
{
    loadedChunks_.erase(coord);
}

void ChunkManager::loadInitialChunks()
{
    const int renderDistance = Constants::RENDER_DISTANCE;
    const ChunkCoord playerPos = worldToChunkCoords(glm::ivec3(camera_.Position));

    std::vector<ChunkCoord> orderedChunks;
    // loops in a square
    for (int x = -renderDistance; x < renderDistance; x++)
    {
        for (int z = -renderDistance; z < renderDistance; z++)
        {
            // circular render distance
            if (isInRenderDistance(x, z, playerPos.x, playerPos.z))
            {
                orderedChunks.push_back(ChunkCoord{playerPos.x + x, playerPos.z + z});
            }
        }
    }

    // sort by distance from player
    std::sort(orderedChunks.begin(), orderedChunks.end(), [&playerPos, this](const ChunkCoord &a, const ChunkCoord &b)
              { return getChunkDistanceSquaredFromPlayer(playerPos, a) < getChunkDistanceSquaredFromPlayer(playerPos, b); });

    {
        // load closest to player first
        std::unique_lock<std::shared_mutex> lock(loadedChunksMutex_);
        for (const auto &chunk : orderedChunks)
        {
            loadChunk(chunk);
        }
    }
}

void ChunkManager::loadVisibleChunks(const ChunkCoord &playerPos, const int renderDistance)
{
    std::vector<ChunkCoord> chunksToLoad;

    {
        std::shared_lock<std::shared_mutex> lock(loadedChunksMutex_);
        for (int x = -renderDistance; x < renderDistance; x++)
        {
            for (int z = -renderDistance; z < renderDistance; z++)
            {
                ChunkCoord chunkCoord{playerPos.x + x, playerPos.z + z};

                // Check if chunk is within the circular render distance
                if (isInRenderDistance(chunkCoord.x, chunkCoord.z, playerPos.x, playerPos.z) &&
                    loadedChunks_.count(chunkCoord) == 0)
                {
                    chunksToLoad.push_back(chunkCoord);
                }
            }
        }
    }

    if (!chunksToLoad.empty())
    {
        std::unique_lock<std::shared_mutex> lock(loadedChunksMutex_);
        for (const auto &coord : chunksToLoad)
        {
            if (loadedChunks_.count(coord) == 0)
            {
                loadChunk(coord);
            }
        }
    }
}

void ChunkManager::unloadDistantChunks(const ChunkCoord &playerPos)
{
    std::vector<ChunkCoord> chunkPositionsToRemove;
    {
        // Shared for read only
        std::shared_lock<std::shared_mutex> lock(loadedChunksMutex_);
        for (auto &[pos, chunk] : loadedChunks_)
        {
            // if chunk is in the middle of processing anything, don't unload
            if (!isInRenderDistance(pos.x, pos.z, playerPos.x, playerPos.z) && chunk->canUnload())
            {
                chunkPositionsToRemove.push_back(pos);
            }
        }
    }

    if (!chunkPositionsToRemove.empty())
    {

        std::unique_lock<std::shared_mutex> lock(loadedChunksMutex_);
        for (const ChunkCoord coord : chunkPositionsToRemove)
        {
            unloadChunk(coord);
        }
    }
}

void ChunkManager::markNeighborChunksForMeshRegeneration(const ChunkCoord &coord)
{
    // Return if the nei
    if (!allNeighborsLoaded(coord))
    {
        return;
    }

    auto neighbors = getChunkNeighbors(coord);
    for (const auto &n_chunkPtr : neighbors)
    {
        // The neighboring chunk exists and all its neighbors at least have their terrain ready && and the chunk can reme
        if (n_chunkPtr && allNeighborsLoaded(n_chunkPtr->getCoord()) && n_chunkPtr->canRemesh())
        {
            // Mesh regeneration
            bool expected = false;
            if (n_chunkPtr->isDirty_.compare_exchange_strong(expected, true))
            {
                n_chunkPtr->setState(ChunkState::NEEDS_MESH_REGEN);
                pipeline_->enqueueForRegen(n_chunkPtr);
            }
        }
    }
}

std::array<std::shared_ptr<Chunk>, 4> ChunkManager::getChunkNeighbors(const ChunkCoord &coord)
{
    // Acquire a shared lock for thread-safe read access to loadedChunks_
    std::shared_lock<std::shared_mutex> lock(loadedChunksMutex_);

    std::array<std::shared_ptr<Chunk>, 4> neighbors;

    // North: +Z
    // South: -Z
    // East:  +X
    // West:  -X
    const ChunkCoord northCoord = {coord.x, coord.z + 1};
    const ChunkCoord southCoord = {coord.x, coord.z - 1};
    const ChunkCoord eastCoord = {coord.x + 1, coord.z};
    const ChunkCoord westCoord = {coord.x - 1, coord.z};

    auto it = loadedChunks_.find(northCoord);
    if (it != loadedChunks_.end())
    {
        neighbors[0] = it->second; // Assign North chunk (index 0)
    }

    it = loadedChunks_.find(southCoord);
    if (it != loadedChunks_.end())
    {
        neighbors[1] = it->second; // Assign South chunk (index 1)
    }

    it = loadedChunks_.find(eastCoord);
    if (it != loadedChunks_.end())
    {
        neighbors[2] = it->second; // Assign East chunk (index 2)
    }

    it = loadedChunks_.find(westCoord);
    if (it != loadedChunks_.end())
    {
        neighbors[3] = it->second; // Assign West chunk (index 3)
    }

    return neighbors;
}

bool ChunkManager::allNeighborsLoaded(const ChunkCoord &coord)
{
    auto neighbors = getChunkNeighbors(coord);

    for (auto &n_chunkPtr : neighbors)
    {
        if (!n_chunkPtr || n_chunkPtr->getState() < ChunkState::TERRAIN_READY)
        {
            return false;
        }
    }
    return true;
}

const std::shared_ptr<Chunk> ChunkManager::getChunk(const ChunkCoord &coord) const
{
    // Shared lock for read only
    std::shared_lock<std::shared_mutex> lock(loadedChunksMutex_);
    auto it = loadedChunks_.find(coord);

    if (it != loadedChunks_.end())
    {
        return it->second;
    }
    else
    {
        return nullptr;
    }
}

Block ChunkManager::getBlockLocal(ChunkCoord chunkCoords, glm::vec3 blockPos)
{
    auto chunkPtr = getChunk(chunkCoords);
    if (!chunkPtr)
    {
        static Block airBlock;
        return airBlock;
    }
    return chunkPtr->getBlockLocal(blockPos);
}

Block ChunkManager::getBlockGlobal(glm::vec3 worldPos) const
{
    // Round the ChunkManager position down to the nearest integers to get block-aligned coordinates
    glm::ivec3 worldCoords = glm::floor(worldPos);
    // Convert ChunkManager coordinates to chunk coordinates
    ChunkCoord chunkCoord = worldToChunkCoords(worldCoords);
    // Convert ChunkManager coordinates to local chunk coordinates
    // Modulo math ensures correct wrapping even with negative positions
    int localX = (worldCoords.x % Constants::CHUNK_SIZE_X + Constants::CHUNK_SIZE_X) % Constants::CHUNK_SIZE_X;
    int localY = worldCoords.y;
    int localZ = (worldCoords.z % Constants::CHUNK_SIZE_Z + Constants::CHUNK_SIZE_Z) % Constants::CHUNK_SIZE_Z;

    glm::ivec3 localBlockPos = glm::ivec3(localX, localY, localZ);

    auto chunkPtr = getChunk(chunkCoord);
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

bool ChunkManager::isBlockSolid(glm::ivec3 blockWorldPos) const
{
    Block block = getBlockGlobal(glm::vec3(blockWorldPos));
    return block.type != BlockType::Air;
}

glm::ivec3 ChunkManager::chunkToWorldCoords(ChunkCoord chunkCoords, glm::ivec3 localPos) const
{
    int x = chunkCoords.x * Constants::CHUNK_SIZE_X + localPos.x;
    int y = localPos.y;
    int z = chunkCoords.z * Constants::CHUNK_SIZE_Z + localPos.z;
    return glm::ivec3(x, y, z);
}

ChunkCoord ChunkManager::worldToChunkCoords(glm::ivec3 worldCoords) const
{
    int chunkX = glm::floor(worldCoords.x / (float)Constants::CHUNK_SIZE_X);
    int chunkZ = glm::floor(worldCoords.z / (float)Constants::CHUNK_SIZE_Z);
    return {chunkX, chunkZ};
}

int ChunkManager::getChunkDistanceSquaredFromPlayer(const ChunkCoord &chunk, const ChunkCoord &playerPos) const
{
    const int dx = chunk.x - playerPos.x;
    const int dz = chunk.z - playerPos.z;
    return dx * dx + dz * dz;
}
