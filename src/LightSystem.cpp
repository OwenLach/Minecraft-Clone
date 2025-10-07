#include "LightSystem.h"
#include "World.h"
#include "Chunk/Chunk.h"
#include "Chunk/ChunkCoord.h"
#include "Block/Block.h"
#include "Constants.h"
#include "Performance/ScopedTimer.h"

#include <glm/glm.hpp>

#include <memory>
#include <iostream>
#include <queue>
#include <array>

LightSystem::LightSystem(World *world, ChunkManager *manager) : world_(world), chunkManager_(manager)
{
}

void LightSystem::updateBorderLighting(std::shared_ptr<Chunk> chunk)
{
    using namespace Constants;

    std::queue<LightNode> lightQueue;
    seedFromNeighborChunks(chunk, lightQueue);

    while (!lightQueue.empty())
    {
        LightNode currNode = lightQueue.front();
        lightQueue.pop();

        // skip if chunk doesn't exist
        if (!currNode.chunk)
        {
            std::cout << "CHUNK DOESN'T EXIST" << std::endl;
            continue;
        }

        Block &currBlock = *currNode.chunk->getBlockLocal(currNode.localPos);
        for (const auto &dir : directions)
        {
            LightNode nNode{currNode.chunk, currNode.localPos + dir};

            // If neighbor position isn't in chunk bounds skip
            if (!Chunk::blockPosInChunkBounds(nNode.localPos))
                continue;

            Block *nBlockPtr = nNode.chunk->getBlockLocal(nNode.localPos);

            if (!isTransparent(nBlockPtr->type))
                continue;

            int potential_new_light;
            // Light doesn't dim downwards
            if (dir.y == -1)
            {
                potential_new_light = currBlock.skylight;
            }
            // Regular for all other directions
            else
            {
                potential_new_light = currBlock.skylight - 1;
            }

            if (potential_new_light > nBlockPtr->skylight)
            {
                nBlockPtr->skylight = static_cast<uint8_t>(potential_new_light);
                lightQueue.push(nNode);
            }
        }
    }
}

void LightSystem::seedInitialSkylight(std::shared_ptr<Chunk> chunk)
{
    using namespace Constants;

    std::queue<LightNode> lightQueue;
    auto &blocks = chunk->getBlocks();

    // 1. Set all air blocks in column to light level 15 and push to queue until the first solid block is reached
    for (int x = 0; x < CHUNK_SIZE_X; x++)
    {
        for (int z = 0; z < CHUNK_SIZE_Z; z++)
        {
            for (int y = CHUNK_SIZE_Y - 1; y >= 0; y--)
            {
                int index = Chunk::getBlockIndex({x, y, z});
                Block &block = blocks[index];

                if (!isTransparent(block.type))
                    break;

                block.skylight = 15;
                lightQueue.push({chunk, {x, y, z}});
            }
        }
    }

    // 2. Progate the light within current chunk only
    while (!lightQueue.empty())
    {
        LightNode currNode = lightQueue.front();
        lightQueue.pop();

        if (!currNode.chunk)
        {
            std::cout << "CHUNK DOESN'T EXIST" << std::endl;
            continue;
        }

        Block &currBlock = *currNode.chunk->getBlockLocal(currNode.localPos);
        for (const auto &dir : directions)
        {
            // Don't go up on initial skylight
            if (dir.y == 1)
                continue;

            glm::ivec3 nPos = currNode.localPos + dir;

            // If local position isn't in chunk bounds, skip
            if (!Chunk::blockPosInChunkBounds(nPos))
                continue;

            Block *nBlockPtr = chunk->getBlockLocal(nPos);

            if (!isTransparent(nBlockPtr->type))
                continue;

            int potential_new_light;
            // Light doesn't dim downwards
            if (dir.y == -1)
            {
                potential_new_light = currBlock.skylight;
            }
            // Regular for all other directions
            else
            {
                potential_new_light = currBlock.skylight - 1;
            }

            if (potential_new_light > nBlockPtr->skylight)
            {
                nBlockPtr->skylight = static_cast<uint8_t>(potential_new_light);
                lightQueue.push({chunk, nPos});
            }
        }
    }
}

