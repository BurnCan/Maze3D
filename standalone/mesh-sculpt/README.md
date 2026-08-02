# Mesh Sculpt

Mesh Sculpt is a small C++20 desktop editor for indexed triangle meshes. The final product name remains undecided.

## Capabilities

It provides OpenGL rendering, an FPS camera, vertex/triangle picking, vertex dragging, triangle deletion, editable geometry text, and New/Open/Save/Save As. No screenshot is currently checked in.

## Platforms and prerequisites

Linux/GCC is verified locally; Ubuntu GCC, Ubuntu Clang, and Windows MSVC are covered by the ready-to-move CI template. Windows is compiled but the graphical program is not launched in CI. macOS is unverified. Requirements are CMake 3.20+, Git, a C++20 compiler, OpenGL, and GLFW's platform development libraries. CMake downloads dependencies, so first configuration and clean-copy verification may require network access.

## Build and test

```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Use `-DMESH_SCULPT_BUILD_APP=OFF` for graphics-independent core/I/O work, `-DMESH_SCULPT_WARNINGS_AS_ERRORS=ON` for strict first-party warnings, or `-DMESH_SCULPT_ENABLE_SANITIZERS=ON` for AddressSanitizer and UndefinedBehaviorSanitizer with supported non-Windows GCC/Clang toolchains. Presets `dev`, `release`, `ci-gcc`, `ci-clang`, and `ci-msvc` are provided.

Run `./build/mesh_sculpt` (multi-config builds may use `build/Debug/mesh_sculpt.exe`). Shaders are copied to `assets` beside the build-tree executable. Runtime lookup tries executable-adjacent assets, then `<prefix>/share/mesh-sculpt`, then a development-only source fallback, and reports every attempted path.

## Install and package

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build build-release
cmake --install build-release --prefix install-root
(cd build-release && cpack)
```

Installation places the executable in `bin`, shaders in `share/mesh-sculpt/shaders`, and documentation/notices in the platform CMake documentation directory. CPack produces TGZ and ZIP archives.

## Controls

`Tab` toggles camera/edit mode. Mouse look and `W/S/A/D/Q/E` move the camera. Left click selects or drags a vertex; `Delete` removes the selected triangle. Menus manage documents.

## `.meshgeo` format

Version 1 is deterministic JSON with `format: indexed-geometry`, `version: 1`, `primitive: triangles`, finite three-component `vertices`, and unsigned triangle `indices`. `examples/cube.meshgeo` is a canonical example. It intentionally has no normals, materials, transforms, hierarchy, or compression.

## Architecture and dependencies

`mesh_sculpt_core` contains graphics-independent document/geometry logic; `mesh_sculpt_io` adds the versioned codec and filesystem service; `mesh_sculpt_platform` resolves assets; the `mesh_sculpt` executable owns graphics and UI. See [architecture details](docs/ARCHITECTURE.md). Direct fetched dependencies are GLFW, GLAD, GLM, Dear ImGui, portable-file-dialogs, and nlohmann/json; see [third-party notices](THIRD_PARTY_NOTICES.md).

## Limitations, provenance, and contributing

There is no undo, autosave, alternative geometry format, or headless graphical smoke test. This standalone project was extracted from [`BurnCan/Maze3D`](https://github.com/BurnCan/Maze3D); that historical attribution is not a build dependency. No first-party redistribution license is currently declared; see [license status](LICENSE_STATUS.md). See [CONTRIBUTING.md](CONTRIBUTING.md) before submitting changes and [the migration checklist](docs/REPOSITORY_MIGRATION.md) before extraction.
