#include "Chunk/ChunkManager.h"
#include "Chunk/Chunk.h"
#include "Chunk/ChunkStateMachine.h"
#include "ThreadPool.h"
#include "FastNoiseLite.h"

#include <iostream>
#include <thread>
#include <algorithm>

ChunkManager::ChunkManager(Shader &shader, TextureAtlas *atlas, Camera &camera)
    : shader_(shader),
      textureAtlas_(atlas),
      camera_(camera),
      chunkThreadPool_(std::thread::hardware_concurrency() > 1 ? std::thread::hardware_concurrency() - 1 : 1)
{
    loadInitialChunks();
}

void ChunkManager::render()
{
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

    // load new chunks
    loadVisibleChunks(playerPos, renderDistance);
    // 2. Process chunks ready for mesh generation
    processTerrainToMesh();
    // 3. Process chunks ready for GPU upload
    processMeshToGPU();
    // 4. Unload chunks
    unloadDistantChunks(playerPos);
}

void ChunkManager::loadChunk(ChunkCoord coord)
{
    // if its already loaded, return
    if (loadedChunks_.find(coord) != loadedChunks_.end())
        return;

    // Create the chunk object. It starts in EMPTY state_.
    auto chunk = std::make_shared<Chunk>(shader_, textureAtlas_, coord, this);
    chunk->setState(ChunkState::TERRAIN_GENERATING);
    loadedChunks_.emplace(coord, chunk);

    // Start terrain generation task for the worker threads
    chunkThreadPool_.enqueue([this, chunk]() { //
        // starts generating terrain on worker thread
        chunk->generateTerrain();
        {
            // get the lock to change state_
            std::unique_lock<std::mutex> mutex(meshGenMutex_);
            chunk->setState(ChunkState::TERRAIN_READY);
            // push to mesh gen queue
            meshGenQueue_.push(chunk);
        }
        // mark neighbors to regenerate their mesh
        // markNeighborChunksForMeshRegeneration(chunk->getCoord());
    });
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

    // load closest to player first
    for (const auto &chunk : orderedChunks)
    {
        loadChunk(chunk);
    }
}

void ChunkManager::loadVisibleChunks(const ChunkCoord &playerPos, const int renderDistance)
{
    for (int x = -renderDistance; x < renderDistance; x++)
    {
        for (int z = -renderDistance; z < renderDistance; z++)
        {
            ChunkCoord chunkCoord{playerPos.x + x, playerPos.z + z};
            if (loadedChunks_.find(chunkCoord) == loadedChunks_.end() && isInRenderDistance(chunkCoord.x, chunkCoord.z, playerPos.x, playerPos.z))
            {
                loadChunk(chunkCoord);
            }
        }
    }
}

void ChunkManager::processTerrainToMesh()
{
    // iterate through the meshGenQueue_
    std::vector<std::shared_ptr<Chunk>> meshGenReady;
    {
        // get a lock since it access meshGenQueue_
        std::unique_lock<std::mutex> lock(meshGenMutex_);
        meshGenReady.reserve(meshGenQueue_.size());

        while (!meshGenQueue_.empty())
        {
            auto chunk = meshGenQueue_.front();
            meshGenQueue_.pop();
            meshGenReady.push_back(chunk);
        }

        // loop through only the chunks ready
        for (const auto &chunk : meshGenReady)
        {
            if (chunk->getState() == ChunkState::TERRAIN_READY &&
                loadedChunks_.find(chunk->getCoord()) != loadedChunks_.end())
            {
                chunk->setState(ChunkState::MESH_GENERATING);
                // start mesh generation on worker thread
                chunkThreadPool_.enqueue([this, chunk]() { //
                    chunk->generateMesh();
                    {
                        std::unique_lock<std::mutex> lock(uploadMutex_);
                        chunk->setState(ChunkState::MESH_READY);
                        uploadQueue_.push(chunk);

                        chunk->isDirty_.store(false);
                    }
                });
            }
        }
    }
}

