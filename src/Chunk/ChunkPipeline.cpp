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
    : threadPool_(std::thread::hardware_concurrency() > 1 ? std::thread::hardware_concurrency() - 2 : 1),
      chunkManager_(chunkManager)
{
}

void ChunkPipeline::generateTerrain(std::shared_ptr<Chunk> chunk)
{
    chunk->setState(ChunkState::TERRAIN_GENERATING);
    threadPool_.enqueue([this, chunk]() { //
        chunk->generateTerrain();
        chunk->setState(ChunkState::TERRAIN_READY);
        // chunk->checkAndNotifyNeighbors();
        generateMesh(chunk);
    });
}

void ChunkPipeline::generateMesh(std::shared_ptr<Chunk> chunk)
{
    chunk->setState(ChunkState::MESH_GENERATING);
    threadPool_.enqueue([this, chunk]() { //
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
    chunk->isDirty_.store(false);
}
