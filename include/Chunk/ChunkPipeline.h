#pragma once

#include "ThreadPool.h"
#include <memory>
#include <queue>
#include <mutex>

class Chunk;
class ChunkManager;
class LightSystem;

// Responsible for handling the whole chunk pipeline process
// Generate terrain -> Propogate light -> Mesh -> Upload to GPU -> Handle remeshing
class ChunkPipeline
{
public:
    ChunkPipeline();
    void init(ChunkManager *chunkManager, LightSystem *lightSystem);
    // Terrain Gen
    void generateTerrain(std::shared_ptr<Chunk> chunk);
    // Lighting
    void propogateLight(std::shared_ptr<Chunk> chunk);
    // Meshing
    void queueInitialMesh(std::shared_ptr<Chunk> chunk);
    void queueRemesh(std::shared_ptr<Chunk> chunk);
    void processMeshes();
    // Uploading -- Must be called from Main thread
    void processGPUUploads();

private:
    ThreadPool threadPool_;
    ChunkManager *chunkManager_;
    LightSystem *lightSystem_;

    // Lighting
    std::queue<std::shared_ptr<Chunk>> lightQueue_;
    std::mutex lightMutex_;
    // Meshing
    std::queue<std::shared_ptr<Chunk>> initialMeshQueue_;
    std::queue<std::shared_ptr<Chunk>> remeshQueue_;
    std::mutex meshMutex_;
    // Uploading
    std::queue<std::shared_ptr<Chunk>> gpuUploadQueue_;
    std::mutex uploadMutex_;

    void generateMesh(std::shared_ptr<Chunk> chunk);
    void uploadMeshToGPU(std::shared_ptr<Chunk> chunk);
};