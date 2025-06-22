#include "World.h"

World::World(Camera &camera, Shader &shader, TextureAtlas *textureAtlas)
    : camera_(camera), chunkManager_(shader, textureAtlas, camera)
{
}

void World::render()
{
    chunkManager_.render();
}

void World::update()
{
    chunkManager_.update();
}