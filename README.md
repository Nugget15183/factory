# Factory

A small factory-automation game written from scratch in C++ and OpenGL. Drill ore out of the ground, smelt it, craft it into parts, and ship it down conveyor belts to sell for cash, then reinvest in a bigger production line.

<img width="578" height="426" alt="Screenshot 2026-11-15 210233" src="https://github.com/user-attachments/assets/b3973e44-c032-4d9b-a74d-101eeb9af739" />
<img width="808" height="638" alt="Screenshot 2026-11-15 215127" src="https://github.com/user-attachments/assets/11aeca5d-99de-4a23-89ae-095768453a30" />
<img width="804" height="633" alt="Screenshot 2026-11-15 215222" src="https://github.com/user-attachments/assets/a8339d94-e2ed-4595-88a1-9f7bcc4bb096" />

## Features

- **Procedurally generated world:** a 100x100 map with iron, coal and copper deposits placed using Perlin noise
- **Production chains:** drill ore, smelt it into ingots, craft ingots into parts, then sell
- **Directional logistics:** every building has rotatable input and output ports, with splitters to branch a line
- **Back-pressure:** buildings have stack limits, so a blocked line backs up like a real factory belt
- **Economy:** buildings cost money, selling items earns it
- **Inspector:** click any building to see its contents, progress and (for constructors) pick a recipe
- **Built-in tutorial:** a goals panel walks new players from their first drill to a thousand-dollar economy
- **Live stats:** money, income per second, playtime, items sold and total earned

## Gameplay

### Buildings

| Building | Cost | What it does |
|---|---|---|
| Conveyor | $2 | Moves items one tile forward. Accepts items from its back and both sides, never its front. |
| Splitter | $15 | Takes items in from the back and alternates between its front, left and right outputs. |
| Drill | $50 | Must be placed on an ore tile. Produces that ore forever. |
| Smeltery | $90 | Turns ore into ingots. |
| Constructor | $120 | Crafts ingots into parts using the recipe you pick in the inspector. |
| Seller | $30 | Accepts items from its back and sides and converts them into money. |

You start with **$100**. Removing a building refunds money.

### Items and recipes

| Process | Input | Output |
|---|---|---|
| Smelting | Iron Ore | Iron Ingot |
| Smelting | Copper Ore | Copper Ingot |
| Constructor | 2 x Iron Ingot | 1 Iron Plate |
| Constructor | 1 x Iron Ingot | 1 Iron Rod |
| Constructor | 1 x Copper Ingot | 2 Copper Wire |

Coal can't be smelted yet, but it can be sold. Every item has a sell price, and processed items sell for more than raw ore. Prices and costs are defined in `world.h` (`sellPrice`, `costOf`).

### Ports

Look at the colored cubes on each building:

- 🟧 **Orange** cubes are **outputs**, the sides items leave from
- 🟦 **Blue** cubes are **inputs**, the sides items can enter from

Press **R** while placing to rotate a building. Each building holds one item type per input and output slot, up to a stack of 20.

### Goals

1. Place a drill
2. Place a seller
3. Sell your first item
4. Earn $100
5. Sell an iron ingot
6. Sell a crafted part
7. Earn $1000

The goals panel shows hints for your current goal. Press **H** to hide or show it.

## Controls

| Input | Action |
|---|---|
| **W A S D** | Move the camera |
| **Left click** | Place the selected building (or inspect a building when nothing is selected) |
| **Right click** | Remove a building |
| **R** | Rotate the building you're placing |
| **1 - 6** | Select a building from the hotbar |
| **Q** | Deselect (so you can click buildings to inspect them) |
| **H** | Show/hide the goals panel |
| **Esc** | Quit |

## Building from source

### Requirements

- A C++20 compiler (developed with MSVC 2022 on Windows; other platforms are untested)
- CMake
- A GPU and driver with OpenGL 3.3 core support

### Dependencies

| Library | Purpose |
|---|---|
| [GLFW](https://www.glfw.org/) | Window and input |
| [GLAD](https://github.com/Dav1dde/glad) | OpenGL loader |
| [GLM](https://github.com/g-truc/glm) | Math |
| [Dear ImGui](https://github.com/ocornut/imgui) | User interface |
| [stb_image](https://github.com/nothings/stb) | Image loading |
| [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader) | OBJ mesh loading |

### Build

```sh
git clone https://github.com/Nugget15183/factory.git
cd factory
cmake -S . -B build
cmake --build build
```

### Download
Go [HERE](https://github.com/Nugget15183/factory/releases/tag/alpha), and download the latest release. Once downloaded, unzip the downloaded file and run it.

The game loads shaders and meshes from the `resources/` folder using relative paths, so run the `factory` executable from a directory that contains `resources/`.

## Project structure

```
src/
  main.cpp     Entry point and main loop
  engine.h/.cpp  Window, rendering, input, placement and all UI panels
  world.h/.cpp   World generation, buildings, items, recipes and the simulation
  util.h         Logging and file helpers
resources/
  main.vert / main.frag   Shaders
  meshes/                 OBJ meshes
```

## How it works

- **Rendering:** ground tiles are drawn with a single instanced draw call. Buildings are scaled cubes drawn with a shared mesh.
- **Simulation:** `world::update` runs once per frame. Each building converts its input to its output on a timer, then pushes one item at a time to the neighboring building that accepts it.
- **Placement:** the cursor is unprojected into a ray and intersected with the ground plane to find the hovered tile.

## Known limitations

- Each building has a single input slot, so recipes use one item type
- Merging different item types onto one belt can block it until the mismatched item drains
- Saving and loading aren't implemented
