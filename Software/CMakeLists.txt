cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)

#------------------------------------------------------------------------------
# bin2c conversion tool

project(bin2c CXX)
add_executable(bin2c src/bin2c.cpp)

# This function makes sure that a list of binary files from the res
# subdirectory is packed into a C header file when the target is built
# First parameter: Name of the target
# Second parameter: Name of the header file
# Third and following parameters: Names of the binary files
function(add_binaries_to_target target header)
	set(dependencies)
	set(bin2c ${header})
	foreach(binary ${ARGN})
		list(APPEND dependencies ${PROJECT_SOURCE_DIR}/res/${binary})
		string(REPLACE . _ str ${binary})
		string(REPLACE / _ str ${str})
		list(APPEND bin2c ${PROJECT_SOURCE_DIR}/res/${binary} ${str})
	endforeach()
	add_custom_target(${header}
		COMMAND bin2c ${bin2c}
		DEPENDS bin2c ${dependencies}
	)
	add_dependencies(${target} ${header})
endfunction()

#------------------------------------------------------------------------------
# wxWidgets

# Ubuntu: Requires libwxgtk3.2-dev to be installed
find_package(wxWidgets REQUIRED core base xml OPTIONAL_COMPONENTS net)

#------------------------------------------------------------------------------
# HIDAPI

set(BUILD_SHARED_LIBS FALSE)
add_subdirectory(hidapi)

#------------------------------------------------------------------------------
# MacroPad (CLI)

project(MacroPad-cli CXX)

include(${wxWidgets_USE_FILE})
set(wxWidgets_USE_STATIC ON)

# Source code
add_executable(macropad-cli
	src/main.cpp
	src/hid.cpp
	src/xmlfile.cpp
)

target_include_directories(macropad-cli PUBLIC src)
target_include_directories(macropad-cli PUBLIC ../Firmware/src) # settings.h and usb_descriptors.h from firmware project
target_link_libraries(macropad-cli PRIVATE ${wxWidgets_LIBRARIES})
target_link_libraries(macropad-cli PRIVATE hidapi::hidapi)

#------------------------------------------------------------------------------
# MacroPad (GUI)

project(MacroPad CXX)

include(${wxWidgets_USE_FILE})
set(wxWidgets_USE_STATIC ON)

# Source code
add_executable(MacroPad
	src/app.cpp
	src/gui.cpp
	src/mainframe.cpp
	src/macropadwidget.cpp
	src/ctrlwidgets.cpp
	src/bmpwidgets.cpp
	src/templates.cpp
	src/hid.cpp
	src/keyboard.cpp
	src/xmlfile.cpp
)

# Resources
add_binaries_to_target(MacroPad images.h
	icon.png
	toolbar_new.png
	toolbar_load.png
	toolbar_save.png
	toolbar_scan.png
	toolbar_read.png
	toolbar_write.png
	toolbar_about.png
	macropad.png
	btn_add.png
	btn_remove.png
	btn_edit.png
	btn_moveup.png
	btn_movedown.png
	btn_rotateleft.png
	btn_rotateright.png
)
add_binaries_to_target(MacroPad profiletemplates.h
	templates/mouse.png
	templates/textedit.png
	templates/krita.png
	templates/xournalpp.png
)
add_binaries_to_target(MacroPad ctrltemplates.h
	templates/leftclick.png
	templates/rightclick.png
	templates/middleclick.png
	templates/mouseleft.png
	templates/mouseright.png
	templates/mouseup.png
	templates/mousedown.png
	templates/scrollup.png
	templates/scrolldown.png
	templates/arrowleft.png
	templates/arrowright.png
	templates/arrowup.png
	templates/arrowdown.png
	templates/cut.png
	templates/copy.png
	templates/paste.png
	templates/escape.png
	templates/cycle.png
	templates/undo.png
	templates/redo.png
	templates/enter.png
	templates/delete.png
	templates/tab.png
)

target_include_directories(MacroPad PUBLIC src ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(MacroPad PUBLIC ../Firmware/src) # settings.h and usb_descriptors.h from firmware project
target_link_libraries(MacroPad PRIVATE ${wxWidgets_LIBRARIES})
target_link_libraries(MacroPad PRIVATE hidapi::hidapi)

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
