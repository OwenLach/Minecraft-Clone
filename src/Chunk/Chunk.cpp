#include "Chunk/Chunk.h"
#include "Chunk/ChunkManager.h"
#include "BlockTypes.h"
#include "FastNoiseLite.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <array>
#include <vector>
#include <cmath>
#include <functional>

static constexpr std::array<glm::ivec3, 6> FACE_OFFSETS = {{
    {1, 0, 0},  // Right
    {-1, 0, 0}, // Left
    {0, 1, 0},  // Top
    {0, -1, 0}, // Bottom
    {0, 0, 1},  // Front
    {0, 0, -1}  // Back
}};

Chunk::Chunk(Shader &shader, TextureAtlas *atlas, ChunkCoord pos, ChunkManager *chunkManager)
    : shader_(shader), textureAtlas_(atlas), chunkCoord_(pos), chunkManager_(chunkManager)
{
    const int chunkSize_X = Constants::CHUNK_SIZE_X;
    const int chunkSize_Y = Constants::CHUNK_SIZE_Y;
    const int chunkSize_Z = Constants::CHUNK_SIZE_Z;
    // reserve for worse case
    meshDataBuffer_.reserve(chunkSize_X * chunkSize_Y * chunkSize_Z * 36);
    blocks_.resize(chunkSize_X * chunkSize_Y * chunkSize_Z);

    boundingBox_.min = glm::vec3(chunkCoord_.x * chunkSize_X, -chunkSize_Y, chunkCoord_.z * chunkSize_Z);
    boundingBox_.max = glm::vec3(chunkCoord_.x * chunkSize_X + chunkSize_X, 0, chunkCoord_.z * chunkSize_Z + chunkSize_Z);
}

void Chunk::render()
{
    // make sure chunk is loaded
    if (!stateMachine_.isReady())
    {
        return;
    }

    shader_.use();
    vao_.bind();

    glm::mat4 model = glm::mat4(1.0f);
    modelMatrix_ = glm::translate(model, glm::vec3(chunkCoord_.x * Constants::CHUNK_SIZE_X,
                                                   -Constants::CHUNK_SIZE_Y,
                                                   chunkCoord_.z * Constants::CHUNK_SIZE_Z));

    shader_.setMat4("model", modelMatrix_);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount_);
}

void Chunk::uploadMeshToGPU()
{
    if (meshDataBuffer_.empty())
    {
        vertexCount_ = 0;
        isDirty_ = false;
        return;
    }

    vbo_.setData(meshDataBuffer_.data(), meshDataBuffer_.size() * sizeof(float));
    vertexCount_ = meshDataBuffer_.size() / 5; // five float per vertex

    configureVertexAttributes();
    meshDataBuffer_.clear();
    isDirty_ = false;
}

void Chunk::configureVertexAttributes()
{
    vao_.bind();
    VertexBufferLayout layout;
    layout.push<float>(3); // position
    layout.push<float>(2); // texture coords
    layout.push<float>(1); // AO
    vao_.addBuffer(vbo_, layout);
}

void Chunk::setDirty()
{
    isDirty_ = true;
}

void Chunk::generateMesh(std::array<std::shared_ptr<Chunk>, 4> neighborChunks)
{
    const size_t estimatedFaces = (Constants::CHUNK_SIZE_X * Constants::CHUNK_SIZE_Y * Constants::CHUNK_SIZE_Z) / 4;
    meshDataBuffer_.clear();
    meshDataBuffer_.reserve(estimatedFaces * 30); // 6 vertices * 5 floats per face

    // loop through chunk and generate each blocks mesh
    for (int x = 0; x < Constants::CHUNK_SIZE_X; x++)
    {
        for (int y = 0; y < Constants::CHUNK_SIZE_Y; y++)
        {
            for (int z = 0; z < Constants::CHUNK_SIZE_Z; z++)
            {
                const size_t index = getBlockIndex(glm::ivec3(x, y, z));
                const Block &block = blocks_[index];

                if (block.type != BlockType::Air)
                    generateBlockMesh(block, neighborChunks);
            }
        }
    }

    meshDataBuffer_.shrink_to_fit();
}

