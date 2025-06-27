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
    UNLOADING
};

class ChunkStateMachine
{
private:
    std::atomic<ChunkState> currentState_{ChunkState::EMPTY};

public:
    bool canTransitionTo(ChunkState newState) const;
    void setState(ChunkState state);
    ChunkState getState() const;
    bool isProcessing() const;
    bool isReady() const;
    bool canUnload() const;

    static const char *toString(ChunkState state);
};