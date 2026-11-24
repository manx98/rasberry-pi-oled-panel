cmake_minimum_required(VERSION 3.22)
project(raspberry_pi_oled_panel)

set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(U8G2_USE_DYNAMIC_ALLOC=1)
option(USE_SDL2 "Build using SDL2 simulator" OFF)
option(PANEL_ENABLE_SENSORS "Enable all optional sensor/actuator devices" OFF)
option(PANEL_ENABLE_INA226 "Enable INA226 power monitor" OFF)
option(PANEL_ENABLE_SGP30 "Enable SGP30 air quality sensor" OFF)
option(PANEL_ENABLE_SHT3X "Enable SHT3x temperature/humidity sensor" OFF)
option(PANEL_ENABLE_BMP280 "Enable BMP280 barometric pressure sensor" OFF)
option(PANEL_ENABLE_FAN "Build GPIO12 temperature-controlled cooling fan" ON)
option(PANEL_ENABLE_RELAY "Enable relay outputs" OFF)
if(PANEL_ENABLE_SENSORS)
    foreach(feature INA226 SGP30 SHT3X BMP280 FAN RELAY)
        set(PANEL_ENABLE_${feature} ON)
    endforeach()
endif()
set(PANEL_OPTIONAL_HARDWARE OFF)
foreach(feature INA226 SGP30 SHT3X BMP280 RELAY)
    if(PANEL_ENABLE_${feature})
        set(PANEL_OPTIONAL_HARDWARE ON)
    endif()
endforeach()
# NetworkManager and Wi-Fi menus are essential; do not allow disabling.
set(PANEL_ENABLE_NETWORK ON CACHE BOOL "Required: build Wi-Fi/NetworkManager features" FORCE)
option(PANEL_ENABLE_RPC "Build unused gRPC server library" OFF)


set(PANEL_SRCS panel/main.cpp
        panel/astra_logo.cpp
        panel/astra_rocket.cpp
        panel/power_dialog.cpp
        panel/power_dialog.h
        panel/power_dialog_state.h
        panel/network_confirmation.cpp
        panel/network_confirmation.h
        panel/network_confirmation_state.h
        panel/status_idle_config.cpp
        panel/status_idle_config.h
        panel/status_idle_settings.cpp
        panel/status_idle_settings.h
        panel/display/orientation_config.cpp
        panel/display/orientation_config.h
        panel/display/orientation_policy.h
        panel/display/orientation_settings.cpp
        panel/display/orientation_settings.h
        sh1106_hal/sh1106_hal.cpp
        sh1106_hal/sh1106_hal.h
        icons/app_icons.cpp
        icons/app_icons.h
        utils/utils.cpp
        utils/utils.h
        panel/text.h
        panel/system.cpp
        panel/system.h
        panel/network.cpp
        panel/network.h
        panel/network_list_layout.h
        utils/wifi.cpp utils/wifi.h
        utils/nm-shared-utils.c utils/nm-shared-utils.h
        utils/nm-utils.c utils/nm-utils.h
        panel/wifi.cpp panel/wifi.h
        panel/wifi_groups_policy.h
        panel/wifi_password_dialog.cpp panel/wifi_password_dialog.h panel/wifi_password_state.h
        utils/wifi_nmcli.cpp utils/wifi_nmcli.h)
if(PANEL_ENABLE_FAN AND NOT USE_SDL2)
    list(APPEND PANEL_SRCS
        panel/fan/fan_controller.cpp
        panel/fan/fan_settings.cpp
        panel/fan/fan_controller.h
        panel/fan/fan_settings.h
        panel/fan/fan_policy.h)
    add_compile_definitions(PANEL_ENABLE_FAN=1)
endif()

if(USE_SDL2)
    message("=== USE_SDL ===")
    find_package(SDL2 REQUIRED)
    list(APPEND PANEL_SRCS
            sh1106_hal/sdl_hal.cpp
            sh1106_hal/sdl_hal.h)
    list(APPEND PANEL_LIBS ${SDL2_LIBRARIES})
    include_directories(${SDL2_INCLUDE_DIRS})
endif ()

add_subdirectory(3rdparty/u8g2-arm-linux EXCLUDE_FROM_ALL)
include_directories(3rdparty/u8g2-arm-linux/include)

add_subdirectory(3rdparty/astra)
include_directories(3rdparty/astra/include)

add_compile_definitions(SPDLOG_COMPILED_LIB)
add_subdirectory(3rdparty/spdlog)
include_directories(3rdparty/spdlog/include)

add_subdirectory(3rdparty/wiringPi)
include_directories(3rdparty/wiringPi/include)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME} ${PANEL_SRCS})

# Ubuntu's older cross-GCC can prefer its bundled glibc 2.35 over the
# target Debian 13 glibc. Put the live SSHFS sysroot libraries first, so
# libnm/GLib are resolved against the same libc as the actual Pi.
if(CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64" AND CMAKE_SYSROOT)
    target_link_options(${PROJECT_NAME} PRIVATE
        "-L${CMAKE_SYSROOT}/lib/aarch64-linux-gnu"
        "-L${CMAKE_SYSROOT}/usr/lib/aarch64-linux-gnu"
        "-Wl,-rpath-link,${CMAKE_SYSROOT}/lib/aarch64-linux-gnu"
        "-Wl,-rpath-link,${CMAKE_SYSROOT}/usr/lib/aarch64-linux-gnu")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBNM REQUIRED IMPORTED_TARGET libnm)
list(APPEND PANEL_LIBS PkgConfig::LIBNM)

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
if(PANEL_OPTIONAL_HARDWARE)
    add_subdirectory(common)
    target_compile_definitions(${PROJECT_NAME} PRIVATE PANEL_ENABLE_SENSORS=1)
    list(APPEND PANEL_LIBS common)
endif()
if(PANEL_ENABLE_RPC)
    add_subdirectory(rpc)
    target_compile_definitions(${PROJECT_NAME} PRIVATE PANEL_ENABLE_RPC=1)
    list(APPEND PANEL_LIBS rpi_rpc)
endif()

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
# The core panel does not require the RPC server or optional sensors.
if (USE_SDL2)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DUSE_SDL2)
endif ()
list(APPEND PANEL_LIBS astra u8g2-arm-linux spdlog wiringPi)
target_link_libraries(${PROJECT_NAME} ${PANEL_LIBS})
