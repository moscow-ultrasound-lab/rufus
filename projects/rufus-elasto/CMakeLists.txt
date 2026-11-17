cmake_minimum_required(VERSION 3.23)

project(elasto)

if(MSVC)
	add_compile_options(/bigobj)
	add_compile_definitions(__DLL__)
endif()

add_executable(elasto app.rc)

target_sources(elasto
	PRIVATE
		../rufus-common-sources/DopplerBasics/CFM/MaskCriteria.cpp
		../rufus-common-sources/DopplerBasics/Utils/NeutralizeTissueMotion.cpp
		../rufus-common-sources/DopplerBasics/CFDataPhaseAnalyzer.cpp
		../rufus-common-sources/DopplerBasics/CFM/CFDataPhaseAnalyzerCFM.cpp
		../rufus-common-sources/DopplerBasics/Elasto/CFDataPhaseAnalyzerElasto.cpp
		../rufus-common-sources/DopplerBasics/CFM/WallFilters/EMD.cpp
		../rufus-common-sources/DopplerBasics/FrameAcquisitionBuffer.cpp
		../rufus-common-sources/DopplerBasics/CFM/WallFilters/KarhunenLoeve.cpp
		../rufus-common-sources/DopplerBasics/Utils/PhaseAnalyzerTimers.cpp
		../rufus-common-sources/DopplerBasics/CFM/WallFilters/WallFilters.cpp
		../rufus-common-sources/DopplerBasics/CFM/WallFilters/WallFiltersInterface.cpp
		../rufus-common-sources/DopplerBasics/Display/DuplexDisplayer.cpp
		../rufus-common-sources/DopplerBasics/Display/ColorFlowPalettes.cpp
		../rufus-common-sources/RFDataImport/S500_CFMFrameSet.cpp
		../rufus-common-sources/RFDataImport/S500_CFMParamFileData.cpp
		../rufus-common-sources/RFDataImport/S500_CFMRawDataDisplay.cpp
		pre.cpp
		sources/AnalyzeCFMFrameSet.cpp
		sources/CFMParamFileData.cpp
		sources/ElastoGrafica.cpp
		sources/TestElastoFiles.cpp
)

target_include_directories(elasto
    PRIVATE
		sources
		./
		../rufus-common-sources
		$ENV{XRADRoot}
)

target_link_directories(elasto
    PRIVATE
		../../XRADInstall/lib/MSVC2019/x64-Debug
		$ENV{QTDIR5x64}/lib
		$ENV{QWTDIR5x64}/lib
)