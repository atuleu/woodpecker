cmake_minimum_required(VERSION 3.13)

include(../pico-sdk/pico_sdk_init.cmake)

project(TestBoard C CXX ASM)

pico_sdk_init()

add_executable(hello_world hello_world.c display.cpp key.cpp)

pico_generate_pio_header(hello_world ${CMAKE_CURRENT_LIST_DIR}/tlc5973.pio)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(hello_world pico_stdlib hardware_pio hardware_spi)

# enable usb output, disable uart output
pico_enable_stdio_usb(hello_world 1)
pico_enable_stdio_uart(hello_world 0)


# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(hello_world)
