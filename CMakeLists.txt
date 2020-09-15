cmake_minimum_required (VERSION 3.16.3)
project (SeriStruct)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

add_subdirectory (src/lib)
add_subdirectory (src/tests)