set(PYMCUPROG_PORT
	/dev/ttyUSB0
	CACHE STRING "Port to use with PYMCUPROG"
)
find_program(PYMCUPROG_EXECUTABLE pymcuprog REQUIRED)
set(PYMCUPROG_OPTIONS -d ${AVR_MCU} -t uart -u ${PYMCUPROG_PORT})

function(add_pymcuprog_upload_target)
	set(options)
	set(oneValueArgs TARGET)
	set(multiValueArgs)
	cmake_parse_arguments(
		PARSE_ARGV 0 arg "${options}" "${oneValueArgs}" "${multiValueArgs}"
	)

	if(NOT arg_TARGET)
		message(FATAL_ERROR "TARGET is required")
	endif(NOT arg_TARGET)

	add_custom_target(
		${arg_TARGET}-ping ${PYMCUPROG_EXECUTABLE} ${PYMCUPROG_OPTIONS} ping
	)
	add_custom_target(
		${arg_TARGET}-upload
		${PYMCUPROG_EXECUTABLE}
		${PYMCUPROG_OPTIONS}
		write
		-f
		${TARGET}-${AVR_MCU}.hex
		--erase
		--verify
		DEPENDS ${TARGET}-${AVR_MCU}}.hex
	)

endfunction(add_pymcuprog_upload_target)
