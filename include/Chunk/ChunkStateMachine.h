#pragma once

#include <atomic>

enum class ChunkState
{

    // Main chunk pipeline
    EMPTY,              // No data
    TERRAIN_GENERATING, // Generating all the blocks and only setting top-down skylight for only the chunk, no light propogation yet
    TERRAIN_READY,      // All blocks and the inital skylight values are ready
    LIGHT_PROPOGATING,  // Get lights from neighbors and does the light propogation
    LIGHT_READY,        // Light values are corretly set including values from neighboring chunks
    MESH_GENERATING,    // OpenGl data (verticies and indices) are being generated for the chunk
    MESH_READY,         // OpenGL data finshed generating
    LOADED,             // Chunk is fully rendered and idle
    UNLOADING,          // Chunk out of render distance and is being unloaded

    // Update states
    NEEDS_LIGHT_UPDATE, // Lighting invalidated, needs recalculation
    NEEDS_MESH_REGEN    // Mesh invalidated, needs rebuilding
};

class ChunkStateMachine
{
private:
    std::atomic<ChunkState> currentState_{ChunkState::EMPTY};

public:
    void setState(ChunkState state);
    ChunkState getState() const;
    bool canTransitionTo(ChunkState newState) const;
    bool isProcessing() const;
    bool canUnload() const;
    bool canRemesh() const;

    static const char *toString(ChunkState state);
};