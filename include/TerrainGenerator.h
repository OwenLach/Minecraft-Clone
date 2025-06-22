#pragma once

#include "FastNoiseLite.h"

class TerrainGenerator
{
public:
    TerrainGenerator();

    float getTerrainNoise(const float x, const float z);
    float getCaveNoise(const float x, const float y, const float z);

private:
    FastNoiseLite terrainNoise;
    FastNoiseLite caveNoise;
};