#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <array>

#include "Chunk.h"
#include "BlockTypes.h"
#include "World.h"
#include "FastNoiseLite.h"

static constexpr std::array<glm::ivec3, 6> FACE_OFFSETS = {{
    {1, 0, 0},  // Right
    {-1, 0, 0}, // Left
    {0, 1, 0},  // Top
    {0, -1, 0}, // Bottom
    {0, 0, 1},  // Front
    {0, 0, -1}  // Back
}};

Chunk::Chunk(Shader &shader, TextureAtlas *atlas, ChunkCoord pos, World *world)
    : shader(shader), textureAtlas(atlas), worldPos(pos), world(world)
{
    // reserve for worse case
    meshDataBuffer.reserve(Constants::CHUNK_SIZE_X * Constants::CHUNK_SIZE_Y * Constants::CHUNK_SIZE_Z * 36);
    blocks.resize(Constants::CHUNK_SIZE_X * Constants::CHUNK_SIZE_Y * Constants::CHUNK_SIZE_Z);
}

void Chunk::renderChunk()
{
    // make sure chunk is loaded
    if (state != ChunkState::LOADED)
    {
        return;
    }

    shader.use();
    vao.bind();

    glm::mat4 model = glm::mat4(1.0f);
    modelMatrix = glm::translate(model, glm::vec3(worldPos.x * Constants::CHUNK_SIZE_X,
                                                  -Constants::CHUNK_SIZE_Y,
                                                  worldPos.z * Constants::CHUNK_SIZE_Z));

    shader.setMat4("model", modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

void Chunk::uploadMeshToGPU()
{
    if (meshDataBuffer.empty())
    {
        vertexCount = 0;
        isDirty = false;
        return;
    }

    vbo.setData(meshDataBuffer.data(), meshDataBuffer.size() * sizeof(float));
    vertexCount = meshDataBuffer.size() / 5; // five float per vertex

    configureVertexAttributes();
    meshDataBuffer.clear();
    isDirty = false;
}

void Chunk::configureVertexAttributes()
{
    vao.bind();
    VertexBufferLayout layout;
    layout.push<float>(3); // position
    layout.push<float>(2); // texture coords
    vao.addBuffer(vbo, layout);
}

void Chunk::setDirty()
{
    isDirty = true;
    // modelMatrixDirty = true;
}

void Chunk::generateMesh()
{
    // clear previous mesh
    meshDataBuffer.clear();

    // loop through chunk and generate each blocks mesh
    for (int x = 0; x < Constants::CHUNK_SIZE_X; x++)
    {
        for (int y = 0; y < Constants::CHUNK_SIZE_Y; y++)
        {
            for (int z = 0; z < Constants::CHUNK_SIZE_Z; z++)
            {
                const Block &block = getBlockAt(glm::ivec3(x, y, z));

                if (block.type != BlockType::Air)
                    generateBlockMesh(block);
            }
        }
    }
}

void Chunk::generateTerrain()
{
    FastNoiseLite noise;
    noise.SetFrequency(0.01f);
    noise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);

    for (int x = 0; x < Constants::CHUNK_SIZE_X; x++)
    {
        for (int z = 0; z < Constants::CHUNK_SIZE_Z; z++)
        {
            for (int y = 0; y < Constants::CHUNK_SIZE_Y; y++)
            {

                float worldX = worldPos.x * Constants::CHUNK_SIZE_X + x;
                float worldZ = worldPos.z * Constants::CHUNK_SIZE_Z + z;

                float noiseVal = noise.GetNoise(worldX, worldZ);
                int height = Constants::TERRAIN_BASE_HEIGHT + (int)(noiseVal * Constants::TERRAIN_HEIGHT_VARIATION);

                BlockType type;
                if (y < height - 5)
                {
                    type = BlockType::Stone;
                }
                else if (y < height)
                {
                    type = BlockType::Dirt;
                }
                else if (y == height)
                {
                    type = BlockType::Grass;
                }
                else
                {
                    type = BlockType::Air;
                }

                glm::ivec3 pos = {x, y, z};
                const size_t index = getBlockIndex(pos);
                blocks[index] = Block(type, pos);
            }
        }
    }
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

std::vector<float> Chunk::generateFacevertices(const Block &block, BlockFaces face, const std::vector<glm::vec2> &faceUVs) const
{
    using Vec3 = glm::vec3;
    using Vec2 = glm::vec2;

    static const std::unordered_map<BlockFaces, std::array<Vec3, 4>> faceCorners = {
        {BlockFaces::Front, {Vec3(-0.5f, -0.5f, 0.5f), Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f)}},
        {BlockFaces::Back, {Vec3(0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f)}},
        {BlockFaces::Left, {Vec3(-0.5f, -0.5f, -0.5f), Vec3(-0.5f, -0.5f, 0.5f), Vec3(-0.5f, 0.5f, 0.5f), Vec3(-0.5f, 0.5f, -0.5f)}},
        {BlockFaces::Right, {Vec3(0.5f, -0.5f, 0.5f), Vec3(0.5f, -0.5f, -0.5f), Vec3(0.5f, 0.5f, -0.5f), Vec3(0.5f, 0.5f, 0.5f)}},
        {BlockFaces::Top, {Vec3(-0.5f, 0.5f, 0.5f), Vec3(0.5f, 0.5f, 0.5f), Vec3(0.5f, 0.5f, -0.5f), Vec3(-0.5f, 0.5f, -0.5f)}},
        {BlockFaces::Bottom, {Vec3(-0.5f, -0.5f, -0.5f), Vec3(0.5f, -0.5f, -0.5f), Vec3(0.5f, -0.5f, 0.5f), Vec3(-0.5f, -0.5f, 0.5f)}},
    };

    static const std::array<unsigned int, 6> quadIndices = {0, 1, 2, 2, 3, 0};

    // Error if invalid UV count
    if (faceUVs.size() != 6)
        throw std::runtime_error("faceUVs must contain exactly 6 elements (6 vertices)");

    const auto &corners = faceCorners.at(face);

    std::vector<float> vertices;
    vertices.reserve(6 * (3 + 2)); // 6 vertices, each with 3 position + 2 UV

    for (size_t i = 0; i < 6; ++i)
    {
        const Vec3 &pos = corners[quadIndices[i]] + glm::vec3(block.position);
        const Vec2 &uv = faceUVs[i];

        vertices.insert(vertices.end(), {pos.x, pos.y, pos.z, uv.x, uv.y});
    }

    return vertices;
}

void Chunk::addBlockFace(const Block &block, const BlockType type, const BlockFaces face)
{
    const std::vector<glm::vec2> faceUVs = textureAtlas->getFaceUVs(type, face);
    const std::vector<float> rawVertices = generateFacevertices(block, face, faceUVs);

    meshDataBuffer.insert(meshDataBuffer.end(), rawVertices.begin(), rawVertices.end());
}

BlockType Chunk::getNeighborBlockType(const glm::ivec3 blockPos, const glm::ivec3 offset)
{
    const glm::ivec3 neighborLocalPos = blockPos + offset;

    // if it's in the chunk, just get it
    if (blockInChunkBounds(neighborLocalPos))
    {
        return getBlockAt(neighborLocalPos).type;
    }
    // get it using world coords
    else
    {
        // get blocks world positoins
        glm::ivec3 blockWorldPos = world->chunkToWorldCoords(worldPos, blockPos);
        // get neighbors world pos using blocks position
        glm::ivec3 neighborWorldPos = blockWorldPos + offset;
        // get type of neighbor
        return world->getBlockAt(neighborWorldPos).type;
    }
}

const Block &Chunk::getBlockAt(const glm::ivec3 &pos) const
{
    if (!blockInChunkBounds(pos))
    {
        // Out of bounds — return a static air block
        static Block airBlock(BlockType::Air, glm::vec3(0));
        return airBlock;
    }

    const size_t index = getBlockIndex(pos);
    return blocks[index];
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