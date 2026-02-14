add_executable(maze_game)

target_sources(maze_game
    PRIVATE
        src/main.cpp
)

target_link_libraries(maze_game
    PRIVATE
        maze_engine
        app
)

if (WIN32)
    target_link_libraries(maze_game PRIVATE opengl32)
elseif (APPLE)
    find_library(OpenGL_LIBRARY OpenGL)
    target_link_libraries(maze_game PRIVATE ${OpenGL_LIBRARY})
else()
    target_link_libraries(maze_game PRIVATE GL)
endif()

