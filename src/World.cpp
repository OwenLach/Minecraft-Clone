#include "World.h"
#include "ThreadPool.h"
#include "FastNoiseLite.h"
#include "Chunk.h"

#include <iostream>
#include <thread>
#include <algorithm>

World::World(Shader &shader, TextureAtlas *atlas, Camera &camera)
    : shader(shader),
      textureAtlas(atlas),
      camera(camera),
      chunkThreadPool(std::thread::hardware_concurrency() > 1 ? std::thread::hardware_concurrency() - 1 : 1)
{
    loadInitialChunks();
}

void World::render()
{
    int totalChunks = 0;
    int chunksRendered = 0;
    int chunksInFrustum = 0;
    int chunksOutOfFrustum = 0;

    for (auto &[pos, chunk] : loadedChunks)
    {
        totalChunks++;
        if (chunk->state == ChunkState::LOADED)
        {
            if (camera.isAABBInFrustum(chunk->boundingBox))
            {
                chunk->renderChunk();
                chunksRendered++;
                chunksInFrustum++;
            }
            else
            {
                chunksOutOfFrustum++;
            }
        }
    }

    // Print every few frames to avoid spam
    // static int frameCount = 0;
    // if (frameCount++ % 60 == 0)
    // {
    //     printf("Total: %d, Loaded: %d, In Frustum: %d, Out: %d, Rendered: %d\n",
    //            totalChunks, chunksInFrustum + chunksOutOfFrustum,
    //            chunksInFrustum, chunksOutOfFrustum, chunksRendered);
    // }
}

void World::update()
{
    int renderDistance = Constants::RENDER_DISTANCE;
    ChunkCoord playerPos = worldToChunkCoords(glm::ivec3(camera.Position));

    // load new chunks
    loadVisibleChunks(playerPos, renderDistance);
    // 2. Process chunks ready for mesh generation
    processTerrainToMesh();
    // 3. Process chunks ready for GPU upload
    processMeshToGPU();
    // 4. Unload chunks
    unloadDistantChunks(playerPos);
}

void World::loadChunk(ChunkCoord coord)
{
    // if its already loaded, return
    if (loadedChunks.find(coord) != loadedChunks.end())
        return;

    // Create the chunk object. It starts in EMPTY state.
    auto chunk = std::make_shared<Chunk>(shader, textureAtlas, coord, this);
    chunk->state = ChunkState::TERRAIN_GENERATING;
    loadedChunks.emplace(coord, chunk);

    // Start terrain generation task for the worker threads
    chunkThreadPool.enqueue([this, chunk]() { //
        // starts generating terrain on worker thread
        chunk->generateTerrain();
        {
            // get the lock to change state
            std::unique_lock<std::mutex> mutex(meshGenMutex);
            chunk->state = ChunkState::TERRAIN_GENERATED;
            // push to mesh gen queue
            meshGenQueue.push(chunk);
        }
    });
}

