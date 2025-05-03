cmake_minimum_required(VERSION 3.10)
project(opengl_test)

set(CMAKE_CXX_STANDARD 17)

# Add your source files
file(GLOB_RECURSE SRC_FILES src/*.cpp)

# Add GLAD
add_library(glad libs/glad/glad.c)  # Path to GLAD's source file
target_include_directories(glad PUBLIC libs/glad/include)  # Path to glad.h

# Add executable
add_executable(opengl_test ${SRC_FILES})

# Include GLFW and GLAD headers
target_include_directories(opengl_test PRIVATE 
    libs/glfw/include
    libs/glad/include
)

# Link GLFW statically (you already have the .a file)
target_link_libraries(opengl_test 
    glad
    ${CMAKE_SOURCE_DIR}/libs/glfw/lib-mingw-w64/libglfw3.a
    opengl32
)
