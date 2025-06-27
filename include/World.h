#pragma once

#include "Chunk/ChunkManager.h"
#include "Camera.h"
#include "TextureAtlas.h"
#include "Shader.h"

class World
{
public:
    World(Camera &camera, Shader &shader, TextureAtlas *textureAtlas);

    void render();
    void update();

private:
    Camera camera_;
    ChunkManager chunkManager_;
};