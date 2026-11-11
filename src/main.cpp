#include <iostream>
#include "engine.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main() {
    auto* e = new engine();

    while (!glfwWindowShouldClose(&e->getWindow())) {
        e->update();
        e->render();
    }

    return 0;
}