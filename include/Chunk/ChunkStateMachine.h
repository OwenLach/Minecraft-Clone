#pragma once

#include <atomic>

enum class ChunkState
{
    EMPTY,
    TERRAIN_GENERATING,
    TERRAIN_READY,
    MESH_GENERATING,
    MESH_READY,
    LOADED,
    UNLOADING,
    NEEDS_MESH_REGEN
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
    bool isReady() const;
    bool canUnload() const;
    bool canRemesh() const;

    static const char *toString(ChunkState state);
};