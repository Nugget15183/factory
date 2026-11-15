//
// Created by conte on 11/13/2026.
//

#ifndef FACTORY_WORLD_H
#define FACTORY_WORLD_H
#include <memory>

#include "glm/vec2.hpp"
#include "glm/vec4.hpp"

enum direction {
    NORTH,
    EAST,
    SOUTH,
    WEST
};

enum itemType {
    NONE,
    IRON_ORE,
    COAL_ORE,
    COPPER_ORE,

};

enum objectType {
    CONVEYOR,
    DRILL,
    CONSTRUCTOR,
    SMELTERY,
    SELLER,
    OBJECT_COUNT,
};

enum cellType {
    GROUND,
    IRON,
    COAL,
    COPPER,
    OBSTRUCTED,
};

struct building {
    objectType type;
    glm::ivec2 pos;
    direction dir = direction::NORTH;

    itemType held = itemType::NONE;
    float progress = 0.0f;
};

struct buildingDef {
    const char* name;
    glm::vec4 col;

    float height;
    bool needsOre;
};

inline const buildingDef& defOf(objectType t) {
    static const buildingDef defs[OBJECT_COUNT] = {
        {"Conveyor", {0.3f, 0.3f, 0.35f, 1}, 0.15f, false},
        {"Drill", {0.8f, 0.7f, 0.2f, 1}, 0.8f, true},
        {"Constructor", {0.3f, 0.5f, 0.8f, 1}, 0.9f, false},
        {"Smeltery", {0.8f, 0.3f, 0.2f, 1}, 0.9f, false},
        {"Seller", {0.2f, 0.7f, 0.3f, 1}, 0.6f, false},
    };
    return defs[t];
}

inline glm::ivec2 dirVec(direction d) {
    switch (d) {
        case direction::NORTH: return {0, -1};
        case direction::EAST: return {1, 0};
        case direction::SOUTH: return {0, 1};
        default: return {-1, 0};
    }
}

class world {
public:
    world(int w, int h);

    bool inBounds(glm::ivec2 p) const {
        int x = p.x + wid / 2;
        int y = p.y + height / 2;
        return x >= 0 && x < wid && y >= 0 && y < height;
    }

    cellType getCell(glm::ivec2 pos) const;
    building* getBuilding(glm::ivec2 pos);

    bool canPlace(objectType t, glm::ivec2 pos);
    const std::vector<std::unique_ptr<building>>& buildings() const { return grid; }

    bool place(objectType t, glm::ivec2 pos, direction d);
    bool remove(glm::ivec2 pos);

    void update(float dt);

    int wid, height;
private:
    int idx(glm::ivec2 p) const {
        return (p.y + height / 2) * wid + (p.x + wid / 2);
    };

    std::vector<cellType> cells;
    std::vector<std::unique_ptr<building>> grid;
};


#endif //FACTORY_WORLD_H