 
add_executable(maze_editor)

target_sources(maze_editor
    PRIVATE
        src/main.cpp
        src/EditorViewport.cpp
        ${CMAKE_SOURCE_DIR}/tools/mesh_sculpt/src/MeshSculptTool.cpp
)

target_link_libraries(maze_editor
    PRIVATE
        maze_engine
        imgui
        app
)

target_include_directories(maze_editor
    PRIVATE
        include
        ${CMAKE_SOURCE_DIR}/tools/mesh_sculpt/include
)

target_compile_definitions(maze_editor
    PRIVATE
        PROJECT_ROOT="${CMAKE_SOURCE_DIR}"
)


