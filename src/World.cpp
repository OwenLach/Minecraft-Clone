#include "World.h"
#include "FastNoiseLite.h"
#include <iostream>

World::World(Shader &shader, TextureAtlas *atlas) : shader(shader), textureAtlas(atlas)
{
    int renderDistance = Constants::RENDER_DISTANCE;

    for (int x = -renderDistance; x < renderDistance; x++)
    {
        for (int z = -renderDistance; z < renderDistance; z++)
        {
            chunkPositions.emplace(ChunkCoord{x, z}, std::make_unique<Chunk>(shader, textureAtlas, ChunkCoord{x, z}, this));
        }
    }

    for (int x = -renderDistance; x < renderDistance; x++)
    {
        for (int z = -renderDistance; z < renderDistance; z++)
        {
            chunkPositions[ChunkCoord{x, z}]->rebuildMesh();
        }
    }
}

void World::render()
{
    int renderDistance = Constants::RENDER_DISTANCE;
    for (int x = -renderDistance; x < renderDistance; x++)
    {
        for (int z = -renderDistance; z < renderDistance; z++)
        {
            chunkPositions.at(ChunkCoord{x, z})->renderChunk();
        }
    }
}

void World::update() const
{
}

glm::ivec3 World::chunkToWorldCoords(ChunkCoord chunkCoords, glm::ivec3 localPos) const
{
    int x = chunkCoords.x * Constants::CHUNK_SIZE_X + localPos.x;
    int y = localPos.y;
    int z = chunkCoords.z * Constants::CHUNK_SIZE_Z + localPos.z;

    return glm::ivec3(x, y, z);
}

glm::ivec3 World::worldToChunkCoords(glm::ivec3 worldCoords) const
{
    int chunkX = glm::floor(worldCoords.x / (float)Constants::CHUNK_SIZE_X);
    int chunkZ = glm::floor(worldCoords.z / (float)Constants::CHUNK_SIZE_Z);
    return glm::ivec3(chunkX, 0, chunkZ);
}

Block World::getBlockAt(ChunkCoord chunkCoords, glm::vec3 blockPos)
{
    auto it = chunkPositions.find(chunkCoords);
    if (it == chunkPositions.end())
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
    glm::ivec3 chunkCoords3D = worldToChunkCoords(worldCoords);
    ChunkCoord chunkCoord = {chunkCoords3D.x, chunkCoords3D.z}; // only x and z are used for chunk addressing

    // Convert world coordinates to local chunk coordinates
    // Modulo math ensures correct wrapping even with negative positions
    int localX = (worldCoords.x % Constants::CHUNK_SIZE_X + Constants::CHUNK_SIZE_X) % Constants::CHUNK_SIZE_X;
    int localY = worldCoords.y;
    int localZ = (worldCoords.z % Constants::CHUNK_SIZE_Z + Constants::CHUNK_SIZE_Z) % Constants::CHUNK_SIZE_Z;

    glm::ivec3 localBlockPos = glm::ivec3(localX, localY, localZ);

    // Attempt to find the chunk at the given chunk coordinates
    auto it = chunkPositions.find(chunkCoord);
    if (it == chunkPositions.end())
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
