cmake_minimum_required(VERSION 4.0)
project(factory)

set(CMAKE_CXX_STANDARD 20)

#build glm
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/packages/glm)

#build glad
add_library(glad STATIC packages/glad/src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/packages/glad/include)

#build glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(packages/glfw-3.5.1)

add_executable(factory src/main.cpp
        src/engine.cpp
        src/engine.h
        src/util.h
        src/stb_image.h)

target_link_libraries(factory PRIVATE glfw glad glm)