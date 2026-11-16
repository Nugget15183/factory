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
    IRON_ORE,
    COAL_ORE,
    COPPER_ORE,

    IRON_INGOT,
    COPPER_INGOT,

    IRON_PLATE,
    IRON_ROD,
    SCREWS,

    COPPER_WIRE,

    NONE,
};

enum objectType {
    CONVEYOR,
    CONVEYOR_SPLIITER,

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

struct recipe {
    itemType in;
    int inCount;

    itemType out;
    int outCount;
};

struct building {
    objectType type;
    glm::ivec2 pos;
    direction dir = NORTH;

    itemType heldIn = NONE;
    itemType heldOut = NONE;
    itemType convertTo = NONE;

    int heldInCount = 0;
    int heldOutCount = 0;
    int nextOut = 0;

    float progress = 0.0f;

    double lastMoveOut = 0.0f;
    double lastConversion = 0.0f;
};

struct buildingDef {
    const char* name;
    glm::vec4 col;

    float height = 1.0f;
    bool needsOre = false;

    bool hasOutput = false;
    bool hasOutputL = false;
    bool hasOutputR = false;

    bool hasInput = false;
    bool hasInputL = false;
    bool hasInputR = false;

    bool doConversions = false;

    //each item takes this seconds to move out
    float moveTime = 0.1f;

    //seconds to turn inputs (or nothing) into an output
    float convertTime = 1.0f;

    bool anyInput() const {
        return hasInputL || hasInputR || hasInput;
    }

    bool anyOutput() const {
        return hasOutputL || hasOutputR || hasOutput;
    }
};

inline const buildingDef& defOf(objectType t) {
    static const buildingDef defs[OBJECT_COUNT] = {
        {"Conveyor",
            {0.3f, 0.3f, 0.35f, 1},
            0.15f,
            false,
            true,
            false,
            false,
            true,
            true,
            true,
            false,
            0.01f
        },

        {"Conveyor Splitter",
            {0.3f, 0.3f, 0.35f, 1},
            0.15f,
            false,
            true,
            true,
            true,
            true,
            false,
            false,
            false,
            0.01f
        },

        {"Drill",
            {0.8f, 0.7f, 0.2f, 1},
            0.8f,
            true,
            true,
            false,
            false,
            false,
            false,
            false,
            true,
            0.01f
        },

        {"Constructor",
            {0.3f, 0.5f, 0.8f, 1},
            0.9f,
            false,
            true,
            false,
            false,
            true,
            true,
            true,
            true,
            0.1f
        },

        {"Smeltery",
            {0.8f, 0.3f, 0.2f, 1},
            0.9f,
            false,
            true,
            false,
            false,
            true,
            false,
            false,
            true,
            0.1f
        },

        {"Seller",
        {0.2f, 0.7f, 0.3f, 1},
            0.6f,
            false,
            false,
            false,
            false,
            true,
            true,
            true,
            false,
            0.1f
        },
    };
    return defs[t];
}

inline int sellPrice(itemType t) {
    int value = 0;
    switch (t) {
        case itemType::IRON_ORE: {value = 3; break;}
        case itemType::COAL_ORE: {value = 7; break;}
        case itemType::COPPER_ORE: {value = 5; break;}

        case itemType::IRON_INGOT: {value = 10; break;}
        case itemType::COPPER_INGOT: {value = 15; break;}

        case itemType::COPPER_WIRE: {value = 15; break;}

        case itemType::IRON_PLATE: {value = 15; break;}
        case itemType::IRON_ROD: {value = 15; break;}
        case itemType::SCREWS: {value = 15; break;}

        default: value = 0;
    }

    return value;
}

static itemType smeltResult(itemType in) {
    switch (in) {
        case IRON_ORE: return IRON_INGOT;
        case COPPER_ORE: return COPPER_INGOT;
        default: return NONE;
    }
}

static const char* itemName(itemType t) {
    switch (t) {
        case itemType::IRON_ORE: return "Iron Ore";
        case itemType::COAL_ORE: return "Coal Ore";
        case itemType::COPPER_ORE: return "Copper Ore";

        case itemType::IRON_INGOT: return "Iron Ingot";
        case itemType::COPPER_INGOT: return "Copper Ingot";

        case itemType::IRON_PLATE: return "Iron Plate";
        case itemType::IRON_ROD: return "Iron Rood";
        case itemType::SCREWS: return "Screws";

        case itemType::COPPER_WIRE: return "Copper Wire";

        default: return "Empty";
    }
}

static const char* dirName(direction d) {
    switch (d) {
        case NORTH: return "North";
        case EAST:  return "East";
        case SOUTH: return "South";
        default:    return "West";
    }
}

inline direction getOpposite(direction d) {
    switch (d) {
        case NORTH: return SOUTH;
        case SOUTH: return NORTH;
        case EAST: return WEST;
        case WEST: return EAST;
    }
}

inline glm::ivec2 dirVec(direction d) {
    switch (d) {
        case NORTH: return {0, -1};
        case EAST: return {1, 0};
        case SOUTH: return {0, 1};
        default: return {-1, 0};
    }
}

inline direction rotateRight(direction d) {
    return (direction)(((int)d + 1) % 4);
}

inline direction rotateLeft(direction d) {
    return (direction)(((int)d + 3) % 4);
}

inline int outputSides(const building &b, direction out[3]) {
    const buildingDef &d = defOf(b.type);

    int n = 0;

    if (d.hasOutput) out[n++] = b.dir;
    if (d.hasOutputL) out[n++] = rotateLeft(b.dir);
    if (d.hasOutputR) out[n++] = rotateRight(b.dir);

    return n;
}

inline int acceptsFrom(const building &f, direction side) {
    const buildingDef &d = defOf(f.type);

    if (f.type == CONVEYOR) return side != f.dir;

    if (d.hasInput && side == getOpposite(f.dir)) return true;
    if (d.hasInputL && side == getOpposite(f.dir)) return true;
    if (d.hasInputR && side == getOpposite(f.dir)) return true;

    return false;
}

inline const std::vector<recipe>& recipes() {
    static const std::vector<recipe> r = {
        {IRON_INGOT, 2, IRON_PLATE, 1},
        {IRON_INGOT, 1, IRON_ROD, 1},
        {COPPER_INGOT, 1, COPPER_WIRE, 2},
    };

    return r;
}

inline const recipe* recipeFor(itemType out) {
    for (const recipe &r : recipes()) {
        if (r.out == out) return &r;
    }

    return nullptr;
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
    int money = 100;
private:
    int idx(glm::ivec2 p) const {
        return (p.y + height / 2) * wid + (p.x + wid / 2);
    };

    std::vector<cellType> cells;
    std::vector<std::unique_ptr<building>> grid;
};

#endif //FACTORY_WORLD_H