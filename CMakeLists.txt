cmake_minimum_required(VERSION 3.28)
set(EXECUTABLE_NAME "RendererBuild")
project(${EXECUTABLE_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")
set(LIBRARY_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH})

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

include_directories("${CMAKE_SOURCE_DIR}/include")

add_executable(${EXECUTABLE_NAME}
        src/Main.cpp
        include/Global.hpp
        src/Global.cpp
        src/Render.cpp
        include/Render.hpp
        include/basic/Structs.hpp
        include/box/BoundingBox.hpp
        include/box/BVHTree.hpp
        include/material/Metal.hpp
        include/hittable/Triangle.hpp
)

target_link_libraries(${EXECUTABLE_NAME} PUBLIC SDL2 SDL2_image)