void Chunk::generateTerrain()
{
    for (int x = 0; x < Constants::CHUNK_SIZE_X; x++)
    {
        for (int z = 0; z < Constants::CHUNK_SIZE_Z; z++)
        {
            for (int y = 0; y < Constants::CHUNK_SIZE_Y; y++)
            {

                float worldX = chunkCoord_.x * Constants::CHUNK_SIZE_X + x;
                float worldZ = chunkCoord_.z * Constants::CHUNK_SIZE_Z + z;

                float terrainNoise = terrainGen_.getTerrainNoise(worldX, worldZ); // [0 - 1]
                float centeredNoise = terrainNoise - 0.5f;                        // -0.5 to 0.5
                int height = Constants::TERRAIN_BASE_HEIGHT + (int)(centeredNoise * Constants::TERRAIN_HEIGHT_VARIATION * 2.0f);

                BlockType type;
                if (y > height)
                {
                    type = BlockType::Air;
                }
                else if (y == height)
                {
                    type = BlockType::Grass;
                }
                else if (y < height && y > Constants::STONE_LEVEL)
                {
                    type = BlockType::Dirt;
                }
                else
                {
                    type = BlockType::Stone;
                }

                // Cave generation
                // float caveVal = terrainGen_.getCaveNoise(worldX, (float)y, worldZ);
                // if (y < height - 5 &&
                //     caveVal > Constants::CAVE_THRESHOLD &&
                //     y > 0)
                // {
                //     type = BlockType::Air;
                // }

                glm::ivec3 pos = {x, y, z};
                const size_t index = getBlockIndex(pos);
                blocks_[index] = Block(type, pos);
            }
        }
    }
}

void Chunk::generateBlockMesh(const Block &block, std::array<std::shared_ptr<Chunk>, 4> neighborChunks)
{
    if (block.type == BlockType::Air)
        return;

    // Create a 3x3x3 cache of neighbor block types.
    std::array<BlockType, 27> neighborTypesCache;
    for (int i = 0; i < 27; i++)
    {
        glm::ivec3 offset = glm::ivec3(i % 3 - 1, (i / 3) % 3 - 1, i / 9 - 1);
        neighborTypesCache[i] = getNeighborBlockType(block.position, offset, neighborChunks);
    }

    // Helper to get neighbor from cache
    auto getNeighborTypeFromCache = [&](int x, int y, int z)
    {
        // Convert offset from [-1, 1] to [0, 2] for array indexing
        return neighborTypesCache[(x + 1) + (y + 1) * 3 + (z + 1) * 9];
    };

    // Render faces if face doesn't have anything in front of it
    for (int faceIdx = 0; faceIdx < 6; faceIdx++)
    {
        const BlockFaces face = static_cast<BlockFaces>(faceIdx);
        const glm::ivec3 offset = FACE_OFFSETS[faceIdx];

        BlockType neighborType = getNeighborTypeFromCache(offset.x, offset.y, offset.z);
        BlockType currType = getNeighborTypeFromCache(0, 0, 0);

        if (isTransparent(neighborType))
        {
            addBlockFace(block, face, getNeighborTypeFromCache);
        }
    }
}

void Chunk::addBlockFace(const Block &block, const BlockFaces face, const std::function<BlockType(int, int, int)> &getNeighborTypeFromCache)
{
    const std::vector<glm::vec2> faceUVs = textureAtlas_->getFaceUVs(block.type, face);
    generateFacevertices(block, face, faceUVs, getNeighborTypeFromCache);
}

