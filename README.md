# OLED 网络控制面板
这是一个基于 I2C OLED 显示器的网络控制面板程序。目标实现通过图形界面控制WIFI和有线网络。

# 依赖
本项目依赖 NetworkManager 网络管理模块，需要安装并配置。

# 编译
1. 安装依赖
```text
apt install libnm-dev
```
2. 编译gRPC
```shell
git clone --recursive https://github.com/grpc/grpc.git
cd grpc
mkdir -p build_dir
cd build_dir
cmake -DgRPC_INSTALL=ON \
      -DgRPC_BUILD_TESTS=OFF \
      -DCMAKE_CXX_STANDARD=17 \
      -DCMAKE_INSTALL_PREFIX=dist \
      ..
```

3. 编译命令
```shell
cmake .. \
    -DgRPC_DIR="/{GRPC_SRC_DIR}/build_dir/dist/lib/cmake/grpc" \
    -DProtobuf_DIR="/{GRPC_SRC_DIR}/build_dir/dist/lib/cmake/protobuf" \
    -Dabsl_DIR="/{GRPC_SRC_DIR}/build_dir/dist/lib/cmake/absl" \
    -Dutf8_range_DIR="/{GRPC_SRC_DIR}/build_dir/dist/lib/cmake/utf8_range"
```

+ 交叉编译添加参数
    ```shell
    -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
    -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
    -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
    -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY \
    -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
    -DCMAKE_SYSROOT=/tmp/aarch64-linux-gnu \
    -DCMAKE_C_COMPILER=/usr/bin/aarch64-linux-gnu-gcc-12 \
    -DCMAKE_CXX_COMPILER=/usr/bin/aarch64-linux-gnu-g++-12 \
    -DCMAKE_C_FLAGS="-march=armv8-a -mcpu=cortex-a53" \
    -DCMAKE_CXX_FLAGS="-march=armv8-a -mcpu=cortex-a53" \
    ```