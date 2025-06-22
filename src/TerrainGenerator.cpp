#include "TerrainGenerator.h"

TerrainGenerator::TerrainGenerator()
{
    terrainNoise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
    caveNoise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
}

float TerrainGenerator::getTerrainNoise(const float x, const float z)
{
    float result = 0.0f;
    float amplitude = 1.0f;
    float frequency = 0.3f;
    float maxValue = 0.0f;

    // 4 octaves with proper scaling
    for (int i = 0; i < 5; i++)
    {
        result += terrainNoise.GetNoise(x * frequency, z * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= 0.5f; // Each octave has half the amplitude
        frequency *= 2.0f; // Each octave has double the frequency
    }

    // Normalize to [0, 1]
    return (result / maxValue + 1.0f) * 0.5f;
}

float TerrainGenerator::getCaveNoise(const float x, const float y, const float z)
{
    caveNoise.SetFrequency(0.02f);
    float largeCaves = caveNoise.GetNoise(x, y, z) * 0.5f;

    // Medium cave details
    caveNoise.SetFrequency(0.03f);
    float mediumCaves = caveNoise.GetNoise(x, y, z) * 0.3f;

    // Small cave details
    caveNoise.SetFrequency(0.05f);
    float smallCaves = caveNoise.GetNoise(x, y, z) * 0.2f;

    // normalize [0 - 1]
    return (largeCaves + mediumCaves + smallCaves + 1) * 0.5f;
}
