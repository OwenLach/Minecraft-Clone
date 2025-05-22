cmake_minimum_required(VERSION 3.10)
project(opengl_test)

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

# Executable
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(opengl_test ${SOURCES})

# add_executable(opengl_test 
#     src/main.cpp
#     src/Application.cpp
#     src/Shader.cpp
#     src/InputManager.cpp
#     src/Camera.cpp
#     src/VertexBuffer.cpp
#     src/VertexArray.cpp
#     src/stb_image_impl.cpp
# )

# Include directories
target_include_directories(opengl_test PRIVATE
    include
    libs/glad/include
    libs/glfw/include
    libs/stb
    libs/glm
)

# Link libraries
target_link_libraries(opengl_test
    glad
    glfw
    opengl32
)
