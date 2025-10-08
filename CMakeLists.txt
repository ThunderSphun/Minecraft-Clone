cmake_minimum_required(VERSION 3.28)
project(minecraft)

set(LIB_DIR ${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)

set(fetchTargets)

find_package(GLEW QUIET)
if (NOT GLEW_FOUND)
    message(STATUS "Could not find glew, queuing download")
    FetchContent_Declare(
        glew
        GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
    )
    list(APPEND fetchTargets glew)
endif()

find_package(glfw3 3.3 QUIET)
if (NOT glfw3_FOUND)
    message(STATUS "Could not find glfw, queuing download")
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
    )
    list(APPEND fetchTargets glfw)
endif()

find_package(glm QUIET)
if (NOT glm_FOUND)
    message(STATUS "Could not find glm, queuing download")
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
    )
    list(APPEND fetchTargets glm)
endif()

find_package(OpenGL REQUIRED)

message(STATUS "downloading ${fetchTargets}")
FetchContent_MakeAvailable(${fetchTargets})

add_subdirectory(${LIB_DIR}/stb)
add_subdirectory(${LIB_DIR}/DearImGui)

if (NOT GLEW_FOUND)
    add_library(GLEW::GLEW ALIAS libglew_static)
endif()

add_compile_definitions(
    GLFW_INCLUDE_NONE
)

include_directories(include)

file(GLOB_RECURSE srcFiles src/*.cpp src/*.c)

add_executable(${PROJECT_NAME} main.cpp ${srcFiles})

target_link_libraries(${PROJECT_NAME} PRIVATE GLEW::GLEW)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)
target_link_libraries(${PROJECT_NAME} PRIVATE stb::image stb::perlin)
target_link_libraries(${PROJECT_NAME} PRIVATE Dear_ImGui)

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

if (CMAKE_GENERATOR MATCHES "Visual Studio")
    message(STATUS "setting visual studio specific stuff")
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

    file(GLOB_RECURSE includeFiles "${CMAKE_CURRENT_SOURCE_DIR}/include/*")
    target_sources(${PROJECT_NAME} PRIVATE ${includeFiles})
    source_group(
        TREE "${CMAKE_CURRENT_SOURCE_DIR}/include"
        PREFIX "Header Files"
        FILES ${includeFiles}
    )
endif()
