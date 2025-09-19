#pragma once

#include "Block/BlockTypes.h"
#include "chunk/ChunkCoord.h"

#include <array>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <queue>

#include <glm/glm.hpp>

class World;
class ChunkManager;
class Chunk;
class Block;

struct LightNode
{
    std::shared_ptr<Chunk> chunk;
    glm::ivec3 localPos;
};

class LightSystem
{
public:
    LightSystem(World *world, ChunkManager *manager);
    void propagateSkylight(std::shared_ptr<Chunk> chunk);
    // Happens right after terrain gen, only sets the inital airblocks above solid ground to 15, does no propogation
    void seedInitialSkylight(std::shared_ptr<Chunk> chunk);
    void updateNeighborLights(std::shared_ptr<Chunk> chunk);

private:
    World *world_;
    ChunkManager *chunkManager_;

    static constexpr std::array<glm::ivec3, 6>
        directions = {
            glm::ivec3(1, 0, 0), glm::ivec3(-1, 0, 0),
            glm::ivec3(0, 1, 0), glm::ivec3(0, -1, 0),
            glm::ivec3(0, 0, 1), glm::ivec3(0, 0, -1)};

    void seedFromNeighborChunks(std::shared_ptr<Chunk> chunk);
    void getNodeFromNeighbor(LightNode &n);
    void addToQueue(const LightNode &n);
    void clearChunkLightLevels(std::shared_ptr<Chunk> chunk);
    inline bool isTransparent(BlockType type) const { return type == BlockType::Air; }

    std::queue<LightNode> lightQueue_;
    std::mutex lightMutex_;
};

// two system pass, removal - addition