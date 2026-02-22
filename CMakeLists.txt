cmake_minimum_required(VERSION 3.10)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
project(Maze3D VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(MAZE3D_BUILD_EDITOR "Build maze editor" ON)

add_subdirectory(engine)
add_subdirectory(game)
add_subdirectory(app)
add_subdirectory(tools)

if (MAZE3D_BUILD_EDITOR)
    add_subdirectory(editor)
endif()

