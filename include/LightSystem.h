#pragma once

#include <memory>

class World;
class Chunk;

class LightSystem
{
public:
    LightSystem(World *world);
    void propogateSkylight(std::shared_ptr<Chunk> chunk);

private:
    World *world_;
};