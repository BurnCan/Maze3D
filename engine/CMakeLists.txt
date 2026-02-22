add_library(maze_engine STATIC)

target_sources(maze_engine
    PRIVATE
        src/engine_dummy.cpp

        src/window/Window.cpp

        src/render/Shader.cpp
        src/render/CubeMesh.cpp
        src/render/CapsuleMesh.cpp
        src/render/BoxRenderer.cpp
        src/render/DynamicMesh.cpp


        src/scene/FPSCamera.cpp


        src/maze/Maze.cpp
        src/maze/MazeMesh.cpp
        src/maze/MazeCollider.cpp

)

# ✅ PUBLIC API headers live here
target_include_directories(maze_engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

include(${CMAKE_SOURCE_DIR}/cmake/FetchDependencies.cmake)

target_link_libraries(maze_engine
    PUBLIC
        glfw
        glad
        glm::glm
        nlohmann_json::nlohmann_json
)

target_compile_definitions(maze_engine
    PUBLIC
        GLM_FORCE_RADIANS
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        MAZE3D_ASSET_ROOT="${CMAKE_SOURCE_DIR}/assets"
)



