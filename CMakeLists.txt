cmake_minimum_required(VERSION 3.10)
project(MinecraftClone)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O0 -g")  # Turn off Release optimizations

# GLAD
add_library(glad libs/glad/glad.c)
target_include_directories(glad PUBLIC libs/glad/include)

# GLFW (statically linked)
add_library(glfw STATIC IMPORTED)
set_target_properties(glfw PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/libs/glfw/lib-mingw-w64/libglfw3.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/libs/glfw/include"
)

# ImGui
file(GLOB IMGUI_SOURCES
    libs/imgui/*.cpp
    libs/imgui/backends/imgui_impl_glfw.cpp
    libs/imgui/backends/imgui_impl_opengl3.cpp
)
add_library(imgui ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
    libs/imgui
    libs/imgui/backends
    "${CMAKE_SOURCE_DIR}/libs/glfw/include" 
)

# Executable
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(minecraft_clone ${SOURCES})

# Include directories
target_include_directories(minecraft_clone PRIVATE
    include
    libs/glad/include
    libs/glfw/include
    libs/stb
    libs/glm
    libs/fastnoiselite
)

# Link libraries
target_link_libraries(minecraft_clone
    glad
    glfw
    imgui
    opengl32
)
