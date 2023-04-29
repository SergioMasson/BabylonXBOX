cmake_minimum_required(VERSION 3.0.0)
project(BabylonianRobots VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# __cplusplus value is not in sync with language version used. MVSC needs this flag to update it accordingly
# https://gitlab.kitware.com/cmake/cmake/-/issues/18837
if (MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()

add_subdirectory(Dependencies)
add_subdirectory(Game)
add_subdirectory(Editor)