void LightSystem::seedFromNeighborChunks(std::shared_ptr<Chunk> chunk, std::queue<LightNode> &lightQueue)
{
    using namespace Constants;

    auto neighbors = chunkManager_->getChunkNeighbors(chunk->getCoord());
    auto westChunk = neighbors[3];
    auto eastChunk = neighbors[2];
    auto southChunk = neighbors[1];
    auto northChunk = neighbors[0];

    auto &currBlocks = chunk->getBlocks();

    if (westChunk)
    {
        auto &westBlocks = westChunk->getBlocks();

        for (int y = 0; y < CHUNK_SIZE_Y; y++)
        {
            for (int z = 0; z < CHUNK_SIZE_Z; z++)
            {
                int currI = Chunk::getBlockIndex({0, y, z});
                int nI = Chunk::getBlockIndex({CHUNK_SIZE_X - 1, y, z});

                Block &currBlock = currBlocks[currI];
                Block &nBlock = westBlocks[nI];

                if (nBlock.skylight <= 0)
                    continue;

                uint8_t potential_new_level = nBlock.skylight - 1;
                if (isTransparent(currBlock.type) &&
                    potential_new_level > currBlock.skylight &&
                    potential_new_level > 0)
                {
                    currBlock.skylight = potential_new_level;
                    lightQueue.push({chunk, {0, y, z}});
                }
            }
        }
    }

    if (eastChunk)
    {
        auto &eastBlocks = eastChunk->getBlocks();

        for (int y = 0; y < CHUNK_SIZE_Y; y++)
        {
            for (int z = 0; z < CHUNK_SIZE_Z; z++)
            {
                int currI = Chunk::getBlockIndex({CHUNK_SIZE_X - 1, y, z});
                int nI = Chunk::getBlockIndex({0, y, z});

                Block &currBlock = currBlocks[currI];
                Block &nBlock = eastBlocks[nI];

                if (nBlock.skylight <= 0)
                    continue;

                uint8_t potential_new_level = nBlock.skylight - 1;
                if (isTransparent(currBlock.type) &&
                    potential_new_level > currBlock.skylight &&
                    potential_new_level > 0)
                {
                    currBlock.skylight = potential_new_level;
                    lightQueue.push({chunk, {CHUNK_SIZE_X - 1, y, z}});
                }
            }
        }
    }

    if (southChunk)
    {
        auto &southBlocks = southChunk->getBlocks();

        for (int y = 0; y < CHUNK_SIZE_Y; y++)
        {
            for (int x = 0; x < CHUNK_SIZE_X; x++)
            {
                int currI = Chunk::getBlockIndex({x, y, 0});
                int nI = Chunk::getBlockIndex({x, y, CHUNK_SIZE_Z - 1});

                Block &currBlock = currBlocks[currI];
                Block &nBlock = southBlocks[nI];

                if (nBlock.skylight <= 0)
                    continue;

                uint8_t potential_new_level = nBlock.skylight - 1;
                if (isTransparent(currBlock.type) &&
                    potential_new_level > currBlock.skylight &&
                    potential_new_level > 0)
                {
                    currBlock.skylight = potential_new_level;
                    lightQueue.push({chunk, {x, y, 0}});
                }
            }
        }
    }

    if (northChunk)
    {
        auto &northBlocks = northChunk->getBlocks();

        for (int y = 0; y < CHUNK_SIZE_Y; y++)
        {
            for (int x = 0; x < CHUNK_SIZE_X; x++)
            {
                int currI = Chunk::getBlockIndex({x, y, CHUNK_SIZE_Z - 1});
                int nI = Chunk::getBlockIndex({x, y, 0});

                Block &currBlock = currBlocks[currI];
                Block &nBlock = northBlocks[nI];

                if (nBlock.skylight <= 0)
                    continue;

                uint8_t potential_new_level = nBlock.skylight - 1;
                if (isTransparent(currBlock.type) &&
                    potential_new_level > currBlock.skylight &&
                    potential_new_level > 0)
                {
                    currBlock.skylight = potential_new_level;
                    lightQueue.push({chunk, {x, y, CHUNK_SIZE_Z - 1}});
                }
            }
        }
    }
}

void LightSystem::clearChunkLightLevels(std::shared_ptr<Chunk> chunk)
{
    for (auto &block : chunk->getBlocks())
        block.skylight = 0;
}