void ChunkManager::processMeshToGPU()
{
    std::vector<std::shared_ptr<Chunk>> uploadReady;
    {
        // get a lock since it access meshGenQueue_
        std::unique_lock<std::mutex> lock(uploadMutex_);
        uploadReady.reserve(uploadQueue_.size());
        // push everthing to meshGenReady vector
        int count = 0;
        while (!uploadQueue_.empty() && count < Constants::MAX_CHUNKS_PER_FRAME)
        {
            uploadReady.push_back(uploadQueue_.front());
            uploadQueue_.pop();
            count++;
        }
    }

    for (const auto &chunk : uploadReady)
    {
        // make sure chunk isn't unloaded
        if (chunk->getState() == ChunkState::MESH_READY &&
            loadedChunks_.find(chunk->getCoord()) != loadedChunks_.end())
        {
            // Keep on main thread since it uses OpenGL related things
            chunk->uploadMeshToGPU();
            chunk->setState(ChunkState::LOADED);
        }
    }
}

void ChunkManager::unloadDistantChunks(const ChunkCoord &playerPos)
{
    std::vector<ChunkCoord> chunkPositionsToRemove;
    for (auto &[pos, chunk] : loadedChunks_)
    {
        // if chunk is in the middle of processing anything, don't unload
        if (!isInRenderDistance(pos.x, pos.z, playerPos.x, playerPos.z) &&
            chunk->getState() != ChunkState::TERRAIN_GENERATING &&
            chunk->getState() != ChunkState::MESH_GENERATING &&
            chunk->getState() != ChunkState::MESH_READY)
        {
            chunkPositionsToRemove.push_back(pos);
        }
    }

    for (const ChunkCoord coord : chunkPositionsToRemove)
    {
        unloadChunk(coord);
    }
}

void ChunkManager::markNeighborChunksForMeshRegeneration(const ChunkCoord &coord)
{
    if (!allNeighborsLoaded(coord))
    {
        return;
    }

    const int x = coord.x;
    const int z = coord.z;
    std::vector<ChunkCoord> neighbors = {
        ChunkCoord{x + 1, z},
        ChunkCoord{x - 1, z},
        ChunkCoord{x, z + 1},
        ChunkCoord{x, z - 1},
    };

    std::unique_lock<std::mutex> lock(meshGenMutex_);

    for (const auto &neighborCoord : neighbors)
    {
        // if neighbor exists, is loaded, and isn't dirty
        // set the new state_ to MESH_GENERATING and push back to meshGenQueue
        auto chunk = loadedChunks_.find(neighborCoord);
        if (chunk != loadedChunks_.end() &&
            chunk->second->getState() == ChunkState::LOADED)
        {
            // Check and set isDirty_ atomically within the lock
            bool expected = false;
            if (chunk->second->isDirty_.compare_exchange_strong(expected, true))
            {
                chunk->second->setState(ChunkState::MESH_GENERATING);
                meshGenQueue_.push(chunk->second);
            }
            // If compare_exchange_strong failed, the chunk was already dirty/queued
        }
    }
}

bool ChunkManager::allNeighborsLoaded(const ChunkCoord &coord)
{
    std::vector<ChunkCoord> neighbors = {
        {coord.x + 1, coord.z},
        {coord.x - 1, coord.z},
        {coord.x, coord.z + 1},
        {coord.x, coord.z - 1}};

    for (auto &n : neighbors)
    {
        if (loadedChunks_.find(n) == loadedChunks_.end())
        {
            return false;
        }
    }
    return true;
}
Block ChunkManager::getBlockLocal(ChunkCoord chunkCoords, glm::vec3 blockPos)
{
    auto it = loadedChunks_.find(chunkCoords);
    if (it == loadedChunks_.end())
    {
        static Block airBlock;
        return airBlock;
    }

    Chunk *chunk = it->second.get();
    return chunk->getBlockLocal(blockPos);
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

    auto it = loadedChunks_.find(chunkCoord);
    if (it != loadedChunks_.end())
    {
        return it->second->getBlockLocal(glm::ivec3(localX, localY, localZ));
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
