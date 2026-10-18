cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(oled C CXX ASM)

set(CMAKE_C_STANDARD 11)

pico_sdk_init()

add_executable(oled
    src/main.c
    src/encoder.h
    src/encoder.c
    src/controller.h
    src/controller.c

    src/output/font.h
    src/output/font.c
    src/output/oled.h
    src/output/oled.c
    src/output/colors.h
    src/output/colors.c
    src/output/led_matrix.h
    src/output/led_matrix.c
)

# Compile the PIO assembly into src/output/ws2812.pio.h
# and make the generated header available to the target.
pico_generate_pio_header(oled
    ${CMAKE_CURRENT_LIST_DIR}/src/output/ws2812.pio
)

target_link_libraries(oled
    pico_stdlib
    hardware_pwm
    hardware_i2c
    hardware_dma
    hardware_pio
)

pico_add_extra_outputs(oled)

pico_enable_stdio_usb(oled 1)
pico_enable_stdio_uart(oled 0)
