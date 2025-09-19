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
    void renderAllChunks();
    void renderChunk(std::shared_ptr<Chunk> chunk, const ChunkCoord &pos);
    void update();

    // Use visitor pattern
    template <typename Visitor>
    void forEachChunk(Visitor &&v)
    {
        // Lock first
        std::shared_lock<std::shared_mutex> lock(loadedChunksMutex_);
        // Apply visitor function, lambda, ect.. on each coord, chunk pair
        for (const auto [coord, chunk] : loadedChunks_)
        {
            if (!chunk)
                continue;
            v(coord, chunk);
        }
    }
    const std::array<std::shared_ptr<Chunk>, 4> getChunkNeighborsFromCache(const ChunkCoord &center);
    const TextureAtlas &getTextureAtlasRef() const;
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

    std::unordered_map<ChunkCoord, std::array<std::weak_ptr<Chunk>, 4>> neighborCache_;
    std::mutex cacheMutex_;

    void processLighting();
    void processMeshing();

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
