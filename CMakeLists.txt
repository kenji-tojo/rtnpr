cmake_minimum_required(VERSION 3.20)
project(rtnpr LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED  ON)

add_subdirectory(ext/glfw)
target_compile_options(glfw PRIVATE -fPIC)
add_subdirectory(ext/glad)
target_compile_options(glad PRIVATE -fPIC)
find_package(OpenGL REQUIRED)
set(GLM_DIR ext/glm)

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)
set(IMGUI_DIR ext/imgui)
set(IMGUI_SRC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
)

find_package(Eigen3 3.3 REQUIRED NO_MODULE)

set(LIBIGL_WITH_OPENGL OFF)
set(LIBIGL_WITH_OPENGL_GLFW OFF)
set(LIBIGL_WITH_VIEWER OFF)
set(LIBIGL_WITH_OPENGL_GLFW_IMGUI OFF)
set(LIBIGL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/libigl)
add_subdirectory(${LIBIGL_DIR})

add_subdirectory(rtnpr)
target_compile_options(rtnpr-static PRIVATE -fPIC)

find_package(Python COMPONENTS Interpreter Development REQUIRED)
add_subdirectory(ext/nanobind)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB VIEWER_SRC viewer/*.cpp)

nanobind_add_module(rtnpr
    nbind.cpp
    ${VIEWER_SRC}
    ${IMGUI_SRC}
)

target_include_directories(rtnpr PRIVATE
    ./
    ext/delfem2/include
    ${LIBIGL_DIR}/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(rtnpr PRIVATE
    rtnpr-static
    Eigen3::Eigen
    glfw
    glad
    ${OPENGL_LIBRARIES}
)