cmake_minimum_required(VERSION 3.10)

project(Rendering_Project)

set(CMAKE_CXX_STANDARD 11)

include("C:/vcpkg/scripts/buildsystems/vcpkg.cmake")

find_package(OpenGL REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(OpenGLApp opengl_main.cpp)

target_link_libraries(OpenGLApp PRIVATE opengl32 glad::glad glfw glm::glm)
