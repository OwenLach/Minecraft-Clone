#pragma once

namespace Constants
{
    // screen settings
    constexpr unsigned int SCREEN_W = 1200;
    constexpr unsigned int SCREEN_H = 800;

    // camera setttings
    constexpr float YAW = -90.0f;
    constexpr float PITCH = 0.0f;
    constexpr float ZOOM = 80.0f;
    constexpr float PLAYER_SPEED = 40.0f;
    constexpr float MOUSE_SENSITIVITY = 0.05f;
    constexpr float ZOOM_SENSITIVITY = 5.0f;

    // chunk settings
    constexpr int CHUNK_SIZE_X = 16;
    constexpr int CHUNK_SIZE_Y = 256;
    constexpr int CHUNK_SIZE_Z = 16;

    // terrain generation settings
    constexpr int TERRAIN_BASE_HEIGHT = 128;
    constexpr int TERRAIN_HEIGHT_VARIATION = 96;
    constexpr int STONE_LEVEL = 100;
    constexpr float CAVE_THRESHOLD = 0.6f;
    constexpr float CAVE_START_Y = 50;

    // rendering settings
    constexpr int RENDER_DISTANCE = 10;
    constexpr int MAX_CHUNKS_PER_FRAME = 10;
    constexpr int MAX_MESHES_PER_FRAME = 4;

}