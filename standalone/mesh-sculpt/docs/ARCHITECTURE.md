# Architecture

```text
mesh_sculpt executable (Application, Window, UI, Renderer)
    +-- mesh_sculpt_platform (AssetLocator)
    +-- mesh_sculpt_io (GeometryFileFormat, GeometryFileService)
            +-- mesh_sculpt_core
    +-- mesh_sculpt_core (MeshDocument, SculptMesh, tools)
```

`SculptApplication` creates the window/OpenGL/ImGui contexts, camera, tool, controller, and UI; its frame loop handles input, updates state, renders the scene/UI, and presents. UI returns actions rather than performing file/geometry operations directly. `MeshSculptTool` coordinates those actions.

`SculptMesh` owns validated vertices and triangle indices. `MeshDocument` adds document state and dirty tracking. `MeshSelection` represents selected elements; `MeshPicker` performs camera-ray selection; `VertexDragManipulator` applies constrained edits. Render-data and overlay builders translate model state without OpenGL, while `MeshSculptRenderer` and `DynamicMesh` form the graphics boundary.

`GeometryFileFormat` is the deterministic versioned JSON codec. `GeometryFileService` performs transactional filesystem loads/saves. Core and I/O tests are graphics-independent. `AssetLocator` is also ImGui-free: it checks executable-adjacent build assets, the installed shared-data layout, then a compile-time development fallback, producing a detailed error on failure.
