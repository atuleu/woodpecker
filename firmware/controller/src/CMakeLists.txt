set(SRC_FILES
	main.c #
	tusb_config.h #
	usb_descriptors.c #
	netusb.c #
	netusb.h #
	lwipopts.h
	${PICO_TINYUSB_PATH}/lib/networking/rndis_reports.c
	${PICO_TINYUSB_PATH}/lib/networking/dhserver.c
	osc.h
	osc.c
	lp5864.h
	lp5864.c
	dma_shared_irq.h
	dma_shared_irq.c
	i2c_dma.h
	i2c_dma.c
	section_rx.h
	section_rx.c
	hid.h
	hid.c
	atomic.h
	encoder.h
	encoder.c
	hid_visual.c
	hid_visual.h
	ctime_prob.h
	ctime_prob.cpp
)

add_executable(woodpecker-controller ${SRC_FILES})

target_link_libraries(
	woodpecker-controller
	PRIVATE pico_stdlib
			tinyusb_device
			pico_lwip
			pico_lwip_nosys
			pico_lwip_mdns
			pico_lwip_http
			pico_unique_id
			pico_httpd_content
			pico_util
			hardware_i2c
			hardware_dma
			hardware_pio
			pico_multicore
)
pico_generate_pio_header(
	woodpecker-controller ${CMAKE_CURRENT_SOURCE_DIR}/uart_rx.pio
)
target_compile_definitions(woodpecker-controller PUBLIC -DUSE_ECM=1)

target_include_directories(
	woodpecker-controller
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} #
			${PICO_TINYUSB_PATH}/lib/networking
			${PICO_LWIP_CONTRIB_PATH}/apps/httpd
)

pico_add_extra_outputs(woodpecker-controller)

pico_add_library(pico_httpd_content NOFLAG)
pico_set_lwip_httpd_content(
	pico_httpd_content INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/content/index.html
)
pico_enable_stdio_uart(woodpecker-controller ENABLED)
pico_enable_stdio_usb(woodpecker-controller 0)
add_openocd_upload_target(TARGET woodpecker-controller)
