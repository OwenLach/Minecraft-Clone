#pragma once

#include <memory>

class World;
class Chunk;

class LightSystem
{
public:
    LightSystem(World *world);
    void propagateSkylight(std::shared_ptr<Chunk> chunk);

private:
    World *world_;

    void clearChunkLightLevels(std::shared_ptr<Chunk> chunk);
};