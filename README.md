# Opengl via libdrm sample
## Build:

1. building in target borad:
    cmake .
    make

2. building in cross-compiling env
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=./aarch64_linux_gnu.cmake
    make
    (also you can run build.sh)

## Depandency Libarary

* libdrm.so (libdrm-rockchip)
* libmali.so (libmali.so_gbm_opencl_r13p0_aarch64)

## Note:

You can find more infomation as below:<br/>
    http://stackoverflow.com/questions/23139886/how-to-create-opengl-context-via-drm-linux<br/>

Here is another sample:<br/>
    https://github.com/robclark/kmscube.git<br/>
    
And anothre:<br/>
    https://github.com/glmark2/glmark2<br/>
