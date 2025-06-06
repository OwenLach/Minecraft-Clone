#include "World.h"
#include "FastNoiseLite.h"
#include <iostream>

World::World(Shader &shader, TextureAtlas *atlas, Camera &camera) : shader(shader), textureAtlas(atlas), camera(camera)
{
    const int renderDistance = Constants::RENDER_DISTANCE;

    for (int x = -renderDistance; x < renderDistance; x++)
    {
        for (int z = -renderDistance; z < renderDistance; z++)
        {
            if (isInRenderDistance(x, z))
            {
                loadChunk(ChunkCoord{x, z});
            }
        }
    }

    for (int x = -renderDistance; x < renderDistance; x++)
    {
        for (int z = -renderDistance; z < renderDistance; z++)
        {
            if (isInRenderDistance(x, z))
            {
                loadedChunks[ChunkCoord{x, z}]->updateMesh();
                ChunksRendered++;
            }
        }
    }
}

void World::render()
{
    // int renderDistance = Constants::RENDER_DISTANCE;
    // for (int x = -renderDistance; x < renderDistance; x++)
    // {
    //     for (int z = -renderDistance; z < renderDistance; z++)
    //     {
    //         if (isInRenderDistance(x, z))
    //         {
    //             loadedChunks.at(ChunkCoord{x, z})->renderChunk();
    //         }
    //     }
    // }

    for (auto &[pos, chunk] : loadedChunks)
    {
        chunk->renderChunk();
    }
}

void World::update()
{
    int renderDistance = Constants::RENDER_DISTANCE;
    ChunkCoord playerPos = worldToChunkCoords(glm::ivec3(camera.Position));

    for (int x = -renderDistance; x < renderDistance; x++)
    {
        for (int z = -renderDistance; z < renderDistance; z++)
        {
            ChunkCoord chunkCoord{playerPos.x + x, playerPos.z + z};
            if (loadedChunks.find(chunkCoord) == loadedChunks.end())
            {
                loadChunk(chunkCoord);
            }
        }
    }
}

void World::loadChunk(ChunkCoord coord)
{
    auto chunk = std::make_unique<Chunk>(shader, textureAtlas, coord, this);
    chunk->updateMesh();
    loadedChunks.emplace(coord, std::move(chunk));
}

glm::ivec3 World::chunkToWorldCoords(ChunkCoord chunkCoords, glm::ivec3 localPos) const
{
    int x = chunkCoords.x * Constants::CHUNK_SIZE_X + localPos.x;
    int y = localPos.y;
    int z = chunkCoords.z * Constants::CHUNK_SIZE_Z + localPos.z;

    return glm::ivec3(x, y, z);
}

ChunkCoord World::worldToChunkCoords(glm::ivec3 worldCoords) const
{
    int chunkX = glm::floor(worldCoords.x / (float)Constants::CHUNK_SIZE_X);
    int chunkZ = glm::floor(worldCoords.z / (float)Constants::CHUNK_SIZE_Z);
    return {chunkX, chunkZ};
}

Block World::getBlockAt(ChunkCoord chunkCoords, glm::vec3 blockPos)
{
    auto it = loadedChunks.find(chunkCoords);
    if (it == loadedChunks.end())
    {
        static Block airBlock;
        return airBlock;
    }

    Chunk *chunk = it->second.get();
    return chunk->getBlockAt(blockPos);
}

Block World::getBlockAt(glm::vec3 worldPos)
{
    // Round the world position down to the nearest integers to get block-aligned coordinates
    glm::ivec3 worldCoords = glm::floor(worldPos);

    // Convert world coordinates to chunk coordinates
    ChunkCoord chunkCoord = worldToChunkCoords(worldCoords);

    // Convert world coordinates to local chunk coordinates
    // Modulo math ensures correct wrapping even with negative positions
    int localX = (worldCoords.x % Constants::CHUNK_SIZE_X + Constants::CHUNK_SIZE_X) % Constants::CHUNK_SIZE_X;
    int localY = worldCoords.y;
    int localZ = (worldCoords.z % Constants::CHUNK_SIZE_Z + Constants::CHUNK_SIZE_Z) % Constants::CHUNK_SIZE_Z;

    glm::ivec3 localBlockPos = glm::ivec3(localX, localY, localZ);

    // Attempt to find the chunk at the given chunk coordinates
    auto it = loadedChunks.find(chunkCoord);
    if (it == loadedChunks.end())
    {
        // If the chunk doesn't exist (e.g., out of render distance), return a default air block.
        // This helps avoid errors and lets face culling work properly on chunk edges.
        return Block(BlockType::Air, glm::ivec3(0));
    }

    // Sanity check: make sure local position is actually within bounds of the chunk
    // This shouldn't happen due to the modulo math, but the check is defensive
    if (localX >= 0 && localX < Constants::CHUNK_SIZE_X &&
        localY >= 0 && localY < Constants::CHUNK_SIZE_Y &&
        localZ >= 0 && localZ < Constants::CHUNK_SIZE_Z)
    {
        // If everything is valid, fetch the block from the chunk
        return it->second->getBlockAt(glm::ivec3(localX, localY, localZ));
    }
    else
    {
        // If something went wrong (e.g., Y is out of bounds), return air to avoid a crash
        return Block(BlockType::Air, glm::ivec3(0));
    }
}
