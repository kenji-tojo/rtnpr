cmake_minimum_required(VERSION 3.20)
project(librtnpr LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED  ON)

# dependencies
find_package(Eigen3 3.3 REQUIRED NO_MODULE)
set(BVH_DIR ${CMAKE_SOURCE_DIR}/ext/bvh/src)

# sources
file(GLOB RTNPR_SRC ${CMAKE_CURRENT_LIST_DIR}/*.cpp)

add_library(rtnpr ${RTNPR_SRC})
target_include_directories(rtnpr PRIVATE ${BVH_DIR})
target_link_libraries(rtnpr PRIVATE  Eigen3::Eigen)