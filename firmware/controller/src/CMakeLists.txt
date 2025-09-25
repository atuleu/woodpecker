add_executable(woodpecker-controller main.c)

target_link_libraries(woodpecker-controller rpi-pico-utils)

pico_add_extra_outputs(woodpecker-controller)

pico_enable_stdio_uart(woodpecker-controller ENABLED)
pico_enable_stdio_usb(woodpecker-controller DISABLED)
