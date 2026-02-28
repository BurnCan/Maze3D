add_library(app
    src/controllers/FPSController.cpp
    src/controllers/EditorFlyController.cpp
    src/controllers/MeshSculptController.cpp
)

target_include_directories(app
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(app
    PUBLIC
        maze_engine
        glfw
)
