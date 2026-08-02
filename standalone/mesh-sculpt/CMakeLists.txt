cmake_minimum_required(VERSION 3.20)
project(MeshSculpt LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/Dependencies.cmake)
find_package(OpenGL REQUIRED)

add_executable(mesh_sculpt
    src/main.cpp
    src/app/MeshSculptController.cpp
    src/platform/Window.cpp
    src/render/DynamicMesh.cpp
    src/render/FPSCamera.cpp
    src/render/Shader.cpp
    src/sculpt/MeshSculptTool.cpp
    src/sculpt/MeshSculptUi.cpp
)

target_include_directories(mesh_sculpt PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_compile_definitions(mesh_sculpt PRIVATE
    GLM_FORCE_RADIANS
    MESH_SCULPT_ASSET_ROOT="${PROJECT_SOURCE_DIR}/assets"
)
target_link_libraries(mesh_sculpt PRIVATE
    glad
    glfw
    glm::glm
    mesh_sculpt_imgui
    OpenGL::GL
)

