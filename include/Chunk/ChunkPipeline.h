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
    void queueInitialMesh(std::shared_ptr<Chunk> chunk);
    void queueRemesh(std::shared_ptr<Chunk> chunk);
    void processMeshes();
    // Uploading -- Must be called from Main thread
    void processGPUUploads();

private:
    void generateMesh(std::shared_ptr<Chunk> chunk);
    void uploadMeshToGPU(std::shared_ptr<Chunk> chunk);

    ThreadPool threadPool_;
    ChunkManager &chunkManager_;

    // Meshing
    std::queue<std::shared_ptr<Chunk>> initialMeshQueue_;
    std::queue<std::shared_ptr<Chunk>> remeshQueue_;
    std::mutex meshMutex_;

    // Uploading
    std::queue<std::shared_ptr<Chunk>> gpuUploadQueue_;
    std::mutex uploadMutex_;
};