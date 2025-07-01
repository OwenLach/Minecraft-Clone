#include "Chunk/ChunkPipeline.h"
#include "Chunk/ChunkStateMachine.h"
#include "Chunk/ChunkManager.h"
#include "Chunk/Chunk.h"

#include <mutex>
#include <vector>
#include <thread>
#include <iostream>

ChunkPipeline::ChunkPipeline(ChunkManager &chunkManager)
    : threadPool_(std::thread::hardware_concurrency() > 1 ? std::thread::hardware_concurrency() - 2 : 1),
      chunkManager_(chunkManager)
{
}

void ChunkPipeline::processTerrainQueue()
{
    // Push all chunks from queue into vector
    std::vector<std::shared_ptr<Chunk>> terrainBatch;
    terrainBatch.reserve(terrainQueue_.size());

    {
        std::unique_lock<std::mutex> lock(terrainMutex_);
        while (!terrainQueue_.empty())
        {
            terrainBatch.push_back(terrainQueue_.front());
            terrainQueue_.pop();
        }
    }

    // For each chunk in the batch, generate terrain on a different thread
    for (const auto &chunk : terrainBatch)
    {
        if (chunk->getState() == ChunkState::EMPTY)
        {
            chunk->setState(ChunkState::TERRAIN_GENERATING);
            threadPool_.enqueue([this, chunk]() { //
                chunk->generateTerrain();
                chunk->setState(ChunkState::TERRAIN_READY);
                // std::cout << "Generated terrain for chunk" << std::endl;
                enqueueForMesh(chunk);
                chunkManager_.markNeighborChunksForMeshRegeneration(chunk->getCoord());
            });
        }
    }
}

void ChunkPipeline::processMeshQueue()
{

    // std::cout << "Processing " << meshQueue_.size() << " chunks in mesh queue" << std::endl;

    std::vector<std::shared_ptr<Chunk>> meshBatch;
    std::vector<std::shared_ptr<Chunk>> chunksToRequeue;

    {
        std::unique_lock<std::mutex> lock(meshMutex_);
        if (meshQueue_.empty())
            return;

        meshBatch.reserve(meshQueue_.size());
        chunksToRequeue.reserve(meshQueue_.size());

        while (!meshQueue_.empty())
        {
            auto chunk = meshQueue_.front();
            meshQueue_.pop();
            meshBatch.push_back(chunk);
        }
    }

    for (const auto &chunk : meshBatch)
    {
        ChunkState state = chunk->getState();

        // Handle different chunk states
        if (state == ChunkState::MESH_READY)
        {
            // This shouldn't happen - log it as a bug
            std::cout << "WARNING: MESH_READY chunk in mesh queue! Coord: ("
                      << chunk->getCoord().x << ", " << chunk->getCoord().z << ")" << std::endl;
            enqueueForUpload(chunk);
        }
        else if ((state == ChunkState::TERRAIN_READY || state == ChunkState::NEEDS_MESH_REGEN) &&
                 !chunk->isProcessing() &&
                 chunkManager_.allNeighborsLoaded(chunk->getCoord()))
        {

            // Ready to generate mesh
            chunk->setState(ChunkState::MESH_GENERATING);
            auto neighbors = chunkManager_.getChunkNeighbors(chunk->getCoord());

            threadPool_.enqueue([this, chunk, neighbors]() { //
                chunk->generateMesh(neighbors);
                chunk->setState(ChunkState::MESH_READY);
                enqueueForUpload(chunk);
            });
        }
        else
        {
            // Not ready yet, requeue for later
            chunksToRequeue.push_back(chunk);
        }
    }

    if (!chunksToRequeue.empty())
    {
        std::unique_lock<std::mutex> lock(meshMutex_);
        for (const auto &chunk : chunksToRequeue)
        {
            meshQueue_.push(chunk);
        }
    }
}

void ChunkPipeline::processUploadQueue()
{
    std::vector<std::shared_ptr<Chunk>> uploadBatch;
    uploadBatch.reserve(uploadQueue_.size());

    {
        std::unique_lock<std::mutex> lock(uploadMutex_);
        int count = 0;
        while (!uploadQueue_.empty() && count < Constants::MAX_CHUNKS_PER_FRAME)
        {
            uploadBatch.push_back(uploadQueue_.front());
            uploadQueue_.pop();
            count++;
        }
    }

    for (const auto &chunk : uploadBatch)
    {
        if (chunk->getState() == ChunkState::MESH_READY)
        {
            // Keep on main thread since it uses OpenGL related things
            chunk->uploadMeshToGPU();
            chunk->setState(ChunkState::LOADED);
            chunk->isDirty_.store(false);
        }
    }
}

void ChunkPipeline::processAll()
{
    processTerrainQueue();
    processMeshQueue();
    processUploadQueue();
}

void ChunkPipeline::enqueueForTerrain(std::shared_ptr<Chunk> chunk)
{
    std::unique_lock<std::mutex> lock(terrainMutex_);
    terrainQueue_.push(chunk);
}

void ChunkPipeline::enqueueForMesh(std::shared_ptr<Chunk> chunk)
{
    std::unique_lock<std::mutex> lock(meshMutex_);
    meshQueue_.push(chunk);
}

void ChunkPipeline::enqueueForUpload(std::shared_ptr<Chunk> chunk)
{
    std::unique_lock<std::mutex> lock(uploadMutex_);
    uploadQueue_.push(chunk);
}

void ChunkPipeline::enqueueForRegen(std::shared_ptr<Chunk> chunk)
{
    // std::cout << "Enqueueing chunk (" << chunk->getCoord().x << ", " << chunk->getCoord().z
    // << ") for regen, state: " << (int)chunk->getState() << std::endl;
    std::unique_lock<std::mutex> lock(meshMutex_);
    enqueueForMesh(chunk);
}
