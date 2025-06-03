#pragma once
#include "Camera.h"
#include "InputManager.h"
#include "Shader.h"
#include "Chunk.h"
#include "ImGuiManager.h"
#include "World.h"

struct GLFWwindow;

class Application
{
public:
    Application();
    void run();

private:
    GLFWwindow *window;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<InputManager> inputManager;
    std::unique_ptr<Shader> shader;
    std::unique_ptr<TextureAtlas> textureAtlas;
    std::unique_ptr<ImGuiManager> imguiManager;
    std::unique_ptr<World> world;

    float lastFrame;
    float fpsTimer;
    int frameCount;
    float fpsToDisplay;

    bool init();
    bool initWindow();
    bool initOpenGL();

    void shutdown();

    void mainLoop();
    void render();
    void update(const float dt);

    void setupInputCallbacks();
    void setGLRenderState();
    float getDeltaTime();
    void processInput(float dt);
    void updateFPS(const float dt);
    void setupImGuiUI();
};