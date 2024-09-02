project(u8g2-arm-linux-example)

set(C_EXAMPLES
        u8g2_4wire_hw_spi
        u8g2_4wire_sw_spi
        u8g2_hw_i2c
        u8g2_sw_i2c
        u8g2_sw_i2c_thread
)

set(CPP_EXAMPLES
        u8g2_4wire_hw_spi_cpp
        u8g2_4wire_sw_spi_cpp
        u8g2_hw_i2c_cpp
        u8g2_sw_i2c_cpp
        u8g2_sw_i2c_thread_cpp
)

foreach (example ${C_EXAMPLES})
    aux_source_directory(c-examples/${example} ${example}_SRC)
    add_executable(${example} ${${example}_SRC})
    target_link_libraries(${example} u8g2-arm-linux periphery)
endforeach ()

foreach (example ${CPP_EXAMPLES})
    aux_source_directory(cpp-examples/${example} ${example}_SRC)
    add_executable(${example} ${${example}_SRC})
    target_link_libraries(${example} u8g2-arm-linux periphery)
endforeach ()