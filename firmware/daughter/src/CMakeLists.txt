set(COMMON_FILES
	clock.h #
	uart.h #
	uart.c #
	debouncer.c
	debouncer.h
)

add_avr_library(common ${COMMON_FILES})

add_avr_executable(top_section main-top.c encoder.h encoder.c)
avr_target_link_libraries(top_section common)
add_pymcuprog_upload_target(TARGET top_section)

add_avr_executable(middle_section main-middle.c fader.h fader.c adc.h adc.c)
avr_target_link_libraries(middle_section common)
add_pymcuprog_upload_target(TARGET middle_section)
