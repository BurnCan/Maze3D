# Mesh Sculpt

Mesh Sculpt is a small interactive OpenGL mesh editor. This directory is a
self-contained extraction staging area intended to become a standalone
repository in the future. The implementation was extracted from
[`BurnCan/Maze3D`](https://github.com/BurnCan/Maze3D).

## Features

- GLFW window and OpenGL rendering with a movable FPS-style camera
- Tab-selectable camera and editing modes
- Vertex and triangle picking, selected-element overlays, and vertex dragging
- Triangle deletion
- Editable vertex and triangle-index text
- **New**, **Open**, **Save**, **Save As**, **Reset Mesh**, and **Exit** menu actions

## Indexed geometry files

The tool's project-local `.meshgeo` extension stores portable, human-readable
indexed geometry. The extension is temporary and may change when the standalone
application receives its final product name. Version 1 has this JSON schema:

```json
{
  "format": "indexed-geometry",
  "version": 1,
  "primitive": "triangles",
  "vertices": [
    [0.0, 0.0, 0.0],
    [1.0, 0.0, 0.0],
    [0.0, 1.0, 0.0]
  ],
  "indices": [0, 1, 2]
}
```

Each vertex is exactly three finite coordinates. Indices are zero-based unsigned
integers, and every consecutive group of three describes one triangle. Vertex
and index ordering is preserved, so triangle winding is preserved. All five
shown fields are required and their semantic values are versioned. Version 1
readers ignore unknown top-level fields for limited forward compatibility;
re-saving does not preserve those fields. Unsupported format identifiers,
versions, primitives, and malformed geometry are rejected transactionally.

Version 1 intentionally contains only one indexed triangle mesh. It has no
normals, texture coordinates, materials, transforms, scene hierarchy, or
compression, and it does not claim compatibility with other geometry formats.
Saving writes a complete temporary file beside the destination and then replaces
the destination. POSIX rename replacement is atomic; on platforms whose rename
API cannot replace an existing file, the save reports an error and retains the
old destination.

## Requirements and dependencies

A C++20 compiler, CMake 3.20 or newer, Git, and OpenGL development files are
required. CMake fetches GLFW 3.3.9, GLAD 0.1.36, GLM 0.9.9.8, Dear ImGui
1.91.9 (docking), nlohmann/json 3.11.3, and portable-file-dialogs 0.1.0.

Linux is the only environment verified during this extraction. On Debian or
Ubuntu, install a compiler, CMake, Git, and the X11/OpenGL development packages
needed by GLFW, then run:

```bash
cmake -S standalone/mesh-sculpt -B build/mesh-sculpt
cmake --build build/mesh-sculpt
```

The same CMake commands should be usable from a macOS Terminal with Xcode
Command Line Tools and CMake installed, or from a Windows Developer PowerShell
with Visual Studio's C++ workload, CMake, and Git installed. Those platforms
have not yet been verified, so they are not currently claimed as supported.

## Running

From the Maze3D repository root after building:

```bash
./build/mesh-sculpt/mesh_sculpt
```

Multi-configuration generators may place the executable in a configuration
subdirectory such as `build/mesh-sculpt/Debug/mesh_sculpt.exe`.

## Controls

| Input | Action |
| --- | --- |
| `Tab` | Toggle camera/edit mode |
| Mouse | Look around in camera mode |
| `W` / `S` | Move forward/backward |
| `A` / `D` | Strafe left/right |
| `Q` / `E` | Move down/up |
| Left mouse button | Select and drag a vertex in the sculpt interaction mode |
| `Delete` | Delete the selected triangle |

The on-screen panels expose the vertex/index text and current mode. The File
menu provides document creation, `.meshgeo` open/save, reset, and exit actions.
