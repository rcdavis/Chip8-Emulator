project(Editor)

include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.2
)
FetchContent_MakeAvailable(glm)

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

add_executable(Editor
    glad/src/gl.c
    src/OpenGL/GLIndexBuffer.cpp
    src/OpenGL/GLShader.cpp
    src/OpenGL/GLTexture.cpp
    src/OpenGL/GLVertexArray.cpp
    src/OpenGL/GLVertexBuffer.cpp
    src/Utils/Log.cpp
    src/Chip8.cpp
    src/Editor.cpp
    src/Main.cpp
    stb/src/stb_image.c
)

target_compile_definitions(Editor PUBLIC
    GLFW_INCLUDE_NONE
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Editor PUBLIC
        DEBUG
        LOGGING_ENABLED
    )
endif()

target_include_directories(Editor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/stb/include
)

cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)

target_link_libraries(Editor PRIVATE
    glm::glm
    glfw
    OpenGL::GL
    spdlog::spdlog_header_only
)
