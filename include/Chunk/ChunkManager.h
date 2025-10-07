#pragma once

#include "Chunk/Chunk.h"
#include "Chunk/ChunkCoord.h"
#include "Block/Block.h"
#include "Shader.h"
#include "TextureAtlas.h"
#include "Camera.h"

#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <chrono>
#include <vector>
#include <memory>

class ChunkPipeline;

struct StateChangeEvent
{
    std::shared_ptr<Chunk> chunk;
    ChunkState newState;
};

class ChunkManager
{
public:
    ChunkManager(Camera &camera);

    void init(ChunkPipeline *pipeline);
    void addChunk(const ChunkCoord &coord);
    void removeChunk(const ChunkCoord &coord);
    void renderAllChunks();
    void renderChunk(std::shared_ptr<Chunk> chunk, const ChunkCoord &pos);
    void update();
    void notifyStateChange(StateChangeEvent event);
    void notifyDependentNeighbors(std::shared_ptr<Chunk> chunk, ChunkState newState);

    template <typename Visitor>
    void forEachChunk(Visitor &&v)
    {
        for (const auto [coord, chunk] : chunks_)
        {
            if (!chunk)
                continue;
            v(coord, chunk);
        }
    }

    const TextureAtlas &getTextureAtlasRef() const;
    const std::shared_ptr<Chunk> getChunk(const ChunkCoord &coord) const;
    std::array<std::shared_ptr<Chunk>, 4> getChunkNeighbors(const ChunkCoord &coord);

private:
    std::unordered_map<ChunkCoord, std::shared_ptr<Chunk>> chunks_;

    std::unordered_set<std::shared_ptr<Chunk>> readyForTerrainGen_;
    std::unordered_set<std::shared_ptr<Chunk>> readyForInitLighting_;
    std::unordered_set<std::shared_ptr<Chunk>> readyForFinalLighting_;
    std::unordered_set<std::shared_ptr<Chunk>> readyForMeshing_;
    std::unordered_set<std::shared_ptr<Chunk>> readyForUpload_;
    std::unordered_set<std::shared_ptr<Chunk>> readyForRemesh_;

    std::queue<StateChangeEvent> stateChangeQueue_;

    Camera &camera_;
    Shader chunkShader_;
    TextureAtlas textureAtlas_;
    ChunkPipeline *pipeline_;

    void processStateChanges();
    void processBatches();

    bool allNeighborsStateReady(const ChunkCoord &coord, ChunkState state);

    static inline bool isInRenderDistance(int chunkX, int chunkZ, int playerX, int playerZ)
    {
        const int renderDistance = Constants::RENDER_DISTANCE;
        const int dx = chunkX - playerX;
        const int dz = chunkZ - playerZ;
        return dx * dx + dz * dz <= renderDistance * renderDistance;
    }
};
