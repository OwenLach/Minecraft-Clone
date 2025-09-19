#include "LightSystem.h"
#include "World.h"
#include "Chunk/Chunk.h"
#include "Chunk/ChunkCoord.h"
#include "Block/Block.h"
#include "Constants.h"
#include "Performance/ScopedTimer.h"

#include <glm/glm.hpp>

#include <memory>
#include <unordered_map>
#include <iostream>
#include <queue>
#include <array>
#include <cassert>
#include <mutex>

LightSystem::LightSystem(World *world, ChunkManager *manager) : world_(world), chunkManager_(manager)
{
}

void LightSystem::propagateSkylight(std::shared_ptr<Chunk> chunk)
{
    using namespace Constants;
    // ScopedTimer timer("propogateLight()");

    auto &blocks = chunk->getBlocks();

    seedFromNeighborChunks(chunk);

    while (true)
    {
        LightNode currNode;
        bool hasNode = false;

        {
            std::lock_guard<std::mutex> lock(lightMutex_);
            if (!lightQueue_.empty())
            {
                currNode = lightQueue_.front();
                lightQueue_.pop();
                hasNode = true;
            }
        }

        if (!hasNode)
            break;

        Block &currBlock = *currNode.chunk->getBlockLocal(currNode.localPos);
        for (const auto &dir : directions)
        {
            LightNode nNode{currNode.chunk, currNode.localPos + dir};

            // Check if block is out of Y bounds first
            if (nNode.localPos.y < 0 || nNode.localPos.y >= CHUNK_SIZE_Y)
                continue;

            // If local position isn't in chunk bounds, get the correct chunk and localPos
            if (!Chunk::blockPosInChunkBounds(nNode.localPos))
                getNodeFromNeighbor(nNode);

            if (!nNode.chunk)
            {
                // std::cout << "CHUNK DOESN'T EXIST" << std::endl;
                continue;
            }

            assert(Chunk::blockPosInChunkBounds(nNode.localPos));

            Block *nBlockPtr = nNode.chunk->getBlockLocal(nNode.localPos);
            if (!nBlockPtr)
            {
                assert(false && "BLOCK DOESN'T EXIST, THIS SHOULD NOT HAPPEN");
                continue;
            }

            if (isTransparent(nBlockPtr->type))
            {
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
                    addToQueue(nNode);
                }
            }
        }
    }
}

void LightSystem::seedInitialSkylight(std::shared_ptr<Chunk> chunk)
{
    using namespace Constants;

    auto &blocks = chunk->getBlocks();

    for (int x = 0; x < CHUNK_SIZE_X; x++)
    {
        for (int z = 0; z < CHUNK_SIZE_Z; z++)
        {
            for (int y = CHUNK_SIZE_Y - 1; y >= 0; y--)
            {
                int index = Chunk::getBlockIndex({x, y, z});
                Block &block = blocks[index];

                // Stop when first solid block reached
                if (!isTransparent(block.type))
                    break;

                // Set all air blocks in column to light level 15 until the first solid block is reached
                block.skylight = 15;
                addToQueue({chunk, {x, y, z}});
            }
        }
    }
}

void LightSystem::updateNeighborLights(std::shared_ptr<Chunk> chunk)
{
    auto neighbors = chunkManager_->getChunkNeighborsFromCache(chunk->getCoord());
    for (const auto &n : neighbors)
    {
        if (n && n->getState() == ChunkState::LOADED)
            n->setState(ChunkState::NEEDS_LIGHT_UPDATE);
    }
}

