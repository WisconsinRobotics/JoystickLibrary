cmake_minimum_required(VERSION 3.1)
project(JoystickLibrary)

set (JoystickLibrary_VERSION_MAJOR 1)
set (JoystickLibrary_VERSION_MINOR 0)

find_package(Threads REQUIRED)
set (CMAKE_CXX_STANDARD 11)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(src)
add_subdirectory(sample)
