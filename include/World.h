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
    size_t ChunksRendered = 0;
    /**
     * @brief Constructs the world and initializes rendering systems.
     *
     * @param shader Reference to the main shader used for rendering.
     * @param atlas Pointer to the texture atlas used for block textures.
     */
    World(Shader &shader, TextureAtlas *atlas, Camera &camera);

    /**
     * @brief Renders all currently loaded and visible chunks.
     *
     * Binds appropriate shaders and textures, uploads vertex data to the GPU,
     * and issues draw calls for chunk meshes.
     */
    void render();

    /**
     * @brief Updates world state, such as loading/unloading chunks and rebuilding meshes.
     *
     * Should be called every frame or tick to handle:
     * - Dirty chunk mesh regeneration
     * - Chunk loading and unloading based on player position
     * - Block and lighting updates
     */
    void update();

    void loadChunk(ChunkCoord coord);
    void unloadChunk(ChunkCoord coord);

    /**
     * @brief Converts local block coordinates within a chunk to world coordinates.
     *
     * @param chunkCoords The chunk's grid position in the world (X and Z).
     * @param localPos The local position inside the chunk (X, Y, Z).
     * @return The corresponding world-space block coordinates.
     */
    glm::ivec3 chunkToWorldCoords(ChunkCoord chunkCoords, glm::ivec3 localPos) const;

    /**
     * @brief Converts a world-space block coordinate to its corresponding chunk coordinate.
     */
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