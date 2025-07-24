#pragma once

#include "Chunk/ChunkManager.h"
#include "Chunk/ChunkPipeline.h"
#include "Raycaster.h"
#include "Constants.h"
#include "Block/BlockOutline.h"

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

    void breakBlock();
    void placeBlock();

    bool isBlockSolid(glm::ivec3 blockWorldPos) const;

private:
    Camera &camera_;
    Shader &shader_;
    TextureAtlas &textureAtlas_;

    ChunkManager chunkManager_;
    ChunkPipeline pipeline_;
    ChunkCoord lastPlayerChunk_;
    Raycaster raycaster;
    BlockOutline blockOutline_;
    glm::ivec3 targetBlockPos_;
    bool hasTargetBlock_ = false;

    void loadNewChunks(ChunkCoord center);
    void unloadDistantChunks();
    void updateChunkStates();
    void updateBlockOutline();

    // Coordinate conversions
    glm::ivec3 chunkToWorldCoords(ChunkCoord chunkCoords, glm::ivec3 localPos) const;
    ChunkCoord worldToChunkCoords(glm::ivec3 worldCoords) const;

    // Block helper
    Block getBlockGlobal(glm::vec3 worldPos) const;
    Block getBlockLocal(ChunkCoord chunkCoords, glm::vec3 blockPos);
    glm::ivec3 getBlockLocalPosition(glm::ivec3 worldPos) const;

    static inline bool isInRenderDistance(int chunkX, int chunkZ, int playerX, int playerZ)
    {
        const int renderDistance = Constants::RENDER_DISTANCE;
        const int dx = chunkX - playerX;
        const int dz = chunkZ - playerZ;
        return dx * dx + dz * dz <= renderDistance * renderDistance;
    }
};