#include "Chunk/ChunkPipeline.h"
#include "Chunk/ChunkStateMachine.h"
#include "Chunk/Chunk.h"

#include <mutex>
#include <vector>
#include <thread>

ChunkPipeline::ChunkPipeline()
    : threadPool_(std::thread::hardware_concurrency() > 1 ? std::thread::hardware_concurrency() - 1 : 1)
{
}

void ChunkPipeline::processTerrainQueue()
{
    std::vector<std::shared_ptr<Chunk>> terrainReady;
    terrainReady.reserve(terrainQueue_.size());

    {
        std::unique_lock<std::mutex> lock(terrainMutex_);

        while (!terrainQueue_.empty())
        {
            terrainReady.push_back(terrainQueue_.front());
            terrainQueue_.pop();
        }
    }

    for (const auto &chunk : terrainReady)
    {
        if (chunk->getState() == ChunkState::TERRAIN_GENERATING)
        {
            threadPool_.enqueue([this, chunk]() { //
                chunk->generateTerrain();
                {
                    std::unique_lock<std::mutex> mutex(meshMutex_);
                    chunk->setState(ChunkState::TERRAIN_READY);
                    meshQueue_.push(chunk);
                }
                // markNeighborChunksForMeshRegeneration(chunk->getCoord());
            });
        }
    }
}

void ChunkPipeline::processMeshQueue()
{
    std::vector<std::shared_ptr<Chunk>> meshReady;
    {
        std::unique_lock<std::mutex> lock(meshMutex_);
        meshReady.reserve(meshQueue_.size());

        while (!meshQueue_.empty())
        {
            auto chunk = meshQueue_.front();
            meshQueue_.pop();
            meshReady.push_back(chunk);
        }

        for (const auto &chunk : meshReady)
        {
            if (chunk->getState() == ChunkState::TERRAIN_READY)
            {
                chunk->setState(ChunkState::MESH_GENERATING);
                threadPool_.enqueue([this, chunk]() { //
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

void ChunkPipeline::processUploadQueue()
{
    std::vector<std::shared_ptr<Chunk>> uploadReady;
    uploadReady.reserve(uploadQueue_.size());

    {
        std::unique_lock<std::mutex> lock(uploadMutex_);

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
        if (chunk->getState() == ChunkState::MESH_READY)
        {
            // Keep on main thread since it uses OpenGL related things
            chunk->uploadMeshToGPU();
            chunk->setState(ChunkState::LOADED);
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
    chunk->setState(ChunkState::TERRAIN_GENERATING);
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
