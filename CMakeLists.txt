cmake_minimum_required(VERSION 3.28)
set(EXECUTABLE_NAME "RendererBuild")
project(${EXECUTABLE_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")
set(LIBRARY_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH})

if (WIN32)
    link_directories("${CMAKE_SOURCE_DIR}/lib")
else ()
    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)

    #-DOpenImageDenoise_DIR=/opt/intel/oneapi/oidn/2.2/lib/cmake/OpenImageDenoise
    #LD_LIBRARY_PATH=/opt/intel/oneapi/tbb/2021.12/env/../lib/intel64/gcc4.8:/opt/intel/oneapi/rkcommon/1.13/lib:/opt/intel/oneapi/ospray_studio/1.0/lib:/opt/intel/oneapi/ospray/3.1/lib:/opt/intel/oneapi/openvkl/2.0/lib:/opt/intel/oneapi/openpgl/0.6/lib:/opt/intel/oneapi/oidn/2.2/lib:/opt/intel/oneapi/mpi/2021.12/opt/mpi/libfabric/lib:/opt/intel/oneapi/mpi/2021.12/lib:/opt/intel/oneapi/ispc/1.23/lib:/opt/intel/oneapi/embree/4.3/lib:/opt/intel/oneapi/compiler/2024.1/opt/compiler/lib:/opt/intel/oneapi/compiler/2024.1/lib:/usr/local/cuda-12.9/lib64:
    find_package(OpenImageDenoise REQUIRED)
endif ()

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
        include/hittable/Parallelogram.hpp
        include/material/DiffuseLight.hpp
        include/hittable/Transform.hpp
        include/util/Matrix.hpp
        src/util/Matrix.cpp
        include/util/OrthonormalBase.hpp
        include/pdf/CosinePDF.hpp
        include/pdf/HittablePDF.hpp
        include/pdf/MixturePDF.hpp
        include/hittable/Box.hpp
        include/material/Dielectric.hpp
        src/Camera.cpp
        include/util/Denoiser.hpp
)

if (WIN32)
target_link_libraries(${EXECUTABLE_NAME} PUBLIC mingw32 SDL2main)
endif ()
target_link_libraries(${EXECUTABLE_NAME} PUBLIC SDL2 SDL2_image)
target_link_libraries(${EXECUTABLE_NAME} PUBLIC OpenImageDenoise)
