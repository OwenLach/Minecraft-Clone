#pragma once

#include <memory>

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
    void generateTerrain(std::shared_ptr<Chunk> chunk);
    void seedInitialLight(std::shared_ptr<Chunk> chunk);
    void propogateLight(std::shared_ptr<Chunk> chunk);
    void generateMesh(std::shared_ptr<Chunk> chunk);
    void uploadMeshToGPU(std::shared_ptr<Chunk> chunk);

private:
    ChunkManager *chunkManager_;
    LightSystem *lightSystem_;
};