#include "world.h"
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>

#include "GLFW/glfw3.h"

building *world::getBuilding(glm::ivec2 pos) {
    if (!inBounds(pos)) return nullptr;
    return grid[idx(pos)].get();
}

cellType world::getCell(glm::ivec2 pos) const {
    if (!inBounds(pos)) return OBSTRUCTED;

    return cells[idx(pos)];
}

bool world::remove(glm::ivec2 pos) {
    if (!inBounds(pos) || grid[idx(pos)] == nullptr) return false;
    grid[idx(pos)].reset();
    return true;
}

bool world::canPlace(objectType t, glm::ivec2 pos) {
    if (!inBounds(pos)) return false;
    if (grid[idx(pos)].get() == nullptr) {return false;}

    cellType c = cells[idx(pos)];

    if (c==OBSTRUCTED)return false;
    if (defOf(t).needsOre && c == GROUND) return false;
    return true;
}


void world::update(float dt) {

}

world::world(int w, int h) {
    wid = w;
    height = h;

    cells.assign(w * h, GROUND);
    grid.resize(w * h);

    float scale = 100.1f;

    for (int x = -w / 2; x < -w / 2 + w; x++) {
        for (int y = -h / 2; y < -h / 2 + h; y++) {
            float n = glm::perlin(glm::vec2(x * scale, y * scale));

            cellType cell;

            if (n < -0.85f) {
                cell = COPPER;
            } else if (n < -0.55f) {
                cell = IRON;
            } else if (n < -0.4f) {
                cell = COAL;
            } else {
                cell = GROUND;
            }

            cells[idx({x, y})] = cell;
        }
    }
}

bool world::place(objectType t, glm::ivec2 pos, direction d) {
    if (!canPlace(t, pos)) return false;

    std::unique_ptr<building> b = std::make_unique<building>();
    b->pos = pos;
    b->dir = d;
    b->type = t;

    grid[idx(pos)] = std::move(b);
    return true;
}