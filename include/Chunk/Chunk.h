#pragma once

#include "Chunk/ChunkCoord.h"
#include "Chunk/ChunkStateMachine.h"
#include "Chunk/ChunkMesh.h"
#include "Chunk/Vertex.h"
#include "Block/Block.h"
#include "Constants.h"
#include "TerrainGenerator.h"

#include <vector>
#include <memory>

#include <glm/glm.hpp>

class ChunkManager;
class ChunkPipeline;
class MeshData;
class Shader;
class TextureAtlas;

struct BoundingBox
{
    glm::vec3 min;
    glm::vec3 max;
};

class Chunk : public std::enable_shared_from_this<Chunk>
{

public:
    Chunk(Shader &shader, TextureAtlas &atlas, ChunkCoord pos);

    // Operate on own block data
    void generateTerrain();
    void removeBlockAt(glm::ivec3 pos);
    void setBlockAt(glm::ivec3 pos, BlockType type);

    // Getters/Setters
    std::vector<Block> &getBlocks();
    ChunkMesh &getMesh();
    void setMeshData(MeshData &newMeshData);
    const ChunkCoord getCoord() const;
    const BoundingBox getBoundingBox() const;
    const TextureAtlas &getTextureAtlasRef() const;
    Block *getBlockLocal(const glm::ivec3 &pos);

    // State
    ChunkState getState() const;
    void setState(ChunkState newState);
    bool canUnload() const;
    bool isProcessing();
    bool canRemesh();

    static inline size_t getBlockIndex(const glm::ivec3 &pos) { return pos.x + (pos.y * Constants::CHUNK_SIZE_X) + (pos.z * Constants::CHUNK_SIZE_X * Constants::CHUNK_SIZE_Y); }
    static inline bool blockPosInChunkBounds(const glm::ivec3 &pos)
    {
        return pos.x >= 0 && pos.x < Constants::CHUNK_SIZE_X &&
               pos.y >= 0 && pos.y < Constants::CHUNK_SIZE_Y &&
               pos.z >= 0 && pos.z < Constants::CHUNK_SIZE_Z;
    }

private:
    // ---- Core Data ------
    std::vector<Block> blocks_;
    ChunkMesh mesh_;
    ChunkStateMachine stateMachine_;
    ChunkCoord chunkCoord_;
    BoundingBox boundingBox_;
    TerrainGenerator terrainGen_;
    TextureAtlas &textureAtlas_;
};