void Chunk::generateFacevertices(const Block &block, BlockFaces face, const std::vector<glm::vec2> &faceUVs, const std::function<BlockType(int, int, int)> &getNeighborTypeFromCache)
{
    using Vec3 = glm::vec3;
    using Vec2 = glm::vec2;
    using IVec3 = glm::ivec3;
    using AOTriplet = std::array<IVec3, 3>;

    static const std::unordered_map<BlockFaces, std::array<Vec3, 4>> faceCorners = {
        {BlockFaces::Front, {Vec3(-0.5f, -0.5f, 0.5f), Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f)}},
        {BlockFaces::Back, {Vec3(0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f)}},
        {BlockFaces::Left, {Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(-0.5f, 0.5f, -0.5f)}},
        {BlockFaces::Right, {Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, -0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, 0.5f)}},
        {BlockFaces::Top, {Vec3(-0.5f, 0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Vec3(0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f)}},
        {BlockFaces::Bottom, {Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.5f, -0.5f, -0.5f), Vec3(0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f)}},
    };

    static const std::unordered_map<BlockFaces, std::array<AOTriplet, 4>> aoOffsets = {
        {BlockFaces::Front, {
                                // Bottom-left vertex
                                AOTriplet{IVec3(-1, 0, 1), IVec3(0, -1, 1), IVec3(-1, -1, 1)},
                                // Bottom-right vertex
                                AOTriplet{IVec3(1, 0, 1), IVec3(0, -1, 1), IVec3(1, -1, 1)},
                                // Top-right vertex
                                AOTriplet{IVec3(1, 0, 1), IVec3(0, 1, 1), IVec3(1, 1, 1)},
                                // Top-left vertex
                                AOTriplet{IVec3(-1, 0, 1), IVec3(0, 1, 1), IVec3(-1, 1, 1)},
                            }},
        {BlockFaces::Back, {
                               // Bottom-left vertex (from back face perspective)
                               AOTriplet{IVec3(1, 0, -1), IVec3(0, -1, -1), IVec3(1, -1, -1)},
                               // Bottom-right vertex
                               AOTriplet{IVec3(-1, 0, -1), IVec3(0, -1, -1), IVec3(-1, -1, -1)},
                               // Top-right vertex
                               AOTriplet{IVec3(-1, 0, -1), IVec3(0, 1, -1), IVec3(-1, 1, -1)},
                               // Top-left vertex
                               AOTriplet{IVec3(1, 0, -1), IVec3(0, 1, -1), IVec3(1, 1, -1)},
                           }},
        {BlockFaces::Left, {
                               // Bottom-left vertex
                               AOTriplet{IVec3(-1, 0, -1), IVec3(-1, -1, 0), IVec3(-1, -1, -1)},
                               // Bottom-right vertex
                               AOTriplet{IVec3(-1, 0, 1), IVec3(-1, -1, 0), IVec3(-1, -1, 1)},
                               // Top-right vertex
                               AOTriplet{IVec3(-1, 0, 1), IVec3(-1, 1, 0), IVec3(-1, 1, 1)},
                               // Top-left vertex
                               AOTriplet{IVec3(-1, 0, -1), IVec3(-1, 1, 0), IVec3(-1, 1, -1)},
                           }},
        {BlockFaces::Right, {
                                // Bottom-left vertex
                                AOTriplet{IVec3(1, 0, 1), IVec3(1, -1, 0), IVec3(1, -1, 1)},
                                // Bottom-right vertex
                                AOTriplet{IVec3(1, 0, -1), IVec3(1, -1, 0), IVec3(1, -1, -1)},
                                // Top-right vertex
                                AOTriplet{IVec3(1, 0, -1), IVec3(1, 1, 0), IVec3(1, 1, -1)},
                                // Top-left vertex
                                AOTriplet{IVec3(1, 0, 1), IVec3(1, 1, 0), IVec3(1, 1, 1)},
                            }},
        {BlockFaces::Top, {
                              // Bottom-left vertex (front-left from top view)
                              AOTriplet{IVec3(-1, 1, 0), IVec3(0, 1, 1), IVec3(-1, 1, 1)},
                              // Bottom-right vertex (front-right from top view)
                              AOTriplet{IVec3(1, 1, 0), IVec3(0, 1, 1), IVec3(1, 1, 1)},
                              // Top-right vertex (back-right from top view)
                              AOTriplet{IVec3(1, 1, 0), IVec3(0, 1, -1), IVec3(1, 1, -1)},
                              // Top-left vertex (back-left from top view)
                              AOTriplet{IVec3(-1, 1, 0), IVec3(0, 1, -1), IVec3(-1, 1, -1)},
                          }},
        {BlockFaces::Bottom, {
                                 // Bottom-left vertex (back-left from bottom view)
                                 AOTriplet{IVec3(-1, -1, 0), IVec3(0, -1, -1), IVec3(-1, -1, -1)},
                                 // Bottom-right vertex (back-right from bottom view)
                                 AOTriplet{IVec3(1, -1, 0), IVec3(0, -1, -1), IVec3(1, -1, -1)},
                                 // Top-right vertex (front-right from bottom view)
                                 AOTriplet{IVec3(1, -1, 0), IVec3(0, -1, 1), IVec3(1, -1, 1)},
                                 // Top-left vertex (front-left from bottom view)
                                 AOTriplet{IVec3(-1, -1, 0), IVec3(0, -1, 1), IVec3(-1, -1, 1)},
                             }},
    };

    static const std::array<unsigned int, 6> quadIndices = {0, 1, 2, 2, 3, 0};

    // Error if invalid UV count
    if (faceUVs.size() != 6)
        throw std::runtime_error("faceUVs must contain exactly 6 elements (6 vertices)");

    // get corners and ao offsets for the face
    const auto &corners = faceCorners.at(face);
    const auto &aoData = aoOffsets.at(face);

    // ao helper function
    auto computeAO = [&](const std::array<glm::ivec3, 3> &offsets)
    {
        bool side1 = !isTransparent(getNeighborTypeFromCache(offsets[0].x, offsets[0].y, offsets[0].z));
        bool side2 = !isTransparent(getNeighborTypeFromCache(offsets[1].x, offsets[1].y, offsets[1].z));
        bool corner = !isTransparent(getNeighborTypeFromCache(offsets[2].x, offsets[2].y, offsets[2].z));

        if (side1 && side2)
        {
            return 0.3f; // Darkest
        }
        int occlusion = side1 + side2 + corner;
        return 1.0f - occlusion * 0.2f; // Simple mapping: 1.0, 0.8, 0.6, 0.4
    };

    meshDataBuffer_.reserve(meshDataBuffer_.size() + 30); // Reserve for 6 vertices * 5 floats

    // loop through quad indicies
    for (size_t i = 0; i < 6; ++i)
    {
        const int cornerIdx = quadIndices[i];
        const Vec3 &pos = corners[cornerIdx] + glm::vec3(block.position);
        const Vec2 &uv = faceUVs[i];

        // get the offsets for the corner
        float ao = computeAO(aoData[cornerIdx]);

        meshDataBuffer_.push_back(pos.x);
        meshDataBuffer_.push_back(pos.y);
        meshDataBuffer_.push_back(pos.z);
        meshDataBuffer_.push_back(uv.x);
        meshDataBuffer_.push_back(uv.y);
        meshDataBuffer_.push_back(ao);
    }
}

