#pragma once

#include <unordered_map>
#include <queue>
#include <mutex>

#include <glm/glm.hpp>

#include "Chunk.h"
#include "Shader.h"
#include "TextureAtlas.h"
#include "ChunkCoord.h"
#include "Block.h"
#include "Camera.h"
#include "ThreadPool.h"

class World
{
public:
    World(Shader &shader, TextureAtlas *atlas, Camera &camera);

    void render();
    void update();

    void loadChunk(ChunkCoord coord);
    void unloadChunk(ChunkCoord coord);

    glm::ivec3 chunkToWorldCoords(ChunkCoord chunkCoords, glm::ivec3 localPos) const;
    ChunkCoord worldToChunkCoords(glm::ivec3 worldCoords) const;
    Block getBlockLocal(ChunkCoord chunkCoords, glm::vec3 blockPos);
    Block getBlockGlobal(glm::vec3 worldPos) const;

    bool isBlockSolid(glm::ivec3 blockWorldPos) const;

private:
    Shader &shader_;
    TextureAtlas *textureAtlas_;
    Camera &camera_;

    // storage for loaded chunks, chunks might be in one of several states
    std::unordered_map<ChunkCoord, std::shared_ptr<Chunk>> loadedChunks_;

    ThreadPool chunkThreadPool_;

    std::queue<std::shared_ptr<Chunk>> meshGenQueue_;
    std::queue<std::shared_ptr<Chunk>> uploadQueue_;
    std::queue<std::shared_ptr<Chunk>> meshRegenQueue_;

    std::mutex meshGenMutex_;
    std::mutex uploadMutex_;

    void loadInitialChunks();
    void loadVisibleChunks(const ChunkCoord &playerPos, const int renderDistance);
    void processTerrainToMesh();
    void processMeshToGPU();
    void unloadDistantChunks(const ChunkCoord &playerPos);

    void markNeighborChunksForMeshRegeneration(const ChunkCoord &coord);
    bool allNeighborsLoaded(const ChunkCoord &coord);

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