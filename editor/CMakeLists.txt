 
add_executable(maze_editor)

target_sources(maze_editor
    PRIVATE
        src/main.cpp
        src/EditorViewport.cpp
)

target_link_libraries(maze_editor
    PRIVATE
        maze_engine
        imgui
)

target_include_directories(maze_editor
    PRIVATE
        include
)


