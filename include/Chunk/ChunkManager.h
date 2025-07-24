#pragma once

#include "Chunk/Chunk.h"
#include "Chunk/ChunkCoord.h"
#include "Chunk/ChunkPipeline.h"
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

class ChunkManager
{
private:
    std::unique_ptr<ChunkPipeline> pipeline_;

    Shader &shader_;
    TextureAtlas &textureAtlas_;
    Camera &camera_;

public:
    ChunkManager(Shader &shader, Camera &camera, TextureAtlas &textureAtlas);

    void addChunk(const ChunkCoord &coord, std::shared_ptr<Chunk> chunk);
    void removeChunk(const ChunkCoord &coord);

    std::unordered_map<ChunkCoord, std::shared_ptr<Chunk>> getLoadedChunksCopy() const;
    const std::shared_ptr<Chunk> getChunk(const ChunkCoord &coord) const;
    std::array<std::shared_ptr<Chunk>, 4> getChunkNeighbors(const ChunkCoord &coord);

    void render();

    void markNeighborsForMeshRegeneration(const ChunkCoord &coord);
    bool allNeighborsTerrainReady(const ChunkCoord &coord);

private:
    std::unordered_map<ChunkCoord, std::shared_ptr<Chunk>> loadedChunks_;
    mutable std::shared_mutex loadedChunksMutex_;

    static inline bool
    isInRenderDistance(int chunkX, int chunkZ, int playerX, int playerZ)
    {
        const int renderDistance = Constants::RENDER_DISTANCE;
        const int dx = chunkX - playerX;
        const int dz = chunkZ - playerZ;
        return dx * dx + dz * dz <= renderDistance * renderDistance;
    }
};