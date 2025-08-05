#include "LightSystem.h"
#include "World.h"
#include "Chunk/Chunk.h"
#include "Block/Block.h"
#include "Constants.h"

#include <glm/glm.hpp>

#include <memory>
#include <iostream>

LightSystem::LightSystem(World *world) : world_(world)
{
}

void LightSystem::propogateSkylight(std::shared_ptr<Chunk> chunk)
{
    // for each column set light level to 15 until block is solid
    // then propogate outward

    // using namespace Constants;

    // int chunkHeight = CHUNK_SIZE_Y - 1;

    // for (int x = 0; x < CHUNK_SIZE_X; x++)
    // {
    //     for (int z = 0; z < CHUNK_SIZE_Z; z++)
    //     {
    //         for (int y = chunkHeight - 1; y >= 0; y--)
    //         {
    //             // Block &block = chunk->getBlockLocal(glm::ivec3(x, y, z));
    //             // glm::ivec3 worldPos = world_->localToGlobalPos(chunk->getCoord(), glm::ivec3(x, y, z));

    //             // if (world_->isBlockSolid(worldPos))
    //             //     break;

    //             // block.skylight = 15;
    //         }
    //         break;
    //     }
    //     break;
    // }
}
