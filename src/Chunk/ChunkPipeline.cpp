#include "Chunk/ChunkPipeline.h"
#include "Chunk/ChunkManager.h"
#include "Chunk/ChunkMeshBuilder.h"
#include "Chunk/Chunk.h"
#include "LightSystem.h"

#include "Performance/ScopedTimer.h"

#include <vector>
#include <iostream>

ChunkPipeline::ChunkPipeline() {}

void ChunkPipeline::init(ChunkManager *chunkManager, LightSystem *lightSystem)
{
    chunkManager_ = chunkManager;
    lightSystem_ = lightSystem;
}

void ChunkPipeline::generateTerrain(std::shared_ptr<Chunk> chunk)
{
    if (!chunk)
        return;

    chunk->generateTerrain();
    chunkManager_->notifyStateChange({chunk, ChunkState::TERRAIN_GENERATED});
}

void ChunkPipeline::seedInitialLight(std::shared_ptr<Chunk> chunk)
{
    if (!chunk)
        return;

    lightSystem_->seedInitialSkylight(chunk);
    chunkManager_->notifyStateChange({chunk, ChunkState::INITIAL_LIGHT_READY});
}

void ChunkPipeline::propogateLight(std::shared_ptr<Chunk> chunk)
{
    if (!chunk)
        return;

    lightSystem_->updateBorderLighting(chunk);
    chunkManager_->notifyStateChange({chunk, ChunkState::FINAL_LIGHT_READY});
}

void ChunkPipeline::generateMesh(std::shared_ptr<Chunk> chunk)
{
    if (!chunk)
        return;

    MeshData emptyMeshData;
    auto neighbors = chunkManager_->getChunkNeighbors(chunk->getCoord());
    const TextureAtlas &atlas = chunkManager_->getTextureAtlasRef();

    ChunkMeshBuilder builder(emptyMeshData, atlas, chunk, neighbors);
    MeshData &newMeshData = builder.buildMesh();

    chunk->setMeshData(newMeshData);
    chunkManager_->notifyStateChange({chunk, ChunkState::MESH_READY});
}

void ChunkPipeline::uploadMeshToGPU(std::shared_ptr<Chunk> chunk)
{
    if (!chunk)
        return;

    chunk->getMesh().uploadMesh();
    chunkManager_->notifyStateChange({chunk, ChunkState::LOADED});
}