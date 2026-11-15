#include <iostream>
#include "engine.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

int main() {
    auto* e = new engine();

    while (!glfwWindowShouldClose(&e->getWindow())) {
        e->update();
        e->render();
    }

    e->close();

    return 0;
}