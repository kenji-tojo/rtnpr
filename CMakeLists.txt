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

# libigl
set(LIBIGL_WITH_OPENGL OFF)
set(LIBIGL_WITH_OPENGL_GLFW OFF)
set(LIBIGL_WITH_VIEWER OFF)
set(LIBIGL_WITH_OPENGL_GLFW_IMGUI OFF)
set(LIBIGL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/libigl)
add_subdirectory(${LIBIGL_DIR})

add_subdirectory(rtnpr)

file(GLOB VIEWER_SRC viewer/*.cpp)

add_executable(viewer
    main.cpp
    ${VIEWER_SRC}
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
)

target_include_directories(viewer PRIVATE
    ./
    ext/delfem2/include
    ${LIBIGL_DIR}/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(viewer PRIVATE
    rtnpr_static
    Eigen3::Eigen
    glfw
    glad
    ${OPENGL_LIBRARIES}
)