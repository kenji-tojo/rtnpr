cmake_minimum_required(VERSION 3.20)
project(rtnpr LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED  ON)

# OpenGL
add_subdirectory(ext/glfw)
add_subdirectory(ext/glad)
find_package(OpenGL REQUIRED)

# glm
set(GLM_DIR ext/glm)

# ImGui
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)
set(IMGUI_DIR ext/imgui)

# Eigen
find_package(Eigen3 3.3 REQUIRED NO_MODULE)

# bvh
set(BVH_DIR ext/bvh/src)

# libigl
set(LIBIGL_WITH_OPENGL OFF)
set(LIBIGL_WITH_OPENGL_GLFW OFF)
set(LIBIGL_WITH_VIEWER OFF)
set(LIBIGL_WITH_OPENGL_GLFW_IMGUI OFF)
add_subdirectory(ext/libigl)
set(LIBIGL_DIR ext/libigl/include)

# rtnpr sources
file(GLOB RTNPR_SRC src/*.cpp)

add_executable(rtnpr
    main.cpp
    ${RTNPR_SRC}
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
)

target_include_directories(rtnpr PUBLIC
    src
    ext/delfem2/include
    ${GLAD_INCLUDE_DIRS}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${BVH_DIR}
    ${LIBIGL_DIR}
    ${GLM_DIR}
)

target_link_libraries(rtnpr PUBLIC
    Eigen3::Eigen
    glfw
    glad
    ${OPENGL_LIBRARIES}
)