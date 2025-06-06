#pragma once

namespace Constants
{
    // screen settings
    constexpr unsigned int SCREEN_W = 1200;
    constexpr unsigned int SCREEN_H = 800;

    // camera setttings
    constexpr float YAW = -90.0f;
    constexpr float PITCH = 0.0f;
    constexpr float ZOOM = 45.0f;
    constexpr float PLAYER_SPEED = 15.0f;
    constexpr float MOUSE_SENSITIVITY = 0.05f;
    constexpr float ZOOM_SENSITIVITY = 5.0f;

    // chunk settings
    constexpr int CHUNK_SIZE_X = 16;
    constexpr int CHUNK_SIZE_Y = 256;
    constexpr int CHUNK_SIZE_Z = 16;

    // terrain gen settings
    constexpr int TERRAIN_BASE_HEIGHT = 128;
    constexpr int TERRAIN_HEIGHT_VARIATION = 48;

    constexpr int RENDER_DISTANCE = 8;
}