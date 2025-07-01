#pragma once

#include <vector>
#include <memory>
#include <atomic>
#include <glm/glm.hpp>

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

#include <functional>

class ChunkManager;

struct Vertex
{
    glm::vec3 pos;
    glm::vec2 uvs;
};

struct BoundingBox
{
    glm::vec3 min;
    glm::vec3 max;
};

class Chunk
{

public:
    std::atomic<bool> isDirty_ = false;

    Chunk(Shader &shader, TextureAtlas *atlas, ChunkCoord pos, ChunkManager *chunkManager);

    void render();
    void generateMesh(std::array<std::shared_ptr<Chunk>, 4> neighborChunks);
    void uploadMeshToGPU();
    void configureVertexAttributes();
    void setDirty();
    void generateTerrain();

    // State
    ChunkState getState() const;
    void setState(ChunkState newState);
    bool canUnload() const;
    bool isProcessing();
    bool canRemesh();

    const ChunkCoord getCoord() const;
    const BoundingBox getBoundingBox() const;

    const Block &getBlockLocal(const glm::ivec3 &pos) const;

private:
    ChunkStateMachine stateMachine_;
    ChunkCoord chunkCoord_;
    BoundingBox boundingBox_;

    std::vector<Block> blocks_;
    std::vector<float> meshDataBuffer_;

    glm::mat4 modelMatrix_;
    size_t vertexCount_ = 0;

    VertexArray vao_;
    VertexBuffer vbo_;
    Shader &shader_;
    TextureAtlas *textureAtlas_;
    ChunkManager *chunkManager_;
    TerrainGenerator terrainGen_;

    void generateBlockMesh(const Block &block, std::array<std::shared_ptr<Chunk>, 4> neighborChunks);
    void generateFacevertices(const Block &block, BlockFaces face, const std::vector<glm::vec2> &faceUVs, const std::function<BlockType(int, int, int)> &getCachedNeighbor);
    void addBlockFace(const Block &block, const BlockFaces face, const std::function<BlockType(int, int, int)> &getCachedNeighbor);

    BlockType getNeighborBlockType(const glm::ivec3 blockPos, const glm::ivec3 offset, std::array<std::shared_ptr<Chunk>, 4> neighborChunks);
    inline bool blockInChunkBounds(const glm::ivec3 &pos) const;
    inline bool isTransparent(BlockType type) const;
    inline size_t getBlockIndex(const glm::ivec3 &pos) const;
};