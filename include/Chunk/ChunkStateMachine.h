#pragma once

#include <atomic>

enum class ChunkState
{

    // Main chunk pipeline
    EMPTY,               // No data
    TERRAIN_GENERATED,   // All blocks and the inital skylight values are ready
    INITIAL_LIGHT_READY, // Initial light has been seeded within chunk
    FINAL_LIGHT_READY,   // Light values are corretly set including values from neighboring chunks
    MESH_READY,          // OpenGL data finshed generating
    LOADED,              // Chunk is fully rendered and idle
    NEEDS_LIGHT_UPDATE,  // Lighting invalidated, needs recalculation
    NEEDS_MESH_REGEN     // Mesh invalidated, needs rebuilding
};

class ChunkStateMachine
{
private:
    std::atomic<ChunkState> currentState_{ChunkState::EMPTY};

public:
    void setState(ChunkState state);
    ChunkState getState() const;
    bool canTransitionTo(ChunkState newState) const;
    static const char *toString(ChunkState state);
};