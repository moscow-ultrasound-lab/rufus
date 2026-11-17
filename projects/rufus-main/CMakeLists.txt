cmake_minimum_required(VERSION 3.23)

project(RUFUS)

if(MSVC)
	add_compile_options(/bigobj)
endif()

add_executable(RUFUS rufus-main.cpp	app.rc)

add_subdirectory(../rufus-b-mode ${CMAKE_BINARY_DIR}/rufus-b-mode)
add_subdirectory(../rufus-beamformer ${CMAKE_BINARY_DIR}/rufus-beamformer)
add_subdirectory(../rufus-doppler ${CMAKE_BINARY_DIR}/rufus-doppler)
add_subdirectory(../rufus-elasto ${CMAKE_BINARY_DIR}/rufus-elasto)
add_subdirectory(../rufus-phantom-generator ${CMAKE_BINARY_DIR}/rufus-phantom-generator)

set(RUFUS_CHILD_TARGETS
	b-mode
	beamformer
	doppler
	elasto
	phantom-generator
)

foreach(RUFUS_CHILD_TARGET IN LISTS RUFUS_CHILD_TARGETS)
	set_target_properties(${RUFUS_CHILD_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")
endforeach()

add_dependencies(RUFUS ${RUFUS_CHILD_TARGETS})