set(SRC_FILES main.c config.h)

add_avr_executable(top_section ${SRC_FILES})
target_compile_definitions(top_section-attiny1617.elf PUBLIC -DTOP_SECTION=1)
add_pymcuprog_upload_target(TARGET top_section)

add_avr_executable(middle_section ${SRC_FILES})
target_compile_definitions(
	middle_section-attiny1617.elf PUBLIC -DMIDDLE_SECTION=1
)
add_pymcuprog_upload_target(TARGET middle_section)
