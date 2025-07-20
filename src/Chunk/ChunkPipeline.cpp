#include "Chunk/ChunkPipeline.h"
#include "Chunk/ChunkStateMachine.h"
#include "Chunk/ChunkManager.h"
#include "Chunk/Chunk.h"

#include "Performance/ScopedTimer.h"

#include <mutex>
#include <vector>
#include <thread>
#include <iostream>

ChunkPipeline::ChunkPipeline(ChunkManager &chunkManager)
    : threadPool_(std::thread::hardware_concurrency() > 1 ? std::thread::hardware_concurrency() / 2 : 1),
      chunkManager_(chunkManager)
{
}

void ChunkPipeline::generateTerrain(std::shared_ptr<Chunk> chunk)
{
    chunk->setState(ChunkState::TERRAIN_GENERATING);
    threadPool_.enqueue([this, chunk]() { //
        chunk->generateTerrain();
        chunk->setState(ChunkState::TERRAIN_READY);
        chunkManager_.markNeighborsForMeshRegeneration(chunk->getCoord());
    });
}

void ChunkPipeline::queueInitialMesh(std::shared_ptr<Chunk> chunk)
{
    std::lock_guard<std::mutex> lock(meshMutex_);
    initialMeshQueue_.push(chunk);
}

void ChunkPipeline::queueRemesh(std::shared_ptr<Chunk> chunk)
{
    std::lock_guard<std::mutex> lock(meshMutex_);
    remeshQueue_.push(chunk);
}

void ChunkPipeline::processMeshes()
{
    std::lock_guard<std::mutex> lock(meshMutex_);
    
    const int max_meshes = Constants::MAX_MESHES_PER_FRAME;
    int meshed = 0;

    // 1. Prioritize Initial Meshes
    while (meshed < max_meshes && !initialMeshQueue_.empty()) {
        std::shared_ptr<Chunk> chunk = initialMeshQueue_.front();
        initialMeshQueue_.pop();
        generateMesh(chunk);
        meshed++;
    }
   
    // 2. Process Re-meshes if we have remaining capacity
    while (meshed < max_meshes && !remeshQueue_.empty()) {
        std::shared_ptr<Chunk> chunk = remeshQueue_.front();
        remeshQueue_.pop();
        generateMesh(chunk);
        meshed++;
    }
}

void ChunkPipeline::generateMesh(std::shared_ptr<Chunk> chunk)
{
    threadPool_.enqueue([this, chunk]() { //
        if (!chunk) return;

        chunk->generateMesh(chunkManager_.getChunkNeighbors(chunk->getCoord()));
        chunk->setState(ChunkState::MESH_READY);
        {
            std::lock_guard<std::mutex> lock(uploadMutex_);
            gpuUploadQueue_.push(chunk);
        }
    });
}


void ChunkPipeline::processGPUUploads()
{
    std::queue<std::shared_ptr<Chunk>> chunksToUpload;

    // Move all pending uploads to local queue
    {
        std::lock_guard<std::mutex> lock(uploadMutex_);
        chunksToUpload = std::move(gpuUploadQueue_);
        gpuUploadQueue_ = std::queue<std::shared_ptr<Chunk>>(); // Clear the queue
    }

    // Process uploads on main thread
    while (!chunksToUpload.empty())
    {
        auto chunk = chunksToUpload.front();
        chunksToUpload.pop();

        if (chunk->getState() == ChunkState::MESH_READY)
        {
            uploadMeshToGPU(chunk);
        }
    }
}

void ChunkPipeline::uploadMeshToGPU(std::shared_ptr<Chunk> chunk)
{
    chunk->setState(ChunkState::LOADED);
    chunk->uploadMeshToGPU();
}
