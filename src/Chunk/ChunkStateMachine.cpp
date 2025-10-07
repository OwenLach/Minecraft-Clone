#include "Chunk/ChunkStateMachine.h"

#include <stdexcept>

bool ChunkStateMachine::canTransitionTo(ChunkState newState) const
{
    ChunkState currentState = currentState_.load();

    switch (currentState)
    {
    case ChunkState::EMPTY:
        return newState == ChunkState::TERRAIN_GENERATED;

    case ChunkState::TERRAIN_GENERATED:
        return newState == ChunkState::INITIAL_LIGHT_READY;

    case ChunkState::INITIAL_LIGHT_READY:
        return newState == ChunkState::FINAL_LIGHT_READY;

    case ChunkState::FINAL_LIGHT_READY:
        return newState == ChunkState::MESH_READY;

    case ChunkState::MESH_READY:
        return newState == ChunkState::LOADED;

    case ChunkState::LOADED:
        // Chunk needed to be reupdated because of a neighbor
        return newState == ChunkState::FINAL_LIGHT_READY ||
               newState == ChunkState::MESH_READY;

    // TODO
    case ChunkState::NEEDS_LIGHT_UPDATE:
        break;
    // TODO
    case ChunkState::NEEDS_MESH_REGEN:
        break;
    }

    return false;
}

void ChunkStateMachine::setState(ChunkState newState)
{
    ChunkState currentState = currentState_.load();
    if (currentState == newState)
        return;

    if (!canTransitionTo(newState))
        throw std::runtime_error("Invalid state transition from " + std::string(toString(currentState_.load())) + " to " + std::string(toString(newState)));

    currentState_.store(newState);
}

ChunkState ChunkStateMachine::getState() const
{
    return currentState_.load();
}

const char *ChunkStateMachine::toString(ChunkState state)
{
    switch (state)
    {
    case ChunkState::EMPTY:
        return "EMPTY";
    case ChunkState::TERRAIN_GENERATED:
        return "TERRAIN_GENERATED";
    case ChunkState::INITIAL_LIGHT_READY:
        return "INITIAL_LIGHT_READY";
    case ChunkState::FINAL_LIGHT_READY:
        return "FINAL LIGHT READY";
    case ChunkState::MESH_READY:
        return "MESH_READY";
    case ChunkState::LOADED:
        return "LOADED";
    case ChunkState::NEEDS_LIGHT_UPDATE:
        return "NEEDS LIGHT UPDATE";
    case ChunkState::NEEDS_MESH_REGEN:
        return "NEEDS_MESH_REGEN";
    default:
        return "UNKNOWN";
    }
}