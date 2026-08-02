cmake_minimum_required(VERSION 3.20)
project(MeshSculpt LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/Dependencies.cmake)
find_package(OpenGL REQUIRED)
include(CTest)

add_library(mesh_sculpt_model
    src/sculpt/SculptMesh.cpp
    src/sculpt/MeshSelection.cpp
    src/sculpt/MeshPicker.cpp
    src/sculpt/VertexDragManipulator.cpp
)
target_include_directories(mesh_sculpt_model PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(mesh_sculpt_model PUBLIC glm::glm)

add_executable(mesh_sculpt
    src/main.cpp
    src/app/SculptApplication.cpp
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
    mesh_sculpt_model
    glad
    glfw
    glm::glm
    mesh_sculpt_imgui
    OpenGL::GL
)

if(BUILD_TESTING)
    add_executable(sculpt_mesh_tests tests/SculptMeshTests.cpp)
    target_link_libraries(sculpt_mesh_tests PRIVATE mesh_sculpt_model)
    add_test(NAME SculptMeshTests COMMAND sculpt_mesh_tests)

    add_executable(mesh_selection_tests tests/MeshSelectionTests.cpp)
    target_link_libraries(mesh_selection_tests PRIVATE mesh_sculpt_model)
    add_test(NAME MeshSelectionTests COMMAND mesh_selection_tests)

    add_executable(mesh_picker_tests tests/MeshPickerTests.cpp)
    target_link_libraries(mesh_picker_tests PRIVATE mesh_sculpt_model)
    add_test(NAME MeshPickerTests COMMAND mesh_picker_tests)

    add_executable(vertex_drag_manipulator_tests tests/VertexDragManipulatorTests.cpp)
    target_link_libraries(vertex_drag_manipulator_tests PRIVATE mesh_sculpt_model)
    add_test(NAME VertexDragManipulatorTests COMMAND vertex_drag_manipulator_tests)
endif()
