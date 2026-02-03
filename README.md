# Maze3D

Maze3D is a small C++ / OpenGL project featuring a custom lightweight engine and a 3D maze game.
It includes procedural maze generation, collision detection, and batched wall rendering.

---

## Features

- Procedural maze generation (recursive backtracker)
- Batched maze wall rendering (`MazeMesh`)
- FPS-style camera with mouse look
- Player collision against maze walls
- Simple engine/game separation
- CMake-based build system

---

----

### Repository Structure
```bash

Maze3D/
├── assets/ # Shaders and runtime assets
├── cmake/ # CMake helper files (dependencies, etc.)
├── engine/ # Engine source (rendering, maze, window, camera)
├── game/ # Game executable
├── editor/ # (Optional / future use)
├── CMakeLists.txt
└── README.md

```

----

-----
#### Dependencies

You will need:

- **C++17 compatible compiler**
  - GCC 9+ / Clang 10+ recommended
- **CMake 3.16+**
- **OpenGL 4.5**
- **GLFW**
- **GLAD**
- **GLM**

All third-party dependencies are fetched automatically via CMake where applicable.

-----

##### Build Instructions (Linux / macOS)

```bash
git clone https://github.com/burncan/Maze3D.git
cd Maze3D

mkdir build
cd build

cmake ..
cmake --build .
Running the Game
From the build directory:

bash

./game/maze_game
The game starts in fullscreen mode by default.

Controls
WASD — Move

Mouse — Look around

ESC — Close window (or window close button)

Notes
Asset paths are resolved using the MAZE3D_ASSET_ROOT compile definition.

Maze walls are generated once and rendered as a single batched mesh.

Back-face culling is supported and wall geometry is CCW-consistent.

Status
This project is under active development.
Expect frequent changes and refactors.
