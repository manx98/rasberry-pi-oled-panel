add_library(common STATIC
        BMP280.cpp
        BMP280.h
        fan.cpp
        fan.h
        i2c.cpp
        i2c.h
        INA226.cpp
        INA226.h
        relay.cpp
        relay.h
        sensor.cpp
        sensor.h
        SGP30.cpp
        SGP30.h
        sht3x.cpp
        sht3x.h
        spi.cpp
        spi.h
        utils.cpp
        utils.h
        log.h
        log.cpp
)

target_link_libraries(common PUBLIC spdlog)