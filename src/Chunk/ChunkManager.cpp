#include "Chunk/ChunkManager.h"
#include "Chunk/Chunk.h"
#include "Chunk/ChunkPipeline.h"
#include "Shader.h"

#include <algorithm>

ChunkManager::ChunkManager(Camera &camera)
    : camera_(camera),
      chunkShader_("../shaders/chunk.vert", "../shaders/chunk.frag"),
      textureAtlas_()
{
}

void ChunkManager::init(ChunkPipeline *pipeline)
{
    pipeline_ = pipeline;
}

void ChunkManager::addChunk(const ChunkCoord &coord)
{
    auto chunk = std::make_shared<Chunk>(chunkShader_, textureAtlas_, coord);
    chunks_[coord] = chunk;
    readyForTerrainGen_.insert(chunk);
}

void ChunkManager::removeChunk(const ChunkCoord &coord)
{
    chunks_.erase(coord);
}

void ChunkManager::update()
{
    processBatches();
    processStateChanges();
}

void ChunkManager::processBatches()
{
    auto terrainBatch = std::move(readyForTerrainGen_);
    auto initLightBatch = std::move(readyForInitLighting_);
    auto finalLightBatch = std::move(readyForFinalLighting_);
    auto meshBatch = std::move(readyForMeshing_);
    auto uploadBatch = std::move(readyForUpload_);

    for (const auto &chunk : terrainBatch)
    {
        pipeline_->generateTerrain(chunk);
    }

    for (const auto &chunk : initLightBatch)
    {
        pipeline_->seedInitialLight(chunk);
    }

    for (const auto &chunk : finalLightBatch)
    {
        if (allNeighborsStateReady(chunk->getCoord(), ChunkState::INITIAL_LIGHT_READY))
        {
            pipeline_->propogateLight(chunk);
        }
        else
        {
            readyForFinalLighting_.insert(chunk);
        }
    }

    for (const auto &chunk : meshBatch)
    {
        if (allNeighborsStateReady(chunk->getCoord(), ChunkState::FINAL_LIGHT_READY))
        {
            pipeline_->generateMesh(chunk);
        }
        else
        {
            readyForMeshing_.insert(chunk);
        }
    }

    for (const auto &chunk : uploadBatch)
    {
        pipeline_->uploadMeshToGPU(chunk);
    }
}

void ChunkManager::processStateChanges()
{
    while (!stateChangeQueue_.empty())
    {
        auto event = stateChangeQueue_.front();
        stateChangeQueue_.pop();

        switch (event.newState)
        {
        case ChunkState::TERRAIN_GENERATED:
            event.chunk->setState(ChunkState::TERRAIN_GENERATED);
            readyForInitLighting_.insert(event.chunk);
            break;

        case ChunkState::INITIAL_LIGHT_READY:
            event.chunk->setState(ChunkState::INITIAL_LIGHT_READY);
            notifyDependentNeighbors(event.chunk, ChunkState::INITIAL_LIGHT_READY);
            readyForFinalLighting_.insert(event.chunk);
            break;

        case ChunkState::FINAL_LIGHT_READY:
            event.chunk->setState(ChunkState::FINAL_LIGHT_READY);
            notifyDependentNeighbors(event.chunk, ChunkState::FINAL_LIGHT_READY);
            readyForMeshing_.insert(event.chunk);
            break;

        case ChunkState::MESH_READY:
            event.chunk->setState(ChunkState::MESH_READY);
            readyForUpload_.insert(event.chunk);
            break;

        case ChunkState::LOADED:
            event.chunk->setState(ChunkState::LOADED);
            break;

        // TODO
        case ChunkState::NEEDS_LIGHT_UPDATE:
            break;

        // TODO
        case ChunkState::NEEDS_MESH_REGEN:
            break;
        }
    }
}

void ChunkManager::notifyStateChange(StateChangeEvent event)
{
    stateChangeQueue_.push(event);
}

void ChunkManager::notifyDependentNeighbors(std::shared_ptr<Chunk> chunk, ChunkState newState)
{
    auto neighbors = getChunkNeighbors(chunk->getCoord());
    for (const auto n : neighbors)
    {
        if (!n)
            continue;

        if (newState == ChunkState::INITIAL_LIGHT_READY && n->getState() >= ChunkState::INITIAL_LIGHT_READY)
        {
            readyForFinalLighting_.insert(n);
        }
        else if (newState == ChunkState::FINAL_LIGHT_READY && n->getState() >= ChunkState::FINAL_LIGHT_READY)
        {
            readyForMeshing_.insert(n);
        }
    }
}

void ChunkManager::renderAllChunks()
{
    textureAtlas_.bindUnit(0);
    chunkShader_.use();
    chunkShader_.setMat4("projection", camera_.getProjectionMatrix());
    chunkShader_.setMat4("view", camera_.getViewMatrix());

    for (auto &[pos, chunk] : chunks_)
    {
        if (camera_.isAABBInFrustum(chunk->getBoundingBox()))
            renderChunk(chunk, pos);
    }
}

void ChunkManager::renderChunk(std::shared_ptr<Chunk> chunk, const ChunkCoord &pos)
{
    ChunkMesh &mesh = chunk->getMesh();
    if (chunk->getState() == ChunkState::LOADED && mesh.hasValidMesh_)
        mesh.render(pos);
}

const TextureAtlas &ChunkManager::getTextureAtlasRef() const
{
    return textureAtlas_;
}

const std::shared_ptr<Chunk> ChunkManager::getChunk(const ChunkCoord &coord) const
{
    auto it = chunks_.find(coord);
    return (it != chunks_.end()) ? it->second : nullptr;
}

std::array<std::shared_ptr<Chunk>, 4> ChunkManager::getChunkNeighbors(const ChunkCoord &coord)
{
    const std::array<ChunkCoord, 4> neighborCoords = {{
        {coord.x, coord.z + 1}, // North
        {coord.x, coord.z - 1}, // South
        {coord.x + 1, coord.z}, // East
        {coord.x - 1, coord.z}  // West
    }};

    std::array<std::shared_ptr<Chunk>, 4> neighbors;
    {
        for (int i = 0; i < 4; i++)
        {
            neighbors[i] = getChunk(neighborCoords[i]);
        }
    }

    return neighbors;
}

bool ChunkManager::allNeighborsStateReady(const ChunkCoord &coord, ChunkState state)
{
    auto neighbors = getChunkNeighbors(coord);
    for (const auto &n_chunkPtr : neighbors)
    {
        // Only if the chunk exists and is less than the state return false
        // Chunks that do not exist don't affect this
        if (n_chunkPtr && n_chunkPtr->getState() < state)
        {
            return false;
        }
    }
    return true;
}
