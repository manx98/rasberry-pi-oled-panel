# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(wiringPi)

# 获取版本号
file(READ "VERSION" VERSION)
string(STRIP "${VERSION}" VERSION)

# 设置链接库
set(LIBS m pthread rt)

# 设置源文件
set(SRC
        wiringPi.c
        wiringSerial.c
        wiringShift.c
        piHiPri.c
        piThread.c
        wiringPiSPI.c
        wiringPiI2C.c
        softPwm.c
        softTone.c
        mcp23008.c
        mcp23016.c
        mcp23017.c
        mcp23s08.c
        mcp23s17.c
        sr595.c
        pcf8574.c
        pcf8591.c
        mcp3002.c
        mcp3004.c
        mcp4802.c
        mcp3422.c
        max31855.c
        max5322.c
        ads1115.c
        sn3218.c
        bmp180.c
        htu21d.c
        ds18b20.c
        rht03.c
        drcSerial.c
        drcNet.c
        pseudoPins.c
        wpiExtensions.c
        wiringPiLegacy.c
)

# 获取头文件
file(GLOB HEADERS include/wiringPi/*.h)

# 编译动态库
add_library(wiringPi STATIC ${SRC})
include_directories(include/wiringPi)
target_include_directories(wiringPi PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(wiringPi PROPERTIES
        OUTPUT_NAME "wiringPi"
        VERSION ${VERSION}
        SOVERSION ${VERSION}
)
target_compile_options(wiringPi PRIVATE ${CFLAGS})
target_link_libraries(wiringPi PRIVATE ${LIBS})

# 安装目标
install(TARGETS wiringPi
        LIBRARY DESTINATION ${DESTDIR}${PREFIX}/lib
)
install(FILES ${HEADERS}
        DESTINATION ${DESTDIR}${PREFIX}/include
)