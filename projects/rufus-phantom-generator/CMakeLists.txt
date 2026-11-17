cmake_minimum_required(VERSION 3.23)

project(phantom-generator)

if(MSVC)
    add_compile_options(/bigobj)
endif()

add_executable(phantom-generator app.rc)

target_sources(phantom-generator
	PRIVATE
		sources/CreateVolumes.cpp
		sources/GenerateFigures.cpp
		sources/TestFiles.cpp
        sources/pre.cpp
)

target_include_directories(phantom-generator
    PRIVATE
        $ENV{XRADRoot}
)

target_link_directories(phantom-generator
    PRIVATE
        ../../XRADInstall/lib/MSVC2019/x64-Debug
        $ENV{QTDIR5x64}/lib
        $ENV{QWTDIR5x64}/lib
)