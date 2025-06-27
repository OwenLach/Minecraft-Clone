#pragma once

#include <queue>
#include <mutex>

#include "ThreadPool.h"
#include "Chunk/Chunk.h"

class ChunkPipeline
{
private:
    ThreadPool threadPool_;

    std::queue<std::shared_ptr<Chunk>> terrainQueue_;
    std::queue<std::shared_ptr<Chunk>> meshQueue_;
    std::queue<std::shared_ptr<Chunk>> uploadQueue_;

    std::mutex terrainMutex_;
    std::mutex meshMutex_;
    std::mutex uploadMutex_;

public:
    ChunkPipeline();

    void processTerrainQueue();
    void processMeshQueue();
    void processUploadQueue();
    void processAll();

    void enqueueForTerrain(std::shared_ptr<Chunk> chunk);
    void enqueueForMesh(std::shared_ptr<Chunk> chunk);
    void enqueueForUpload(std::shared_ptr<Chunk> chunk);
};