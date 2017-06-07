cmake_minimum_required(VERSION 3.5)
project(glsample)

set(CMAKE_CXX_STANDARD 11)

include_directories(./native/include)

link_directories(./native/lib)

add_executable(glsample_gbm main_gbm.cpp)
add_executable(glsample_wl main_wayland.cpp)

target_include_directories(glsample_gbm PUBLIC ./native/include/EGLs/winsys_gbm)
target_include_directories(glsample_wl PUBLIC ./native/include/EGLs/winsys_wayland)

target_link_libraries(glsample_gbm mali-gbm drm)
target_link_libraries(glsample_wl mali-wayland drm)