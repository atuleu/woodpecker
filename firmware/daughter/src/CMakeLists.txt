set(COMMON_FILES
	clock.h #
	uart.h #
	uart.c
)

add_avr_library(common ${COMMON_FILES})

add_avr_executable(top_section main-top.c)
avr_target_link_libraries(top_section common)
add_pymcuprog_upload_target(TARGET top_section)

add_avr_executable(middle_section main-middle.c)
avr_target_link_libraries(middle_section common)
add_pymcuprog_upload_target(TARGET middle_section)
