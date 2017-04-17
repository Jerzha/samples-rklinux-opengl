SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_C_COMPILER "aarch64-linux-gnu-gcc")
SET(CMAKE_CXX_COMPILER "aarch64-linux-gnu-g++")
SET(CMAKE_SYSTEM_PROCESSOR "armv8")

add_definitions(-fPIC)
add_definitions(-DARMLINUX)