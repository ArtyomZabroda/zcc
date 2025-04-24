cmake_minimum_required(VERSION 3.20)

project(zcc)

add_subdirectory(src)

include(CTest)
enable_testing()
add_subdirectory(tests)