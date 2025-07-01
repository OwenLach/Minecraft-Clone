#pragma once

#include "Chunk/Chunk.h"
#include "Chunk/ChunkCoord.h"
#include "Chunk/ChunkPipeline.h"
#include "Block.h"
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
    TextureAtlas *textureAtlas_;
    Camera &camera_;

public:
    ChunkManager(Shader &shader, TextureAtlas *atlas, Camera &camera);

    void render();
    void update();

    void loadChunk(ChunkCoord coord);
    void unloadChunk(ChunkCoord coord);

    glm::ivec3 chunkToWorldCoords(ChunkCoord chunkCoords, glm::ivec3 localPos) const;
    ChunkCoord worldToChunkCoords(glm::ivec3 worldCoords) const;
    Block getBlockLocal(ChunkCoord chunkCoords, glm::vec3 blockPos);
    Block getBlockGlobal(glm::vec3 worldPos) const;

    bool isBlockSolid(glm::ivec3 blockWorldPos) const;
    void markNeighborChunksForMeshRegeneration(const ChunkCoord &coord);
    std::array<std::shared_ptr<Chunk>, 4> getChunkNeighbors(const ChunkCoord &coord);
    bool allNeighborsLoaded(const ChunkCoord &coord);
    const std::shared_ptr<Chunk> getChunk(const ChunkCoord &coord) const;

    std::unordered_map<ChunkCoord, std::shared_ptr<Chunk>> loadedChunks_;
    mutable std::shared_mutex loadedChunksMutex_;

private:
    // storage for loaded chunks, chunks might be in one of several states

    void loadInitialChunks();
    void loadVisibleChunks(const ChunkCoord &playerPos, const int renderDistance);
    void unloadDistantChunks(const ChunkCoord &playerPos);

    int getChunkDistanceSquaredFromPlayer(const ChunkCoord &chunk, const ChunkCoord &playerPos) const;

    static inline bool
    isInRenderDistance(int chunkX, int chunkZ, int playerX, int playerZ)
    {
        const int renderDistance = Constants::RENDER_DISTANCE;
        const int dx = chunkX - playerX;
        const int dz = chunkZ - playerZ;
        return dx * dx + dz * dz <= renderDistance * renderDistance;
    }
};