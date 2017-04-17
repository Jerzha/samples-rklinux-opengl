Build:

1. building in rk borad:
    cmake .
    make

2. building in cross-compiling env
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=./aarch64_linux_gnu.cmake
    make
    (also can run build.sh)

Note:

You can find more infomation as below:
    http://stackoverflow.com/questions/23139886/how-to-create-opengl-context-via-drm-linux

Here is another sample:
    https://github.com/robclark/kmscube.git