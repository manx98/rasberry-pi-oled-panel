project(u8g2-arm-linux)

include_directories(include)
cmake_minimum_required(VERSION 3.5)

set(COMPONENT_ADD_INCLUDEDIRS csrc)
list(APPEND SRC
        csrc/mui.c
        csrc/mui_u8g2.c
        csrc/u8g2_arc.c
        csrc/u8g2_bitmap.c
        csrc/u8g2_box.c
        csrc/u8g2_buffer.c
        csrc/u8g2_button.c
        csrc/u8g2_circle.c
        csrc/u8g2_cleardisplay.c
        csrc/u8g2_d_memory.c
        csrc/u8g2_d_setup.c
        csrc/u8g2_font.c
        csrc/u8g2_fonts.c
        csrc/u8g2_hvline.c
        csrc/u8g2_input_value.c
        csrc/u8g2_intersection.c
        csrc/u8g2_kerning.c
        csrc/u8g2_line.c
        csrc/u8g2_ll_hvline.c
        csrc/u8g2_message.c
        csrc/u8g2_polygon.c
        csrc/u8g2_selection_list.c
        csrc/u8g2_setup.c
        csrc/u8log.c
        csrc/u8log_u8g2.c
        csrc/u8log_u8x8.c
        csrc/u8x8_8x8.c
        csrc/u8x8_byte.c
        csrc/u8x8_cad.c
        csrc/u8x8_capture.c
        csrc/u8x8_d_a2printer.c
        csrc/u8x8_d_ch1120.c
        csrc/u8x8_d_gp1247ai.c
        csrc/u8x8_d_gp1287ai.c
        csrc/u8x8_d_gp1294ai.c
        csrc/u8x8_d_gu800.c
        csrc/u8x8_d_hd44102.c
        csrc/u8x8_d_il3820_296x128.c
        csrc/u8x8_d_ist3020.c
        csrc/u8x8_d_ist3088.c
        csrc/u8x8_d_ist7920.c
        csrc/u8x8_d_ks0108.c
        csrc/u8x8_d_lc7981.c
        csrc/u8x8_d_ld7032_60x32.c
        csrc/u8x8_d_ls013b7dh03.c
        csrc/u8x8_d_max7219.c
        csrc/u8x8_d_pcd8544_84x48.c
        csrc/u8x8_d_pcf8812.c
        csrc/u8x8_d_pcf8814_hx1230.c
        csrc/u8x8_d_s1d15e06.c
        csrc/u8x8_d_s1d15300.c
        csrc/u8x8_d_s1d15721.c
        csrc/u8x8_d_sbn1661.c
        csrc/u8x8_d_sed1330.c
        csrc/u8x8_d_sh1106_64x32.c
        csrc/u8x8_d_sh1106_72x40.c
        csrc/u8x8_d_sh1107.c
        csrc/u8x8_d_sh1108.c
        csrc/u8x8_d_sh1122.c
        csrc/u8x8_d_ssd1305.c
        csrc/u8x8_d_ssd1306_48x64.c
        csrc/u8x8_d_ssd1306_64x32.c
        csrc/u8x8_d_ssd1306_64x48.c
        csrc/u8x8_d_ssd1306_72x40.c
        csrc/u8x8_d_ssd1306_96x16.c
        csrc/u8x8_d_ssd1306_96x40.c
        csrc/u8x8_d_ssd1306_128x32.c
        csrc/u8x8_d_ssd1306_128x64_noname.c
        csrc/u8x8_d_ssd1306_2040x16.c
        csrc/u8x8_d_ssd1309.c
        csrc/u8x8_d_ssd1312.c
        csrc/u8x8_d_ssd1316.c
        csrc/u8x8_d_ssd1317.c
        csrc/u8x8_d_ssd1318.c
        csrc/u8x8_d_ssd1320.c
        csrc/u8x8_d_ssd1322.c
        csrc/u8x8_d_ssd1325.c
        csrc/u8x8_d_ssd1326.c
        csrc/u8x8_d_ssd1327.c
        csrc/u8x8_d_ssd1329.c
        csrc/u8x8_d_ssd1362.c
        csrc/u8x8_d_ssd1363.c
        csrc/u8x8_d_ssd1606_172x72.c
        csrc/u8x8_d_ssd1607_200x200.c
        csrc/u8x8_d_st7302.c
        csrc/u8x8_d_st7511.c
        csrc/u8x8_d_st7528.c
        csrc/u8x8_d_st7539.c
        csrc/u8x8_d_st7565.c
        csrc/u8x8_d_st7567.c
        csrc/u8x8_d_st7571.c
        csrc/u8x8_d_st7586s_erc240160.c
        csrc/u8x8_d_st7586s_jlx320160.c
        csrc/u8x8_d_st7586s_jlx384160.c
        csrc/u8x8_d_st7586s_md240128.c
        csrc/u8x8_d_st7586s_s028hn118a.c
        csrc/u8x8_d_st7586s_ymc240160.c
        csrc/u8x8_d_st7588.c
        csrc/u8x8_d_st7920.c
        csrc/u8x8_d_st75160.c
        csrc/u8x8_d_st75161.c
        csrc/u8x8_d_st75256.c
        csrc/u8x8_d_st75320.c
        csrc/u8x8_d_stdio.c
        csrc/u8x8_d_t6963.c
        csrc/u8x8_d_uc1601.c
        csrc/u8x8_d_uc1604.c
        csrc/u8x8_d_uc1608.c
        csrc/u8x8_d_uc1609.c
        csrc/u8x8_d_uc1610.c
        csrc/u8x8_d_uc1611.c
        csrc/u8x8_d_uc1617.c
        csrc/u8x8_d_uc1628.c
        csrc/u8x8_d_uc1638.c
        csrc/u8x8_d_uc1701_dogs102.c
        csrc/u8x8_d_uc1701_mini12864.c
        csrc/u8x8_debounce.c
        csrc/u8x8_display.c
        csrc/u8x8_fonts.c
        csrc/u8x8_gpio.c
        csrc/u8x8_input_value.c
        csrc/u8x8_message.c
        csrc/u8x8_selection_list.c
        csrc/u8x8_setup.c
        csrc/u8x8_string.c
        csrc/u8x8_u8toa.c
        csrc/u8x8_u16toa.c
        csrc/u8x8_d_sdl_128x64.c
        csrc/u8x8_sdl_key.c
        port/u8g2port.c
        cppsrc/Print.cpp
        cppsrc/U8g2lib.cpp
        cppsrc/U8x8lib.cpp
        include/mui.h
        include/mui_u8g2.h
        include/MUIU8g2.h
        include/Print.h
        include/Printable.h
        include/u8g2.h
        include/U8g2lib.h
        include/u8g2port.h
        include/u8x8.h
        include/U8x8lib.h
)
add_library(${PROJECT_NAME} STATIC ${SRC})
set(LIBS wiringPi)
if (SDL2_FOUND)
    list(APPEND LIBS ${SDL2_LIBRARIES})
else ()
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DNO_SDL)
endif ()
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/3rdparty/wiringPi/include)
target_link_libraries(${PROJECT_NAME} ${LIBS})
add_subdirectory(examples)