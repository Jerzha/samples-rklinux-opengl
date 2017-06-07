rm -f CMakeCache.txt
cmake -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=./aarch64_linux_gnu.cmake

make -j2
