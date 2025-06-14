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

    // Converts local block coordinates within a chunk to world coordinates.
    glm::ivec3 chunkToWorldCoords(ChunkCoord chunkCoords, glm::ivec3 localPos) const;

    // Converts a world-space block coordinate to its corresponding chunk coordinate.
    ChunkCoord worldToChunkCoords(glm::ivec3 worldCoords) const;

    // Retrieves a block from a specific chunk using chunk coordinates and local block position.
    Block getBlockAt(ChunkCoord chunkCoords, glm::vec3 blockPos);

    // Retrieves a block from the world using global world coordinates.
    Block getBlockAt(glm::vec3 worldPos) const;

    bool isBlockSolid(glm::ivec3 blockWorldPos) const;

private:
    Shader &shader;
    TextureAtlas *textureAtlas;
    Camera &camera;

    // storage for loaded chunks, chunks might be in one of several states
    std::unordered_map<ChunkCoord, std::shared_ptr<Chunk>> loadedChunks;

    ThreadPool chunkThreadPool;

    std::queue<std::shared_ptr<Chunk>> meshGenQueue;
    std::queue<std::shared_ptr<Chunk>> uploadQueue;

    std::mutex meshGenMutex;
    std::mutex uploadMutex;

    void loadInitialChunks();
    void loadVisibleChunks(const ChunkCoord &playerPos, const int renderDistance);
    void processTerrainToMesh();
    void processMeshToGPU();
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