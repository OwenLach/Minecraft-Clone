#pragma once

#include "Chunk/Chunk.h"
#include "Chunk/ChunkCoord.h"
#include "Block/Block.h"
#include "ThreadPool.h"
#include "Shader.h"
#include "TextureAtlas.h"
#include "Camera.h"

#include <unordered_map>
#include <queue>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>
#include <vector>
#include <memory>

class ChunkPipeline;

class ChunkManager
{
public:
    ChunkManager(Camera &camera);

    void init(ChunkPipeline *pipeline);
    void addChunk(const ChunkCoord &coord);
    void removeChunk(const ChunkCoord &coord);
    void render();
    void update();

    std::unordered_map<ChunkCoord, std::shared_ptr<Chunk>> getLoadedChunksCopy() const;
    const std::shared_ptr<Chunk> getChunk(const ChunkCoord &coord) const;
    std::array<std::shared_ptr<Chunk>, 4> getChunkNeighbors(const ChunkCoord &coord);
    void markNeighborsForMeshRegeneration(const ChunkCoord &coord);

private:
    Camera &camera_;
    Shader chunkShader_;
    TextureAtlas textureAtlas_;
    ChunkPipeline *pipeline_;

    std::unordered_map<ChunkCoord, std::shared_ptr<Chunk>> loadedChunks_;
    mutable std::shared_mutex loadedChunksMutex_;

    bool allNeighborsTerrainReady(const ChunkCoord &coord);
    bool allNeighborsLightReady(const ChunkCoord &coord);

    static inline bool isInRenderDistance(int chunkX, int chunkZ, int playerX, int playerZ)
    {
        const int renderDistance = Constants::RENDER_DISTANCE;
        const int dx = chunkX - playerX;
        const int dz = chunkZ - playerZ;
        return dx * dx + dz * dz <= renderDistance * renderDistance;
    }
};