cmake_minimum_required(VERSION 3.20)
project(rtnpr-static LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED  ON)

# dependencies
find_package(Eigen3 3.3 REQUIRED NO_MODULE)
set(BVH_DIR ${CMAKE_SOURCE_DIR}/ext/bvh/src)

add_library(rtnpr_static
    plane.cpp
    trimesh.cpp
    raytracer.cpp
)
target_include_directories(rtnpr_static PRIVATE ${BVH_DIR})
target_link_libraries(rtnpr_static PRIVATE  Eigen3::Eigen)
