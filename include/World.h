#pragma once

#include "Chunk/ChunkManager.h"
#include "Chunk/ChunkPipeline.h"
#include "Constants.h"

#include <glm/glm.hpp>

struct ChunkCoord;
class Block;
class Camera;
class Shader;
class TextureAtlas;

class World
{
public:
    World(Camera &camera, Shader &shader, TextureAtlas &textureAtlas);

    void update();
    void render();

private:
    void loadNewChunks(ChunkCoord center);
    void unloadDistantChunks();

    Camera &camera_;
    Shader &shader_;
    TextureAtlas &textureAtlas_;

    ChunkManager chunkManager_;
    ChunkPipeline pipeline_;
    ChunkCoord lastPlayerChunk_;

    // Coordinate conversions
    glm::ivec3 chunkToWorldCoords(ChunkCoord chunkCoords, glm::ivec3 localPos) const;
    ChunkCoord worldToChunkCoords(glm::ivec3 worldCoords) const;

    // Block helpers
    Block getBlockLocal(ChunkCoord chunkCoords, glm::vec3 blockPos);
    Block getBlockGlobal(glm::vec3 worldPos) const;
    bool isBlockSolid(glm::ivec3 blockWorldPos) const;

    // void markNeighborChunksForMeshRegeneration(const ChunkCoord &coord);
    // bool allNeighborsLoaded(const ChunkCoord &coord);

    static inline bool
    isInRenderDistance(int chunkX, int chunkZ, int playerX, int playerZ)
    {
        const int renderDistance = Constants::RENDER_DISTANCE;
        const int dx = chunkX - playerX;
        const int dz = chunkZ - playerZ;
        return dx * dx + dz * dz <= renderDistance * renderDistance;
    }
};