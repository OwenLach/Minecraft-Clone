#include "Chunk/ChunkManager.h"
#include "Chunk/Chunk.h"
#include "Chunk/ChunkStateMachine.h"
#include "Chunk/ChunkPipeline.h"
#include "Shader.h"
#include "ThreadPool.h"

#include "FastNoiseLite.h"

#include <iostream>
#include <thread>
#include <algorithm>

ChunkManager::ChunkManager(Camera &camera)
    : camera_(camera),
      chunkShader_("../shaders/chunk.vert", "../shaders/chunk.frag"),
      textureAtlas_()
{
}

void ChunkManager::init(ChunkPipeline *pipeline)
{
    pipeline_ = pipeline;
}

void ChunkManager::addChunk(const ChunkCoord &coord)
{
    auto chunk = std::make_shared<Chunk>(chunkShader_, textureAtlas_, coord);
    chunk->setState(ChunkState::TERRAIN_GENERATING);
    pipeline_->generateTerrain(chunk);

    std::unique_lock<std::shared_mutex> lock(loadedChunksMutex_);
    loadedChunks_[coord] = chunk;
}

void ChunkManager::removeChunk(const ChunkCoord &coord)
{
    std::unique_lock<std::shared_mutex> lock(loadedChunksMutex_);
    loadedChunks_.erase(coord);
}

void ChunkManager::update()
{
    std::shared_lock<std::shared_mutex> lock(loadedChunksMutex_);
    for (const auto &[pos, chunk] : loadedChunks_)
    {
        if (!chunk)
            continue;

        ChunkState state = chunk->getState();

        // IMPORTANT: Set state immediately to prevent re-queueing
        // Scan for chunks that are ready for light propogation
        if (state == ChunkState::TERRAIN_READY && allNeighborsTerrainReady(pos))
        {
            chunk->setState(ChunkState::LIGHT_PROPOGATING);
            pipeline_->propogateLight(chunk);
        }
        // Scan for chunks ready for initail mesh generation
        else if (state == ChunkState::LIGHT_READY && allNeighborsLightReady(pos))
        {
            chunk->setState(ChunkState::MESH_GENERATING);
            pipeline_->queueInitialMesh(chunk);
        }
        // Scan for chunks that need lighting updated
        else if (state == ChunkState::NEEDS_LIGHT_UPDATE)
        {

            chunk->setState(ChunkState::LIGHT_PROPOGATING);
            pipeline_->propogateLight(chunk);
        }
        // Scan for chunks that need a remesh
        else if (state == ChunkState::NEEDS_MESH_REGEN && allNeighborsTerrainReady(pos))
        {
            // chunk->setState(ChunkState::LIGHT_PROPOGATING);
            chunk->setState(ChunkState::MESH_GENERATING);
            pipeline_->queueRemesh(chunk);
        }
    }

    pipeline_->processMeshes();
    pipeline_->processGPUUploads();
}

void ChunkManager::renderAllChunks()
{
    textureAtlas_.bindUnit(0);
    chunkShader_.use();
    chunkShader_.setMat4("projection", camera_.getProjectionMatrix());
    chunkShader_.setMat4("view", camera_.getViewMatrix());

    std::shared_lock<std::shared_mutex> lock(loadedChunksMutex_);
    for (auto &[pos, chunk] : loadedChunks_)
    {
        if (camera_.isAABBInFrustum(chunk->getBoundingBox()))
            renderChunk(chunk, pos);
    }
}

void ChunkManager::renderChunk(std::shared_ptr<Chunk> chunk, const ChunkCoord &pos)
{
    ChunkMesh &mesh = chunk->getMesh();
    if (mesh.hasValidMesh_)
        mesh.render(pos);
}

const std::array<std::shared_ptr<Chunk>, 4> ChunkManager::getChunkNeighborsFromCache(const ChunkCoord &center)
{
    std::array<std::weak_ptr<Chunk>, 4> weakPtrsArr;
    {
        std::lock_guard<std::mutex> lock(cacheMutex_);

        auto it = neighborCache_.find(center);
        // If its in the cache copy the weak pointers and handle outside of lock
        if (it != neighborCache_.end())
        {
            for (size_t i = 0; i < 4; i++)
                weakPtrsArr[i] = it->second[i];
        }
        // Otherwise get copy shared_ptrs from ChunkManager as weakptrs, cache them, and return them
        else
        {
            auto neighbors = getChunkNeighbors(center);
            for (size_t i = 0; i < 4; i++)
                weakPtrsArr[i] = neighbors[i];
            neighborCache_[center] = weakPtrsArr;
            return neighbors;
        }
    }

    // Convert to shared_ptrs and return by value
    std::array<std::shared_ptr<Chunk>, 4> returnArray;
    for (size_t i = 0; i < 4; i++)
        returnArray[i] = weakPtrsArr[i].lock();

    return returnArray;
}

const TextureAtlas &ChunkManager::getTextureAtlasRef() const

{
    return textureAtlas_;
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

void ChunkManager::markNeighborsForMeshRegeneration(const ChunkCoord &coord)
{
    std::shared_lock<std::shared_mutex> lock(loadedChunksMutex_);

    auto originalChunk = getChunk(coord);
    if (!originalChunk || originalChunk->getState() < ChunkState::TERRAIN_READY)
        return; // Don't mark neighbors if this chunk isn't ready

    for (const auto &n_chunkPtr : getChunkNeighbors(coord))
    {
        if (!n_chunkPtr)
            continue;

        // The neighboring chunk exists, all its neighbors at least have their terrain ready, and the chunk can remesh
        if (n_chunkPtr && allNeighborsTerrainReady(n_chunkPtr->getCoord()) && n_chunkPtr->canRemesh())
            n_chunkPtr->setState(ChunkState::NEEDS_MESH_REGEN);
    }
}

bool ChunkManager::allNeighborsTerrainReady(const ChunkCoord &coord)
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

bool ChunkManager::allNeighborsLightReady(const ChunkCoord &coord)
{
    auto neighbors = getChunkNeighbors(coord);

    for (auto &n_chunkPtr : neighbors)
    {
        if (!n_chunkPtr || n_chunkPtr->getState() < ChunkState::LIGHT_READY)
        {
            return false;
        }
    }
    return true;
}