BlockType Chunk::getNeighborBlockType(const glm::ivec3 blockPos, const glm::ivec3 offset, std::array<std::shared_ptr<Chunk>, 4> neighborChunks)
{
    const auto northChunk = neighborChunks[0];
    const auto southChunk = neighborChunks[1];
    const auto eastChunk = neighborChunks[2];
    const auto westChunk = neighborChunks[3];

    const glm::ivec3 neighborLocalPos = blockPos + offset;

    // if it's in the chunk, just get it
    if (blockInChunkBounds(neighborLocalPos))
    {
        return blocks_[getBlockIndex(neighborLocalPos)].type;
    }
    else
    {
        // Check which chunk block is in
        // Determine which neighboring chunk the block falls into and get its local coordinates

        if (neighborLocalPos.x < 0 &&
            neighborLocalPos.z >= 0 && neighborLocalPos.z < Constants::CHUNK_SIZE_Z &&
            neighborLocalPos.y >= 0 && neighborLocalPos.y < Constants::CHUNK_SIZE_Y)
        {
            if (westChunk)
            {
                glm::ivec3 localPosInNeighbor = {neighborLocalPos.x + Constants::CHUNK_SIZE_X, neighborLocalPos.y, neighborLocalPos.z};
                return westChunk->blocks_[getBlockIndex(localPosInNeighbor)].type;
            }
        }
        // Check East neighbor (+X direction)
        else if (neighborLocalPos.x >= Constants::CHUNK_SIZE_X &&
                 neighborLocalPos.z >= 0 && neighborLocalPos.z < Constants::CHUNK_SIZE_Z &&
                 neighborLocalPos.y >= 0 && neighborLocalPos.y < Constants::CHUNK_SIZE_Y)
        {
            if (eastChunk)
            {
                glm::ivec3 localPosInNeighbor = {neighborLocalPos.x - Constants::CHUNK_SIZE_X, neighborLocalPos.y, neighborLocalPos.z};
                return eastChunk->blocks_[getBlockIndex(localPosInNeighbor)].type;
            }
        }
        // Check South neighbor (-Z direction)
        else if (neighborLocalPos.z < 0 &&
                 neighborLocalPos.x >= 0 && neighborLocalPos.x < Constants::CHUNK_SIZE_X &&
                 neighborLocalPos.y >= 0 && neighborLocalPos.y < Constants::CHUNK_SIZE_Y)
        {
            if (southChunk)
            {
                glm::ivec3 localPosInNeighbor = {neighborLocalPos.x, neighborLocalPos.y, neighborLocalPos.z + Constants::CHUNK_SIZE_Z};
                return southChunk->blocks_[getBlockIndex(localPosInNeighbor)].type;
            }
        }
        // Check North neighbor (+Z direction)
        else if (neighborLocalPos.z >= Constants::CHUNK_SIZE_Z &&
                 neighborLocalPos.x >= 0 && neighborLocalPos.x < Constants::CHUNK_SIZE_X &&
                 neighborLocalPos.y >= 0 && neighborLocalPos.y < Constants::CHUNK_SIZE_Y)
        {
            if (northChunk)
            {
                glm::ivec3 localPosInNeighbor = {neighborLocalPos.x, neighborLocalPos.y, neighborLocalPos.z - Constants::CHUNK_SIZE_Z};
                return northChunk->blocks_[getBlockIndex(localPosInNeighbor)].type;
            }
        }

        // Fallback
        return BlockType::Air;
    }
}

