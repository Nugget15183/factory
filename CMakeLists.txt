cmake_minimum_required(VERSION 4.0)
project(factory)

set(CMAKE_CXX_STANDARD 20)
include(FetchContent)

#fetch ImGui
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        v1.91.9
)
FetchContent_MakeAvailable(imgui)

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

add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui PUBLIC  glfw)

add_executable(factory src/main.cpp
        src/engine.cpp
        src/engine.h
        src/util.h
        src/stb_image.h
        src/tiny_obj_loader.h
        src/world.cpp
        src/world.h)

target_link_libraries(factory PRIVATE glfw glad glm imgui)