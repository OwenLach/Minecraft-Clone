#pragma once

#include "Block.h"
#include "Constants.h"
#include "TextureAtlas.h"
#include "Shader.h"
#include "Chunk/ChunkCoord.h"
#include "Chunk/ChunkStateMachine.h"
#include "Chunk/Vertex.h"
#include "FastNoiseLite.h"
#include "TerrainGenerator.h"
#include "OpenGL/VertexArray.h"
#include "OpenGL/VertexBuffer.h"
#include "OpenGL/ElementBuffer.h"
#include "OpenGL/VertexBufferLayout.h"

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
    Chunk(Shader &shader, TextureAtlas &atlas, ChunkCoord pos, ChunkManager &chunkManager, ChunkPipeline &pipeline);

    // --- Core Methods ---
    void render();
    void generateTerrain();
    void generateMesh(std::array<std::shared_ptr<Chunk>, 4> neighborChunks);
    void uploadMeshToGPU();
    void removeBlockAt(glm::ivec3 pos);

    // State
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

    std::vector<unsigned int> indices_;
    std::vector<Vertex> vertices_;
    size_t vertexCount_ = 0;
    int indexCount_ = 0;

    ChunkStateMachine stateMachine_;
    ChunkCoord chunkCoord_;
    BoundingBox boundingBox_;
    TerrainGenerator terrainGen_;

    glm::mat4 modelMatrix_;

    // ------ References
    Shader &shader_;
    TextureAtlas &textureAtlas_;
    ChunkManager &chunkManager_;
    ChunkPipeline &pipeline_;
    VertexArray vao_;
    VertexBuffer vbo_;
    ElementBuffer ebo_;

    void generateBlockMesh(const Block &block, const std::array<std::shared_ptr<Chunk>, 4> neighborChunks);
    void generateFaceVertices(const Block &block, BlockFaces face, const std::function<BlockType(int, int, int)> &getCachedNeighbor);

    BlockType getNeighborBlockType(const glm::ivec3 blockPos, const glm::ivec3 offset, std::array<std::shared_ptr<Chunk>, 4> neighborChunks);
    inline bool blockInChunkBounds(const glm::ivec3 &pos) const;
    inline bool isTransparent(BlockType type) const;
    bool isBlockHidden(const glm::ivec3 &pos);
    inline size_t getBlockIndex(const glm::ivec3 &pos) const;
};