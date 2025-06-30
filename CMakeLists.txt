cmake_minimum_required(VERSION 3.27)
project(raspberry_pi_oled_panel)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_SYSROOT)
    set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:${CMAKE_SYSROOT}/usr/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu/pkgconfig")
endif ()

add_compile_definitions(U8G2_USE_DYNAMIC_ALLOC=1)
option(USE_SDL2 "build with SDL2" OFF)

set(PANEL_SRCS panel/main.cpp
        panel/astra_logo.cpp
        panel/astra_rocket.cpp
        sh1106_hal/sh1106_hal.cpp
        sh1106_hal/sh1106_hal.h
        icons/app_icons.cpp
        icons/app_icons.h
        utils/utils.cpp
        utils/utils.h
        panel/text.h
        utils/wifi.cpp
        utils/wifi.h
        utils/nm-shared-utils.c
        utils/nm-shared-utils.h
        utils/nm-utils.c
        utils/nm-utils.h
        panel/wifi.cpp
        panel/wifi.h
        panel/system.cpp
        panel/system.h
        panel/network.cpp
        panel/network.h)
if(USE_SDL2)
    message("=== USE_SDL ===")
    find_package(SDL2 REQUIRED)
    list(APPEND PANEL_SRCS
            sh1106_hal/sdl_hal.cpp
            sh1106_hal/sdl_hal.h)
    list(APPEND PANEL_LIBS ${SDL2_LIBRARIES})
    include_directories(${SDL2_INCLUDE_DIRS})
endif ()

add_subdirectory(3rdparty/u8g2-arm-linux)
include_directories(3rdparty/u8g2-arm-linux/include)

add_subdirectory(3rdparty/astra)
include_directories(3rdparty/astra/include)

add_compile_definitions(-DSPDLOG_COMPILED_LIB)
add_subdirectory(3rdparty/spdlog)
include_directories(3rdparty/spdlog/include)

add_subdirectory(3rdparty/wiringPi)
include_directories(3rdparty/wiringPi/include)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME} ${PANEL_SRCS})

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBNM REQUIRED libnm)
list(APPEND PANEL_LIBS ${LIBNM_LIBRARIES})

set(GRPC_ROOT_DIR "" CACHE PATH "Custom gRPC deps dir")
if ( NOT "${GRPC_ROOT_DIR}" STREQUAL "" )
    # 不是交叉编译才使用gRPC编译出来的bin文件
    if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "${CMAKE_HOST_SYSTEM_PROCESSOR}")
        set(ENV{PATH} "${GRPC_ROOT_DIR}/bin:$ENV{PATH}")
    endif ()
    set(gRPC_DIR "${GRPC_ROOT_DIR}/lib/cmake/grpc")
    set(Protobuf_DIR "${GRPC_ROOT_DIR}/lib/cmake/protobuf")
    set(absl_DIR "${GRPC_ROOT_DIR}/lib/cmake/absl")
    set(utf8_range_DIR "${GRPC_ROOT_DIR}/lib/cmake/utf8_range")
endif ()
add_subdirectory(common)
add_subdirectory(rpc)

foreach (LIBNM_INCLUDE_DIR ${LIBNM_INCLUDE_DIRS})
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SYSROOT}/${LIBNM_INCLUDE_DIR})
endforeach ()

# 引入 CheckSymbolExists 模块
include(CheckSymbolExists)

# 检查 <sys/random.h> 中的 getrandom 函数
check_symbol_exists(getrandom "sys/random.h" USE_SYS_RANDOM_H)

# 检查 <linux/random.h> 中的 getrandom 函数
check_symbol_exists(getrandom "<linux/random.h>" HAVE_GETRANDOM)

# 设置预处理宏
set(USE_SYS_RANDOM_H ${USE_SYS_RANDOM_H})
if ("${USE_SYS_RANDOM_H}" STREQUAL "1" OR "${HAVE_GETRANDOM}" STREQUAL "1")
    set(HAVE_GETRANDOM 1)
else ()
    set(HAVE_GETRANDOM 0)
endif ()

# 创建配置头文件
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

# 添加头文件搜索路径
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
add_dependencies(${PROJECT_NAME} rpi_rpc)
if (USE_SDL2)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DUSE_SDL2)
endif ()
list(APPEND PANEL_LIBS astra u8g2-arm-linux spdlog rpi_rpc common wiringPi)
target_link_libraries(${PROJECT_NAME} ${PANEL_LIBS})