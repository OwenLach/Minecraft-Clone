#include "LightSystem.h"
#include "World.h"
#include "Chunk/Chunk.h"
#include "Block/Block.h"
#include "Constants.h"
#include "Performance/ScopedTimer.h"

#include <glm/glm.hpp>

#include <memory>
#include <iostream>
#include <deque>
#include <array>

LightSystem::LightSystem(World *world) : world_(world)
{
}

void LightSystem::propagateSkylight(std::shared_ptr<Chunk> chunk)
{
    // for each column set light level to 15 until block is solid
    // then propogate outward
    ScopedTimer timer("PropogateLight()");
    using namespace Constants;

    auto isTransparent = [](BlockType type) -> bool
    {
        return type == BlockType::Air;
    };

    clearChunkLightLevels(chunk);

    std::deque<glm::ivec3> lightQueue;

    for (int x = 0; x < CHUNK_SIZE_X; x++)
    {
        for (int z = 0; z < CHUNK_SIZE_Z; z++)
        {
            for (int y = CHUNK_SIZE_Y - 1; y >= 0; y--)
            {
                Block &block = chunk->getBlockLocal(glm::ivec3(x, y, z));
                glm::ivec3 worldPos = world_->localToGlobalPos(chunk->getCoord(), glm::ivec3(x, y, z));

                // Stop when first solid block reached
                if (!isTransparent(block.type))
                    break;

                // Set all air blocks in column to light level 15 until the first solid block is reached
                block.skylight = 15;
                lightQueue.push_back(worldPos);
            }
        }
    }

    const std::array<glm::ivec3, 6> directions = {
        glm::ivec3(1, 0, 0), glm::ivec3(-1, 0, 0),
        glm::ivec3(0, 1, 0), glm::ivec3(0, -1, 0),
        glm::ivec3(0, 0, 1), glm::ivec3(0, 0, -1)};

    while (!lightQueue.empty())
    {
        // Get the block from the queue
        glm::ivec3 worldPos = lightQueue.front();
        lightQueue.pop_front();

        // Get from global pos
        Block *currBlockPtr = world_->getBlockGlobal(worldPos);
        if (!currBlockPtr)
            continue;

        Block &currBlock = *currBlockPtr;

        // Loop though all directions to get each neighbors
        for (const auto &dir : directions)
        {
            // Caculate neighbors world pos and get the actual block pointer to it
            const glm::ivec3 nPos = worldPos + dir;
            Block *neighborPtr = world_->getBlockGlobal(nPos);

            // Skip if didn't find
            if (!neighborPtr)
                continue;

            Block &neighborBlock = *neighborPtr;
            if (isTransparent(neighborBlock.type))
            {
                int potential_new_light;
                // Special downwards case, light may not dim
                if (dir.y == -1 && currBlock.skylight == 15)
                {
                    potential_new_light = currBlock.skylight;
                }
                // Regular for all other directions
                else
                {
                    potential_new_light = currBlock.skylight - 1;
                }

                if (potential_new_light > neighborBlock.skylight)
                {
                    neighborBlock.skylight = static_cast<uint8_t>(potential_new_light);
                    lightQueue.push_back(nPos);
                }
            }
        }
    }
}

void LightSystem::clearChunkLightLevels(std::shared_ptr<Chunk> chunk)
{
    using namespace Constants;

    for (int x = 0; x < CHUNK_SIZE_X; ++x)
        for (int y = 0; y < CHUNK_SIZE_Y; ++y)
            for (int z = 0; z < CHUNK_SIZE_Z; ++z)
            {
                chunk->getBlockLocal(glm::ivec3(x, y, z)).skylight = 0;
            }
}
