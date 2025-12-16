project(Editor)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

set(SPDLOG_HEADER_ONLY ON CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.16.0
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
    imgui_src
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.5-docking
)
FetchContent_MakeAvailable(imgui_src)

add_library(imgui STATIC
    ${imgui_src_SOURCE_DIR}/imgui.cpp
    ${imgui_src_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_src_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_src_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_src_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_src_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    ${imgui_src_SOURCE_DIR}
    ${imgui_src_SOURCE_DIR}/backends
)

add_executable(Editor
    glad/src/gl.c
    src/OpenGL/GLShader.cpp
    src/Utils/Log.cpp
    src/Chip8.cpp
    src/Editor.cpp
    src/Main.cpp
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res
     DESTINATION ${CMAKE_BINARY_DIR}/Editor)

target_compile_definitions(Editor PUBLIC
    GLFW_INCLUDE_NONE
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Editor PUBLIC
        DEBUG
        LOGGING_ENABLED
        ASSERTS_ENABLED
    )
endif()

target_include_directories(Editor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/glad/include
)

cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)

target_link_libraries(Editor PRIVATE
    glfw
    OpenGL::GL
    imgui
    spdlog::spdlog_header_only
)