void World::unloadChunk(ChunkCoord coord)
{
    loadedChunks.erase(coord);
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

Block World::getBlockAt(glm::vec3 worldPos) const
{
    static Block airBlock;
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

bool World::isBlockSolid(glm::ivec3 blockWorldPos) const
{
    Block block = getBlockAt(glm::vec3(blockWorldPos));
    return block.type != BlockType::Air;
}

void World::loadInitialChunks()
{
    const int renderDistance = Constants::RENDER_DISTANCE;
    const ChunkCoord playerPos = worldToChunkCoords(glm::ivec3(camera.Position));

    std::vector<ChunkCoord> orderedChunks;
    // loops in a square
    for (int x = -renderDistance; x < renderDistance; x++)
    {
        for (int z = -renderDistance; z < renderDistance; z++)
        {
            // circular render distance
            if (isInRenderDistance(x, z, playerPos.x, playerPos.z))
            {
                orderedChunks.push_back(ChunkCoord{playerPos.x + x, playerPos.z + z});
            }
        }
    }

    // sort by distance from player
    std::sort(orderedChunks.begin(), orderedChunks.end(), [&playerPos, this](const ChunkCoord &a, const ChunkCoord &b)
              { return getChunkDistanceSquaredFromPlayer(playerPos, a) < getChunkDistanceSquaredFromPlayer(playerPos, b); });

    // load closest to player first
    for (const auto &chunk : orderedChunks)
    {
        loadChunk(chunk);
    }
}

void World::loadVisibleChunks(const ChunkCoord &playerPos, const int renderDistance)
{
    for (int x = -renderDistance; x < renderDistance; x++)
    {
        for (int z = -renderDistance; z < renderDistance; z++)
        {
            ChunkCoord chunkCoord{playerPos.x + x, playerPos.z + z};
            if (loadedChunks.find(chunkCoord) == loadedChunks.end() && isInRenderDistance(chunkCoord.x, chunkCoord.z, playerPos.x, playerPos.z))
            {
                loadChunk(chunkCoord);
            }
        }
    }
}

void World::processTerrainToMesh()
{
    // iterate through the meshGenQueue
    std::vector<std::shared_ptr<Chunk>> meshGenReady;
    {
        // get a lock since it access meshGenQueue
        std::unique_lock<std::mutex> lock(meshGenMutex);
        meshGenReady.reserve(meshGenQueue.size());
        // push everthing to meshGenReady vector
        while (!meshGenQueue.empty())
        {
            meshGenReady.push_back(meshGenQueue.front());
            meshGenQueue.pop();
        }
    }

    for (const auto &chunk : meshGenReady)
    {
        // make sure chunk's terrain is generated and it isn't unloaded
        if (chunk->state == ChunkState::TERRAIN_GENERATED && loadedChunks.find(chunk->chunkCoord) != loadedChunks.end())
        {
            // start mesh generation on worker thread
            chunkThreadPool.enqueue([this, chunk]() { //
                chunk->generateMesh();
                {
                    std::unique_lock<std::mutex> lock(uploadMutex);
                    chunk->state = ChunkState::MESH_READY_FOR_UPLOAD;
                    uploadQueue.push(chunk);
                }
            });
        }
    }
}

void World::processMeshToGPU()
{
    std::vector<std::shared_ptr<Chunk>> uploadReady;
    {
        // get a lock since it access meshGenQueue
        std::unique_lock<std::mutex> lock(uploadMutex);
        uploadReady.reserve(uploadQueue.size());
        // push everthing to meshGenReady vector
        int count = 0;
        while (!uploadQueue.empty() && count < Constants::MAX_CHUNKS_PER_FRAME)
        {
            uploadReady.push_back(uploadQueue.front());
            uploadQueue.pop();
            count++;
        }
    }

    for (const auto &chunk : uploadReady)
    {
        // make sure chunk isn't unloaded
        if (chunk->state == ChunkState::MESH_READY_FOR_UPLOAD && loadedChunks.find(chunk->chunkCoord) != loadedChunks.end())
        {
            // Keep on main thread since it uses OpenGL related things
            chunk->uploadMeshToGPU();
            chunk->state = ChunkState::LOADED;
        }
    }
}

void World::unloadDistantChunks(const ChunkCoord &playerPos)
{
    std::vector<ChunkCoord> chunkPositionsToRemove;
    for (auto &[pos, chunk] : loadedChunks)
    {
        // if chunk is in the middle of processing anything, don't unload
        if (!isInRenderDistance(pos.x, pos.z, playerPos.x, playerPos.z) &&
            chunk->state != ChunkState::TERRAIN_GENERATING &&
            chunk->state != ChunkState::MESH_GENERATING &&
            chunk->state != ChunkState::MESH_READY_FOR_UPLOAD)
        {
            chunkPositionsToRemove.push_back(pos);
        }
    }

    for (const ChunkCoord coord : chunkPositionsToRemove)
    {
        unloadChunk(coord);
    }
}

int World::getChunkDistanceSquaredFromPlayer(const ChunkCoord &chunk, const ChunkCoord &playerPos) const
{
    const int dx = chunk.x - playerPos.x;
    const int dz = chunk.z - playerPos.z;

    return dx * dx + dz * dz;
}
