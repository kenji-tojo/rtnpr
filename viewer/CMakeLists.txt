cmake_minimum_required(VERSION 3.20)
project(viewer LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED  ON)

add_library(viewer viewer.cpp gui.cpp)
