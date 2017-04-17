cmake_minimum_required(VERSION 3.5)
project(glsample)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
include_directories(native/include)

link_directories(native/lib)

add_executable(glsample ${SOURCE_FILES})

target_link_libraries(glsample mali drm)