void LightSystem::seedFromNeighborChunks(std::shared_ptr<Chunk> chunk)
{
    using namespace Constants;

    auto neighbors = chunkManager_->getChunkNeighborsFromCache(chunk->getCoord());
    auto westChunk = neighbors[3];
    auto eastChunk = neighbors[2];
    auto southChunk = neighbors[1];
    auto northChunk = neighbors[0];

    auto currBlocks = chunk->getBlocks();

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
                if (isTransparent(currBlock.type) && (potential_new_level > currBlock.skylight))
                {
                    currBlock.skylight = potential_new_level;
                    addToQueue({chunk, {0, y, z}});
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
                if (isTransparent(currBlock.type) && (potential_new_level > currBlock.skylight))
                {
                    currBlock.skylight = potential_new_level;
                    addToQueue({chunk, {CHUNK_SIZE_X - 1, y, z}});
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
                if (isTransparent(currBlock.type) && (potential_new_level > currBlock.skylight))
                {
                    currBlock.skylight = potential_new_level;
                    addToQueue({chunk, {x, y, 0}});
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
                if (isTransparent(currBlock.type) && (potential_new_level > currBlock.skylight))
                {
                    currBlock.skylight = potential_new_level;
                    addToQueue({chunk, {x, y, CHUNK_SIZE_Z - 1}});
                }
            }
        }
    }
}

void LightSystem::getNodeFromNeighbor(LightNode &n)
{
    using namespace Constants;

    auto nLocalPos = n.localPos;
    ChunkCoord originCoord = n.chunk->getCoord();

    // neighbors[0] = North neighbor
    // neighbors[1] = South neighbor
    // neighbors[2] = East neighbor
    // neighbors[3] = West neighbor
    if (nLocalPos.x < 0 && nLocalPos.z >= 0 && nLocalPos.z < CHUNK_SIZE_Z)
    {
        auto westChunk = chunkManager_->getChunkNeighborsFromCache(originCoord)[3];
        if (westChunk)
        {
            n.chunk = westChunk;
            n.localPos = {nLocalPos.x + CHUNK_SIZE_X, nLocalPos.y, nLocalPos.z};
        }
        else
        {
            n.chunk = nullptr;
        }
    }
    // Check East neighbor (+X direction)
    else if (nLocalPos.x >= CHUNK_SIZE_X && nLocalPos.z >= 0 && nLocalPos.z < CHUNK_SIZE_Z)
    {
        auto eastChunk = chunkManager_->getChunkNeighborsFromCache(originCoord)[2];
        if (eastChunk)
        {
            n.chunk = eastChunk;
            n.localPos = {nLocalPos.x - CHUNK_SIZE_X, nLocalPos.y, nLocalPos.z};
        }
        else
        {
            n.chunk = nullptr;
        }
    }
    // Check South neighbor (-Z direction)
    else if (nLocalPos.z < 0 && nLocalPos.x >= 0 && nLocalPos.x < CHUNK_SIZE_X)
    {
        auto southChunk = chunkManager_->getChunkNeighborsFromCache(originCoord)[1];
        if (southChunk)
        {
            n.chunk = southChunk;
            n.localPos = {nLocalPos.x, nLocalPos.y, nLocalPos.z + CHUNK_SIZE_Z};
        }
        else
        {
            n.chunk = nullptr;
        }
    }
    // Check North neighbor (+Z direction)
    else if (nLocalPos.z >= CHUNK_SIZE_Z && nLocalPos.x >= 0 && nLocalPos.x < CHUNK_SIZE_X)
    {
        auto northChunk = chunkManager_->getChunkNeighborsFromCache(originCoord)[0];
        if (northChunk)
        {
            n.chunk = northChunk;
            n.localPos = {nLocalPos.x, nLocalPos.y, nLocalPos.z - CHUNK_SIZE_Z};
        }
        else
        {
            n.chunk = nullptr;
        }
    }
    else
    {
        assert(false && "THIS SHOULD NEVER HAPPEN");
    }
}

void LightSystem::addToQueue(const LightNode &n)
{
    std::lock_guard<std::mutex> lock(lightMutex_);
    lightQueue_.push(n);
}

void LightSystem::clearChunkLightLevels(std::shared_ptr<Chunk> chunk)
{
    // ScopedTimer timer("clearChunkLightLevels");
    for (auto &block : chunk->getBlocks())
        block.skylight = 0;
}