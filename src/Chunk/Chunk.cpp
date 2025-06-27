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

void Chunk::generateMesh()
{
    if (!isDirty_)
        return;

    // clear previous mesh
    meshDataBuffer_.clear();

    // loop through chunk and generate each blocks mesh
    for (int x = 0; x < Constants::CHUNK_SIZE_X; x++)
    {
        for (int y = 0; y < Constants::CHUNK_SIZE_Y; y++)
        {
            for (int z = 0; z < Constants::CHUNK_SIZE_Z; z++)
            {
                const Block &block = getBlockLocal(glm::ivec3(x, y, z));

                if (block.type != BlockType::Air)
                    generateBlockMesh(block);
            }
        }
    }
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

                // // Cave generation
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

ChunkState Chunk::getState() const
{
    return stateMachine_.getState();
}

void Chunk::setState(ChunkState newState)
{
    stateMachine_.setState(newState);
}

const ChunkCoord Chunk::getCoord() const
{
    return chunkCoord_;
}

const BoundingBox Chunk::getBoundingBox() const
{
    return boundingBox_;
}

void Chunk::generateBlockMesh(const Block &block)
{

    if (block.type == BlockType::Air)
        return;

    // render faces if face doesn't have anything in front of it
    for (int faceIdx = 0; faceIdx < 6; faceIdx++)
    {
        const BlockFaces face = static_cast<BlockFaces>(faceIdx);
        const glm::ivec3 offset = FACE_OFFSETS[faceIdx];

        BlockType neighborType = getNeighborBlockType(block.position, offset);

        if (isTransparent(neighborType))
        {
            addBlockFace(block, block.type, face);
        }
    }
}

void Chunk::generateFacevertices(const Block &block, BlockFaces face, const std::vector<glm::vec2> &faceUVs)
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
    auto computeAO = [](bool side1, bool side2, bool corner)
    {
        if (side1 && side2)
        {
            return 0;
        }
        return 3 - (side1 + side2 + corner);
    };

    std::vector<float> vertices;
    vertices.reserve(6 * (3 + 2 + 1)); // 6 vertices, each with 3 position + 2 UV + AO

    // loop through quad indicies
    for (size_t i = 0; i < 6; ++i)
    {
        const int cornerIdx = quadIndices[i];

        // example corners for FRONT face: { Vec3(-0.5f, -0.5f, 0.5f), Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f) }
        //               current corner vector + block position
        const Vec3 &pos = corners[cornerIdx] + glm::vec3(block.position);
        const Vec2 &uv = faceUVs[i];

        // get the offsets for the corner
        const AOTriplet &offsets = aoData[cornerIdx];

        // get the ao using curr block position + ao offsets
        // 0 = darkest, 3 = brightest
        glm::ivec3 blockWorldCoords = chunkManager_->chunkToWorldCoords(chunkCoord_, block.position);

        bool side1 = chunkManager_->isBlockSolid(blockWorldCoords + offsets[0]);
        bool side2 = chunkManager_->isBlockSolid(blockWorldCoords + offsets[1]);
        bool corner = chunkManager_->isBlockSolid(blockWorldCoords + offsets[2]);
        int aoValue = computeAO(side1, side2, corner);

        // get aoValue in range[0 - 1]
        float ao;
        switch (aoValue)
        {
        case 0:
            ao = 0.3f;
            break; // Darkest - both sides blocked
        case 1:
            ao = 0.5f;
            break; // Dark - two neighbors blocked
        case 2:
            ao = 0.7f;
            break; // Medium - one neighbor blocked
        case 3:
            ao = 1.0f;
            break; // Bright - no neighbors blocked
        default:
            ao = 1.0f;
            break;
        }

        vertices.insert(vertices.end(), {pos.x, pos.y, pos.z, uv.x, uv.y, ao});
    }

    meshDataBuffer_.insert(meshDataBuffer_.end(), vertices.begin(), vertices.end());
    // return vertices;
}

void Chunk::addBlockFace(const Block &block, const BlockType type, const BlockFaces face)
{
    const std::vector<glm::vec2> faceUVs = textureAtlas_->getFaceUVs(type, face);
    generateFacevertices(block, face, faceUVs);
}

BlockType Chunk::getNeighborBlockType(const glm::ivec3 blockPos, const glm::ivec3 offset)
{
    const glm::ivec3 neighborLocalPos = blockPos + offset;

    // if it's in the chunk, just get it
    if (blockInChunkBounds(neighborLocalPos))
    {
        return getBlockLocal(neighborLocalPos).type;
    }
    // get it using chunkManager_ coords
    else
    {
        // get blocks chunkManager_ positoins
        glm::ivec3 blockWorldPos = chunkManager_->chunkToWorldCoords(chunkCoord_, blockPos);
        // get neighbors chunkManager_ pos using blocks position
        glm::ivec3 neighborWorldPos = blockWorldPos + offset;
        // get type of neighbor
        return chunkManager_->getBlockGlobal(neighborWorldPos).type;
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