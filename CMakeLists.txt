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

include_directories(${CMAKE_SOURCE_DIR})

add_executable(${PROJECT_NAME} panel/main.cpp
        panel/astra_logo.cpp
        panel/astra_rocket.cpp
        sh1106_hal/sh1106_hal.cpp
        sh1106_hal/sh1106_hal.h
)

target_link_libraries(${PROJECT_NAME} astra u8g2-arm-linux periphery)