cmake_minimum_required(VERSION 2.8)
project(simulation)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
# link_directories(/usr/local/Cellar/glfw/3.2.1/lib)
find_package(GLUT REQUIRED)

include_directories(
  include
  ${OPENGL_INCLUDE_DIRS}
  ${GLUT_INCLUDE_DIR}
  ${GLEW_INCLUDE_DIRS}
  ${GLFW_INCLUDE_DIRS}
)

add_executable(simulation
  src/main.cpp
  src/simulation.cpp
  src/layout.cpp
  src/car.cpp
  src/inference.cpp
  src/search.cpp
  src/decision_making.cpp
)

target_link_libraries(simulation ${OPENGL_LIBRARY} ${GLUT_LIBRARY} ${GLFW_LIBRARIES})
