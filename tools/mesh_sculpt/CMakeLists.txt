cmake_minimum_required(VERSION 3.10)
project(mesh_sculpt)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create executable
add_executable(mesh_sculpt
    src/main.cpp
    src/MeshSculptTool.cpp
    src/MeshSculptUi.cpp
    ${CMAKE_SOURCE_DIR}/app/src/controllers/MeshSculptController.cpp   # <-- add this
)

# Include directories
target_include_directories(mesh_sculpt
    PRIVATE
        ${CMAKE_SOURCE_DIR}/engine/include
        ${CMAKE_SOURCE_DIR}/tools/mesh_sculpt/include
        ${CMAKE_SOURCE_DIR}/app/include
)

# Link against engine library and imgui
target_link_libraries(mesh_sculpt PUBLIC maze_engine imgui)

# Optional: link ImGui if needed
# target_link_libraries(mesh_sculpt PRIVATE imgui)

target_compile_definitions(mesh_sculpt
    PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}"
)
