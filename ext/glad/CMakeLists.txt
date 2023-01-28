cmake_minimum_required(VERSION 3.20)
project(mis LANGUAGES C)

set(GLAD_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/include")

add_library(glad STATIC  "${CMAKE_CURRENT_LIST_DIR}/src/glad.c")
target_include_directories(glad PUBLIC ${GLAD_INCLUDE_DIRS})

