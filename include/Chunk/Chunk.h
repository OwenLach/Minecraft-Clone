#pragma once

#include "Block.h"
#include "Constants.h"
#include "TextureAtlas.h"
#include "Shader.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "Chunk/ChunkCoord.h"
#include "Chunk/ChunkStateMachine.h"
#include "FastNoiseLite.h"
#include "TerrainGenerator.h"

#include <vector>
#include <memory>
#include <atomic>
#include <functional>

#include <glm/glm.hpp>

class ChunkManager;
class ChunkPipeline;

struct BoundingBox
{
    glm::vec3 min;
    glm::vec3 max;
};

class Chunk : public std::enable_shared_from_this<Chunk>
{

public:
    std::atomic<bool> isDirty_ = false;

    Chunk(Shader &shader, TextureAtlas &atlas, ChunkCoord pos, ChunkManager &chunkManager, ChunkPipeline &pipeline);

    // --- Core Methods ---
    void render();
    void generateTerrain();
    void generateMesh(std::array<std::shared_ptr<Chunk>, 4> neighborChunks);
    void uploadMeshToGPU();

    // --- Reactive System ---
    // Called when a neighbor's terrain becomes ready.
    void onNeighborReady();
    // Checks neighbors after its own terrain is ready.
    void checkAndNotifyNeighbors();

    // State
    void setDirty();
    ChunkState getState() const;
    void setState(ChunkState newState);
    bool canUnload() const;
    bool isProcessing();
    bool canRemesh();

    const ChunkCoord getCoord() const;
    const BoundingBox getBoundingBox() const;
    const Block &getBlockLocal(const glm::ivec3 &pos) const;

    void configureVertexAttributes();

private:
    // ---- Core Data ------
    std::vector<Block> blocks_;
    std::vector<float> meshDataBuffer_;

    ChunkStateMachine stateMachine_;
    ChunkCoord chunkCoord_;
    BoundingBox boundingBox_;
    TerrainGenerator terrainGen_;

    std::atomic<int> neighborsReadyCount_{0};

    glm::mat4 modelMatrix_;
    size_t vertexCount_ = 0;

    // ------ References
    Shader &shader_;
    TextureAtlas &textureAtlas_;
    ChunkManager &chunkManager_;
    ChunkPipeline &pipeline_;
    VertexArray vao_;
    VertexBuffer vbo_;

    void generateBlockMesh(const Block &block, const std::array<std::shared_ptr<Chunk>, 4> neighborChunks);
    void generateFacevertices(const Block &block, BlockFaces face, const std::vector<glm::vec2> &faceUVs, const std::function<BlockType(int, int, int)> &getCachedNeighbor);
    void addBlockFace(const Block &block, const BlockFaces face, const std::function<BlockType(int, int, int)> &getCachedNeighbor);

    BlockType getNeighborBlockType(const glm::ivec3 blockPos, const glm::ivec3 offset, std::array<std::shared_ptr<Chunk>, 4> neighborChunks);
    inline bool blockInChunkBounds(const glm::ivec3 &pos) const;
    inline bool isTransparent(BlockType type) const;
    bool isBlockHidden(const glm::ivec3 &pos);
    inline size_t getBlockIndex(const glm::ivec3 &pos) const;
};