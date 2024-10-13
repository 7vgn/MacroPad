cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_PATH ${CMAKE_SOURCE_DIR}/pico-sdk)
include(pico-sdk/external/pico_sdk_import.cmake)

set(PROJECT_NAME MacroPad)
set(PICO_BOARD pico)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

project(${PROJECT_NAME} C CXX ASM)

pico_sdk_init()

add_executable(${PROJECT_NAME}
	src/main.cpp
	src/input.cpp
	src/settingstools.cpp
	src/eeprom.cpp
	src/hid.cpp
	src/usb_descriptors.cpp
	src/display.cpp
	src/font.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC src)

# Serial output via UART since USB is used for other purposes
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

target_link_libraries(${PROJECT_NAME}
	pico_stdlib
	pico_multicore
	pico_unique_id
	tinyusb_device
	hardware_spi
	hardware_i2c
	hardware_adc
)

# Create .uf2 file for flashing in USB boot mode
pico_add_extra_outputs(${PROJECT_NAME})

#------------------------------------------------------------------------------
# Flashing via SWD

# This section only concern flashing via SWD. If you are flashing the .uf2 file
# via USB, you can ignore it.

# Add a target for flashing via SWD (using the Pico Debug Probe)
# Requires OpenOCD to be set up and, if necessary, patched for the specific
# flash chip. See BuildingInstructions.pdf for details.
add_custom_target(flash sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program ${PROJECT_NAME}.elf verify reset exit"
	DEPENDS ${PROJECT_NAME}
)

#------------------------------------------------------------------------------
# Doxygen

find_package(Doxygen)
if(DOXYGEN_FOUND)
	set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/src/Doxyfile.in)
	set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
	configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
	add_custom_target(doc ALL
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
else(DOXYGEN_FOUND)
	message("Doxygen not found. API documentation will not be built.")
endif(DOXYGEN_FOUND)
