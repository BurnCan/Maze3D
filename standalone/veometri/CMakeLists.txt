cmake_minimum_required(VERSION 3.20)
project(Veometri VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
include(GNUInstallDirs)

option(VEOMETRI_BUILD_APP "Build the Veometri desktop application" ON)
option(VEOMETRI_BUILD_TESTS "Build Veometri tests" ${BUILD_TESTING})
option(VEOMETRI_WARNINGS_AS_ERRORS "Treat project warnings as errors" OFF)
option(VEOMETRI_ENABLE_SANITIZERS "Enable supported sanitizers" OFF)
option(VEOMETRI_INSTALL "Enable install rules" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/Dependencies.cmake)

add_library(veometri_warnings INTERFACE)
if(MSVC)
    target_compile_options(veometri_warnings INTERFACE /W4 /permissive-)
    if(VEOMETRI_WARNINGS_AS_ERRORS)
        target_compile_options(veometri_warnings INTERFACE /WX)
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(veometri_warnings INTERFACE -Wall -Wextra -Wpedantic)
    if(VEOMETRI_WARNINGS_AS_ERRORS)
        target_compile_options(veometri_warnings INTERFACE -Werror)
    endif()
endif()

add_library(veometri_sanitizers INTERFACE)
if(VEOMETRI_ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND NOT WIN32)
        target_compile_options(veometri_sanitizers INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(veometri_sanitizers INTERFACE -fsanitize=address,undefined)
    else()
        message(WARNING "Veometri sanitizers are supported only by GCC/Clang on non-Windows platforms")
    endif()
endif()

set(veometri_first_party veometri_warnings veometri_sanitizers)
add_library(veometri_core
    src/sculpt/SculptMesh.cpp src/sculpt/MeshDocument.cpp src/sculpt/MeshSelection.cpp
    src/sculpt/MeshPicker.cpp src/sculpt/VertexDragManipulator.cpp
    src/sculpt/VeometriRenderData.cpp src/sculpt/MeshEditorState.cpp src/sculpt/MeshOverlayData.cpp)
target_include_directories(veometri_core PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(veometri_core PUBLIC glm::glm PRIVATE ${veometri_first_party})

add_library(Veometri::Core ALIAS veometri_core)

add_library(veometri_io src/io/GeometryFileFormat.cpp src/io/GeometryFileService.cpp)
target_include_directories(veometri_io PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(veometri_io PUBLIC veometri_core PRIVATE nlohmann_json::nlohmann_json ${veometri_first_party})

add_library(Veometri::IO ALIAS veometri_io)

add_library(veometri_platform src/platform/AssetLocator.cpp)
target_include_directories(veometri_platform PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_definitions(veometri_platform PRIVATE VEOMETRI_DEVELOPMENT_ASSET_ROOT="${PROJECT_SOURCE_DIR}/assets")
target_link_libraries(veometri_platform PRIVATE ${veometri_first_party})
add_library(Veometri::Runtime ALIAS veometri_platform)

if(VEOMETRI_BUILD_APP)
    find_package(OpenGL REQUIRED)
    veometri_enable_app_dependencies()
    add_executable(veometri src/main.cpp src/app/VeometriApplication.cpp src/app/VeometriController.cpp
        src/platform/Window.cpp src/render/DynamicMesh.cpp src/render/FPSCamera.cpp src/render/Shader.cpp
        src/sculpt/VeometriTool.cpp src/sculpt/VeometriRenderer.cpp src/sculpt/VeometriUi.cpp
        src/sculpt/VeometriEditorUi.cpp)
    target_include_directories(veometri PRIVATE ${PROJECT_SOURCE_DIR}/include)
    target_compile_definitions(veometri PRIVATE GLM_FORCE_RADIANS)
    target_link_libraries(veometri PRIVATE veometri_core veometri_io veometri_platform glad glfw
        glm::glm veometri_imgui portable_file_dialogs OpenGL::GL ${veometri_first_party})
    add_custom_command(TARGET veometri POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:veometri>/assets)
endif()

if(BUILD_TESTING AND VEOMETRI_BUILD_TESTS)
    function(veometri_test target source library)
        add_executable(${target} ${source})
        target_link_libraries(${target} PRIVATE ${library} ${veometri_first_party})
        add_test(NAME ${target} COMMAND ${target})
    endfunction()
    veometri_test(sculpt_mesh_tests tests/SculptMeshTests.cpp veometri_core)
    veometri_test(mesh_selection_tests tests/MeshSelectionTests.cpp veometri_core)
    veometri_test(mesh_document_tests tests/MeshDocumentTests.cpp veometri_core)
    veometri_test(geometry_file_format_tests tests/GeometryFileFormatTests.cpp veometri_io)
    veometri_test(geometry_file_service_tests tests/GeometryFileServiceTests.cpp veometri_io)
    veometri_test(mesh_picker_tests tests/MeshPickerTests.cpp veometri_core)
    veometri_test(vertex_drag_manipulator_tests tests/VertexDragManipulatorTests.cpp veometri_core)
    veometri_test(veometri_render_data_tests tests/VeometriRenderDataTests.cpp veometri_core)
    veometri_test(mesh_editor_state_tests tests/MeshEditorTextStateTests.cpp veometri_core)
    veometri_test(mesh_overlay_data_tests tests/MeshOverlayDataTests.cpp veometri_core)
    veometri_test(asset_locator_tests tests/AssetLocatorTests.cpp veometri_platform)
    target_compile_definitions(geometry_file_format_tests PRIVATE VEOMETRI_EXAMPLE_FILE="${PROJECT_SOURCE_DIR}/examples/cube.meshgeo")
    find_package(Python3 COMPONENTS Interpreter)
    if(Python3_Interpreter_FOUND)
        add_test(NAME standalone_independence COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/scripts/check_standalone_independence.py)
        add_test(NAME product_identity COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/scripts/check_product_identity.py)
    endif()
endif()

find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format)
if(CLANG_FORMAT_EXECUTABLE)
    file(GLOB_RECURSE format_files CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/include/*.h ${PROJECT_SOURCE_DIR}/tests/*.cpp)
    add_custom_target(format COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${format_files})
    add_custom_target(format-check COMMAND ${Python3_EXECUTABLE} ${PROJECT_SOURCE_DIR}/scripts/check_format.py --clang-format ${CLANG_FORMAT_EXECUTABLE})
endif()

if(VEOMETRI_INSTALL)
    if(TARGET veometri)
        install(TARGETS veometri RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif()
    install(DIRECTORY assets/shaders DESTINATION ${CMAKE_INSTALL_DATADIR}/veometri)
    install(FILES README.md LICENSE_STATUS.md THIRD_PARTY_NOTICES.md DESTINATION ${CMAKE_INSTALL_DOCDIR})
endif()

set(CPACK_GENERATOR "TGZ;ZIP")
set(CPACK_PACKAGE_NAME Veometri)
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_FILE_NAME "Veometri-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}")
include(CPack)
