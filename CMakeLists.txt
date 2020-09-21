cmake_minimum_required(VERSION 3.1)
project(Destral)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules/" ${CMAKE_MODULE_PATH})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bins)

add_subdirectory(src)
add_subdirectory(thirdparty)


add_subdirectory(sandbox)
