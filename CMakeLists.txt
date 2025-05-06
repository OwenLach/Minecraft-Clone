cmake_minimum_required(VERSION 3.10)
project(opengl_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O0 -g")  # Turn off Release optimizations

# Include source files
file(GLOB_RECURSE SRC_FILES src/*.cpp)

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
add_executable(opengl_test ${SRC_FILES})

# Include directories
target_include_directories(opengl_test PRIVATE
    libs/glad/include
    libs/glfw/include
    libs/stb
)

# Link libraries
target_link_libraries(opengl_test
    glad
    glfw
    opengl32
)
