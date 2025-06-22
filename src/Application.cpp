#include "Application.h"
#include "Constants.h"
#include "VertexArray.h"
#include "TextureAtlas.h"
#include "BlockTypes.h"
#include "Block.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

Application::Application()
    : lastFrame_(0.0f),
      fpsTimer_(0.0f),
      frameCount_(0),
      fpsToDisplay_(0.0f)
{
}

void Application::run()
{
    try
    {
        init();
        mainLoop();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Application error: " << e.what() << std::endl;
    }

    shutdown();
}

void Application::init()
{
    initWindow();
    initOpenGL();
    initComponents();
}

void Application::initWindow()
{
    // initialize and configure glfw window
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // window creation
    window_ = glfwCreateWindow(Constants::SCREEN_W, Constants::SCREEN_H, "Minecraft Clone", NULL, NULL);
    if (!window_)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to make GLFW window");
    }

    glfwMakeContextCurrent(window_);

    // Enable VSync
    glfwSwapInterval(1);
}

void Application::initOpenGL()
{
    // load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        throw std::runtime_error("Failed to initialize OpenGL");
    }
}

void Application::initComponents()
{
    camera_ = std::make_unique<Camera>(glm::vec3(0.0f, -100.0f, 0.0f));
    inputManager_ = std::make_unique<InputManager>(camera_.get());
    setupInputCallbacks();
    textureAtlas_ = std::make_unique<TextureAtlas>();
    shader_ = std::make_unique<Shader>("../shaders/vShader.glsl", "../shaders/fShader.glsl");
    imguiManager_ = std::make_unique<ImGuiManager>(window_);
    world_ = std::make_unique<World>(*camera_, *shader_, textureAtlas_.get());
    crosshair_ = std::make_unique<Crosshair>();
}

void Application::shutdown()
{
    glfwTerminate();
}

void Application::mainLoop()
{
    while (!glfwWindowShouldClose(window_))
    {
        float dt = getDeltaTime();

        processInput(dt);
        update(dt);
        render();

        glfwSwapBuffers(window_);
        glfwPollEvents();
    }
}

void Application::render()
{
    setGLRenderState();
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glClearColor(0.529f, 0.808f, 0.922f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureAtlas_->ID);

    shader_->use();
    shader_->setMat4("projection", camera_->getProjectionMatrix());
    shader_->setMat4("view", camera_->getViewMatrix());

    world_->render();
    crosshair_->render();
    imguiManager_->render();
}

void Application::update(const float dt)
{
    updateFPS(dt);
    world_->update();
    imguiManager_->update();
    setupImGuiUI();
}

void Application::setGLRenderState()
{
    // Enable depth testing and cull facing
    glEnable(GL_DEPTH_TEST);
    // Enable face culling
    glEnable(GL_CULL_FACE);
    // Cull back-facing triangles
    glCullFace(GL_BACK);
    // Counter-clockwise is front
    glFrontFace(GL_CCW);
}

void Application::setupInputCallbacks()
{
    // set camera to the window
    glfwSetWindowUserPointer(window_, inputManager_.get());
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetFramebufferSizeCallback(window_, InputManager::framebuffer_size_callback);
    glfwSetCursorPosCallback(window_, InputManager::mouse_callback);
    glfwSetScrollCallback(window_, InputManager::scroll_callback);
}

float Application::getDeltaTime()
{
    float currentFrame = static_cast<float>(glfwGetTime());
    float deltaTime = currentFrame - lastFrame_;
    lastFrame_ = currentFrame;

    return deltaTime;
}

void Application::processInput(float dt)
{
    inputManager_->processKeyboard(window_, dt);
}

void Application::updateFPS(const float dt)
{
    frameCount_++;
    fpsTimer_ += dt;
    if (fpsTimer_ >= 0.5f) // update every 0.5 seconds
    {
        fpsToDisplay_ = frameCount_ / fpsTimer_;
        fpsTimer_ = 0.0f;
        frameCount_ = 0;
    }
}

void Application::setupImGuiUI()
{
    ImGui::Begin("Stats");
    ImGui::SetWindowSize(ImVec2(300, 90));
    ImGui::Text("FPS: %.1f", fpsToDisplay_);
    glm::vec3 camPos = camera_->Position;
    ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", camPos.x, camPos.y, camPos.z);
    float zoom = camera_->Zoom;
    ImGui::Text("FOV: (%.2f)", zoom);
    ImGui::End();
}
