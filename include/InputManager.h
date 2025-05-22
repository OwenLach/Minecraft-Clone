#pragma once

#include "Camera.h"
#include <GLFW/glfw3.h>

class InputManager
{
public:
    /**
     * Constructs an InputManager and initializes its state.
     *
     * @param camera Pointer to the Camera object that input will control.
     *               Must not be nullptr.
     * Initializes firstMouse to true and sets the initial mouse position
     * to the center of the screen based on Settings.
     */
    InputManager(Camera *camera);

    /**
     * Processes keyboard input and moves the camera accordingly.
     *
     * Checks for specific key presses (W, A, S, D, SPACE, LEFT_CONTROL, ESCAPE)
     * and moves the camera or closes the window based on input.
     *
     * @param window    Pointer to the GLFW window.
     * @param deltaTime Time elapsed since the last frame, used to adjust movement speed.
     */
    void processKeyboard(GLFWwindow *window, float deltaTime);

    /**
     * GLFW callback for window resize events.
     * Adjusts the OpenGL viewport to the new window dimensions.
     *
     * @param window The GLFW window that was resized.
     * @param width  The new width of the framebuffer.
     * @param height The new height of the framebuffer.
     */
    static void framebuffer_size_callback(GLFWwindow *window, int width, int height);

    /**
     * Handles mouse movement events.
     *
     * Retrieves the InputManager instance from the GLFW window user pointer,
     * calculates mouse movement offsets, and forwards them to the camera.
     *
     * @param window Pointer to the GLFW window.
     * @param xposIn Current mouse X position.
     * @param yposIn Current mouse Y position.
     */
    static void mouse_callback(GLFWwindow *window, double xposIn, double yposIn);

    /**
     * Handles mouse scroll wheel input.
     *
     * @param window The GLFW window pointer.
     * @param xoffset Horizontal scroll offset.
     * @param yoffset Vertical scroll offset.
     */
    static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

private:
    Camera *camera;
    bool firstMouse;
    float lastX;
    float lastY;
};