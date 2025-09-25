set(SRC_FILES
	main.c #
	tusb_config.h #
	usb_descriptor.c #
	lwipopts.h
	${PICO_TINYUSB_PATH}/lib/networking/rndis_report.c
)

add_executable(woodpecker-controller main.c)

target_link_libraries(
	woodpecker-controller
	pico_stdlib
	tinyusb_device
	pico_lwip
	pico_lwip_nosys
	pico_lwip_mdns
	pico_unique_id
)

target_compile_definitions(woodpecker-controller PUBLIC -DUSE_ECM=1)
target_include_directories(
	woodpecker-controller PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
								  ${PICO_TINYUSB_PATH}/lib/networking
)
pico_add_extra_outputs(woodpecker-controller)

pico_enable_stdio_uart(woodpecker-controller ENABLED)
pico_enable_stdio_usb(woodpecker-controller 0)