const Block &Chunk::getBlockLocal(const glm::ivec3 &pos) const
{
    if (!blockInChunkBounds(pos))
    {
        // Out of bounds — return a static air block
        static Block airBlock(BlockType::Air, glm::vec3(0));
        return airBlock;
    }

    const size_t index = getBlockIndex(pos);
    return blocks_[index];
}

inline bool Chunk::isTransparent(BlockType type) const
{
    return type == BlockType::Air;
}

inline size_t Chunk::getBlockIndex(const glm::ivec3 &pos) const
{
    return pos.x + (pos.y * Constants::CHUNK_SIZE_X) + (pos.z * Constants::CHUNK_SIZE_X * Constants::CHUNK_SIZE_Y);
}

inline bool Chunk::blockInChunkBounds(const glm::ivec3 &pos) const
{
    return pos.x >= 0 && pos.x < Constants::CHUNK_SIZE_X &&
           pos.y >= 0 && pos.y < Constants::CHUNK_SIZE_Y &&
           pos.z >= 0 && pos.z < Constants::CHUNK_SIZE_Z;
}

ChunkState Chunk::getState() const
{
    return stateMachine_.getState();
}

void Chunk::setState(ChunkState newState)
{
    stateMachine_.setState(newState);
}

bool Chunk::canUnload() const
{
    return stateMachine_.canUnload();
}

bool Chunk::isProcessing()
{
    return stateMachine_.isProcessing();
}

bool Chunk::canRemesh()
{
    return stateMachine_.canRemesh();
}
const ChunkCoord Chunk::getCoord() const
{
    return chunkCoord_;
}

const BoundingBox Chunk::getBoundingBox() const
{
    return boundingBox_;
}
