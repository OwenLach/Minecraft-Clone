#pragma once
#include "Camera.h"
#include "InputManager.h"
#include "Shader.h"
#include "TextureAtlas.h"
#include "Chunk/Chunk.h"
#include "ImGuiManager.h"
#include "Crosshair.h"
#include "World.h"

struct GLFWwindow;

class Application
{
public:
    Application();
    void run();

private:
    GLFWwindow *window_;
    std::unique_ptr<Camera> camera_;
    std::unique_ptr<InputManager> inputManager_;
    std::unique_ptr<Shader> shader_;
    std::unique_ptr<TextureAtlas> textureAtlas_;
    std::unique_ptr<ImGuiManager> imguiManager_;
    std::unique_ptr<World> world_;
    std::unique_ptr<Crosshair> crosshair_;

    float lastFrame_;
    float fpsTimer_;
    int frameCount_;
    float fpsToDisplay_;

    void init();
    void initWindow();
    void initOpenGL();
    void initComponents();

    void shutdown();

    void mainLoop();
    void render();
    void update(const float dt);

    void setGLRenderState();
    float getDeltaTime();
    void processInput(float dt);
    void updateFPS(const float dt);
    void setupImGuiUI();
};