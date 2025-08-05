#include "Chunk/ChunkStateMachine.h"

#include <stdexcept>

bool ChunkStateMachine::canTransitionTo(ChunkState newState) const
{
    ChunkState currentState = currentState_.load();

    switch (currentState)
    {
    case ChunkState::EMPTY:
        return newState == ChunkState::TERRAIN_GENERATING;

    case ChunkState::TERRAIN_GENERATING:
        return newState == ChunkState::TERRAIN_READY ||
               newState == ChunkState::UNLOADING;

    case ChunkState::TERRAIN_READY:
        return newState == ChunkState::LIGHT_PROPOGATING ||
               newState == ChunkState::UNLOADING;

    case ChunkState::LIGHT_PROPOGATING:
        return newState == ChunkState::LIGHT_READY ||
               newState == ChunkState::UNLOADING;

    case ChunkState::LIGHT_READY:
        return newState == ChunkState::MESH_GENERATING ||
               newState == ChunkState::UNLOADING;

    case ChunkState::MESH_GENERATING:
        return newState == ChunkState::MESH_READY ||
               newState == ChunkState::UNLOADING;

    case ChunkState::MESH_READY:
        return newState == ChunkState::LOADED ||
               newState == ChunkState::UNLOADING ||
               newState == ChunkState::NEEDS_MESH_REGEN;

    case ChunkState::LOADED:
        return newState == ChunkState::NEEDS_MESH_REGEN ||
               newState == ChunkState::UNLOADING;

    case ChunkState::UNLOADING:
        return newState == ChunkState::EMPTY;

    case ChunkState::NEEDS_MESH_REGEN:
        return newState == ChunkState::MESH_GENERATING ||
               newState == ChunkState::UNLOADING;
    }

    return false;
}

void ChunkStateMachine::setState(ChunkState newState)
{
    if (!canTransitionTo(newState))
    {
        throw std::runtime_error("Invalid state transition from " +
                                 std::string(toString(currentState_.load())) + " to " +
                                 std::string(toString(newState)));
    }
    currentState_.store(newState);
}

ChunkState ChunkStateMachine::getState() const
{
    return currentState_.load();
}

bool ChunkStateMachine::isProcessing() const
{
    ChunkState state = currentState_.load();
    return state == ChunkState::TERRAIN_GENERATING ||
           state == ChunkState::LIGHT_PROPOGATING ||
           state == ChunkState::MESH_GENERATING ||
           state == ChunkState::UNLOADING;
}

bool ChunkStateMachine::canUnload() const
{
    return !isProcessing();
}

bool ChunkStateMachine::canRemesh() const
{
    ChunkState state = currentState_.load();
    return (state == ChunkState::LOADED || state == ChunkState::MESH_READY) && !isProcessing();
}
const char *ChunkStateMachine::toString(ChunkState state)
{
    switch (state)
    {
    case ChunkState::EMPTY:
        return "EMPTY";
    case ChunkState::TERRAIN_GENERATING:
        return "TERRAIN_GENERATING";
    case ChunkState::TERRAIN_READY:
        return "TERRAIN_READY";
    case ChunkState::LIGHT_PROPOGATING:
        return "LIGHT PROPOGATING";
    case ChunkState::LIGHT_READY:
        return "LIGHT READY";
    case ChunkState::MESH_GENERATING:
        return "MESH_GENERATING";
    case ChunkState::MESH_READY:
        return "MESH_READY";
    case ChunkState::LOADED:
        return "LOADED";
    case ChunkState::UNLOADING:
        return "UNLOADING";
    case ChunkState::NEEDS_MESH_REGEN:
        return "NEEDS_MESH_REGEN";
    default:
        return "UNKNOWN";
    }
}