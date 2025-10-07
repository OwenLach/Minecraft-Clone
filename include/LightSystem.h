#pragma once

#include "Block/BlockTypes.h"
#include "chunk/ChunkCoord.h"

#include <array>
#include <memory>
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
    // Update the chunk's lighting from its neighbors
    void updateBorderLighting(std::shared_ptr<Chunk> chunk);
    // Happens right after terrain gen, only propogates light within chunk
    void seedInitialSkylight(std::shared_ptr<Chunk> chunk);

private:
    World *world_;
    ChunkManager *chunkManager_;

    static constexpr std::array<glm::ivec3, 6>
        directions = {
            glm::ivec3(1, 0, 0), glm::ivec3(-1, 0, 0),
            glm::ivec3(0, 1, 0), glm::ivec3(0, -1, 0),
            glm::ivec3(0, 0, 1), glm::ivec3(0, 0, -1)};

    void seedFromNeighborChunks(std::shared_ptr<Chunk> chunk, std::queue<LightNode> &lightQueue);
    void clearChunkLightLevels(std::shared_ptr<Chunk> chunk);
    inline bool isTransparent(BlockType type) const { return type == BlockType::Air; }
};