#pragma once

#include "Chunk/ChunkManager.h"
#include "Block/BlockOutline.h"
#include "Block/BlockTypes.h"
#include "Raycaster.h"
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
    World(Camera &camera);

    void update();
    void render();
    void breakBlock();
    void placeBlock();
    void setPlayerBlockType(BlockType type);

    bool isBlockSolid(glm::ivec3 blockWorldPos) const;

private:
    Camera &camera_;
    ChunkManager chunkManager_;
    ChunkCoord lastPlayerChunk_;
    BlockType playerBlockType_ = BlockType::Dirt;

    Raycaster raycaster;
    BlockOutline blockOutline_;
    glm::ivec3 targetBlockPos_;
    bool hasTargetBlock_ = false;

    void loadNewChunks(ChunkCoord center);
    void unloadDistantChunks();
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