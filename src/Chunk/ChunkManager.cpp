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

ChunkManager::ChunkManager(Shader &shader, Camera &camera, TextureAtlas &textureAtlas)
    : shader_(shader),
      camera_(camera),
      textureAtlas_(textureAtlas),
      pipeline_(std::make_unique<ChunkPipeline>(*this))
{
    // loadInitialChunks();
}

void ChunkManager::addChunk(const ChunkCoord &coord, std::shared_ptr<Chunk> chunk)
{
    std::unique_lock<std::shared_mutex> lock(loadedChunksMutex_);
    loadedChunks_[coord] = chunk;
}

void ChunkManager::removeChunk(const ChunkCoord &coord)
{
    std::unique_lock<std::shared_mutex> lock(loadedChunksMutex_);
    loadedChunks_.erase(coord);
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

std::array<std::shared_ptr<Chunk>, 4> ChunkManager::getChunkNeighbors(const ChunkCoord &coord)
{
    // Acquire a shared lock for thread-safe read access to loadedChunks_

    const std::array<ChunkCoord, 4> neighborCoords = {{
        {coord.x, coord.z + 1}, // North
        {coord.x, coord.z - 1}, // South
        {coord.x + 1, coord.z}, // East
        {coord.x - 1, coord.z}  // West
    }};

    std::array<std::shared_ptr<Chunk>, 4> neighbors;
    {
        std::shared_lock<std::shared_mutex> lock(loadedChunksMutex_);
        for (int i = 0; i < 4; i++)
        {
            auto it = loadedChunks_.find(neighborCoords[i]);
            neighbors[i] = (it != loadedChunks_.end()) ? it->second : nullptr;
        }
    }

    return neighbors;
}

void ChunkManager::render()
{
    std::shared_lock<std::shared_mutex> lock(loadedChunksMutex_);
    for (auto &[pos, chunk] : loadedChunks_)
    {
        if (camera_.isAABBInFrustum(chunk->getBoundingBox()) && chunk->getState() == ChunkState::LOADED)
            chunk->render();
    }
}