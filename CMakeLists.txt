cmake_minimum_required(VERSION 3.27)
project(raspberry_pi_oled_panel)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(U8G2_USE_DYNAMIC_ALLOC=1)
add_subdirectory(3rdparty/c-periphery)
include_directories(3rdparty/c-periphery/include)

add_subdirectory(3rdparty/u8g2-arm-linux)
include_directories(3rdparty/u8g2-arm-linux/include)

add_subdirectory(3rdparty/astra)
include_directories(3rdparty/astra/include)

add_subdirectory(3rdparty/spdlog)
include_directories(3rdparty/spdlog/include)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME} panel/main.cpp
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
)

if(CMAKE_SYSROOT)
    set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:${CMAKE_SYSROOT}/usr/lib/${CMAKE_SYSTEM_PROCESSOR}-linux-gnu/pkgconfig")
endif ()

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBNM REQUIRED libnm)

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
if(${USE_SYS_RANDOM_H} OR ${HAVE_GETRANDOM})
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

target_link_libraries(${PROJECT_NAME} astra u8g2-arm-linux periphery spdlog ${LIBNM_LIBRARIES})