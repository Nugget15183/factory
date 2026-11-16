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
    if (grid[idx(pos)].get()) {return false;}

    cellType c = cells[idx(pos)];

    if (c==OBSTRUCTED)return false;
    if (defOf(t).needsOre && c == GROUND) return false;
    return true;
}

void world::update(float dt) {
    for (auto &b : grid) {
        if (!b)continue;

        const buildingDef &d = defOf(b->type);

        double t = glfwGetTime();

        if (b->type == SELLER) {
            if (b->heldIn!=NONE && b->heldInCount>0) {
                int value = sellPrice(b->heldIn);
                money += value;
                b->heldInCount--;
                if (b->heldInCount==0) b->heldIn=NONE;
            }
            continue;
        }

        if (!d.hasOutput) continue;

        itemType out = NONE;
        int need = d.hasInput ? 1 : 0;

        switch (b->type) {
            case DRILL: {
                out = b->convertTo;
                break;
            }
            case SMELTERY: {
                out = smeltResult(b->heldIn);
                break;
            }
            case CONSTRUCTOR: {
                const recipe *r = recipeFor(b->convertTo);

                if (r && b->heldIn == r->in) {
                    out = r->out;
                    need = r->inCount;
                }

                break;
            }
            default: {
                out = b->heldIn;
                break;
            }
        }

        bool ready = out != NONE && b->heldInCount >= need && (b->heldOut == NONE || b->heldOut == out);

        float ct = d.doConversions ? d.convertTime : 0.0f;

        if (!ready) {
            b->lastConversion = t; // don't bank idle time
            b->progress = 0.0f;
        } else if (ct <= 0.0f || t - b->lastConversion >= ct) {
            if (d.hasInput) {
                b->heldInCount--;
                if (b->heldInCount == 0) b->heldIn = NONE;
            }

            b->heldOut = out;
            b->heldOutCount++;
            b->lastConversion = t;
            b->progress = 0.0f;
        } else {
            b->progress = (float)((t - b->lastConversion) / ct);
        }

        //moving items between buildings
        if (b->heldOut != NONE && b->heldOutCount > 0 && t - b->lastMoveOut >= d.moveTime) {
            building *f = getBuilding(b->pos + dirVec(b->dir));

            if (f && defOf(f->type).hasInput) {
                bool same  = f->heldIn == b->heldOut;
                bool empty = f->heldIn == NONE;

                if (same || empty) {
                    f->heldIn = b->heldOut;
                    f->heldInCount++;
                    b->heldOutCount--;
                    b->lastMoveOut = t;
                    if (b->heldOutCount == 0) b->heldOut = NONE;
                }
            }
        }
    }
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

    if (t==DRILL) {
        switch (cells[idx(pos)]) {
            case COAL: b->convertTo = COAL_ORE; break;
            case IRON: b->convertTo = IRON_ORE; break;
            case COPPER: b->convertTo = COPPER_ORE; break;
            default: break;
        }
    }

    grid[idx(pos)] = std::move(b);
    return true;
}