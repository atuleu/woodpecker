cmake_minimum_required(VERSION 3.13)

set(FAMILY rp2040)
set(BOARD pico_sdk)

set(PICO_SDK_FETCH_FROM_GIT 1)
set(PICO_SDK_FETCH_FROM_GIT_TAG 2.2.0)

include(cmake/pico_sdk_import.cmake)

project(
	woodpecker_controller
	LANGUAGES C CXX
	VERSION 0.1.0
)

pico_sdk_init()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED 1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 1)

include(FetchContent)
set(PICO_UTILS_DIR ${PROJECT_SOURCE_DIR}/deps/rpi-pico-utils)

# if(EXISTS ${PICO_UTILS_DIR}) FetchContent_Declare(rpi-pico-utils SOURCE_DIR
# ${PICO_UTILS_DIR}) else() FetchContent_Declare( rpi-pico-utils GIT_REPOSITORY
# https://github.com/atuleu/rpi-pico-utils GIT_TAG main ) endif()

# FetchContent_MakeAvailable(rpi-pico-utils)

add_subdirectory(src)
