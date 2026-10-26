cmake_minimum_required(VERSION 3.20)
project(raytracer LANGUAGES CXX)

option(ENABLE_PROFILING "Build with gprof instrumentation" OFF)
option(ENABLE_COVERAGE "Build with gcov coverage instrumentation" OFF)
option(BUILD_TESTING "Build unit tests when GoogleTest is available" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(raytracer_core
    geometry/triangle.cc
    world/world.cc
    utility/obj_loader.cc
    utility/bmp_writer.cc
    view/camera.cc
    view/renderer.cc)
target_include_directories(raytracer_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(raytracer_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(raytracer raytracer.cc)
target_link_libraries(raytracer PRIVATE raytracer_core)

if(ENABLE_PROFILING)
    target_compile_options(raytracer_core PUBLIC -pg)
    target_link_options(raytracer_core PUBLIC -pg)
endif()
if(ENABLE_COVERAGE)
    target_compile_options(raytracer_core PUBLIC --coverage)
    target_link_options(raytracer_core PUBLIC --coverage)
endif()

if(BUILD_TESTING)
    include(CTest)
    find_package(GTest QUIET)
    if(GTest_FOUND)
        add_subdirectory(tests)
    else()
        message(STATUS "GoogleTest not found; configure with -DBUILD_TESTING=OFF or install GTest")
    endif()
endif()
