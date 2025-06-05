#pragma once

#include <glm/glm.hpp>

#include <array>

namespace Constants
{
    constexpr unsigned int SCREEN_W = 1200;
    constexpr unsigned int SCREEN_H = 800;

    constexpr float YAW = -90.0f;
    constexpr float PITCH = 0.0f;
    constexpr float ZOOM = 45.0f;
    constexpr float PLAYER_SPEED = 15.0f;
    constexpr float MOUSE_SENSITIVITY = 0.05f;
    constexpr float ZOOM_SENSITIVITY = 5.0f;

    constexpr int CHUNK_SIZE_X = 16;
    constexpr int CHUNK_SIZE_Y = 32;
    constexpr int CHUNK_SIZE_Z = 16;

    constexpr int RENDER_DISTANCE = 8;
}