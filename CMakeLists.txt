cmake_minimum_required(VERSION 3.28)
project(minecraft)

set(LIB_DIR ${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)

set(fetchTargets)

find_package(GLEW QUIET)
if (NOT GLEW_FOUND)
    message(STATUS "Could not find glew, downloading instead")
    FetchContent_Declare(
        glew
        GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
    )
    list(APPEND fetchTargets glew)
endif()

find_package(glfw3 3.3 QUIET)
if (NOT glfw3_FOUND)
    message(STATUS "Could not find glfw, downloading instead")
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
    )
    list(APPEND fetchTargets glfw)
endif()

find_package(glm QUIET)
if (NOT glm_FOUND)
    message(STATUS "Could not find glm, downloading instead")
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
    )
    list(APPEND fetchTargets glm)
endif()

find_package(OpenGL REQUIRED)

FetchContent_MakeAvailable(${fetchTargets})

if (NOT GLEW_FOUND)
    add_library(GLEW::GLEW ALIAS libglew_static)
endif()

add_subdirectory(${LIB_DIR}/stb)
add_subdirectory(${LIB_DIR}/DearImGui)

include_directories(include)

file(GLOB_RECURSE srcfiles src/*.cpp src/*.c)

add_executable(${PROJECT_NAME} main.cpp ${srcfiles})

target_link_libraries(${PROJECT_NAME} PRIVATE GLEW::GLEW)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)
target_link_libraries(${PROJECT_NAME} PRIVATE stb::image stb::perlin)
target_link_libraries(${PROJECT_NAME} PRIVATE Dear_ImGui)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)
