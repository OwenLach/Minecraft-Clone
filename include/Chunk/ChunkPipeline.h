#pragma once

#include "ThreadPool.h"
#include <memory>
#include <queue>
#include <mutex>

class Chunk;
class ChunkManager;

class ChunkPipeline
{
public:
    ChunkPipeline(ChunkManager &chunkManager);

    void generateTerrain(std::shared_ptr<Chunk> chunk);
    void generateMesh(std::shared_ptr<Chunk> chunk);

    // Must be called from Main thread
    void processGPUUploads();

private:
    void uploadMeshToGPU(std::shared_ptr<Chunk> chunk);

    ThreadPool threadPool_;
    ChunkManager &chunkManager_;

    std::queue<std::shared_ptr<Chunk>> gpuUploadQueue_;
    std::mutex uploadMutex_;
};