cmake_minimum_required(VERSION 3.23)

project(doppler)

if(MSVC)
	add_compile_options(/bigobj)
endif()

add_executable(doppler app.rc)

target_sources(doppler
	PRIVATE
		../rufus-common-sources/BBasics/BModeDetailedTest.cpp
		../rufus-common-sources/BBasics/RiceBasics.cpp
		../rufus-common-sources/DopplerBasics/Elasto/CFDataPhaseAnalyzerElasto.cpp
		../rufus-common-sources/DopplerBasics/CFDataPhaseAnalyzer.cpp
		../rufus-common-sources/DopplerBasics/CFM/CFDataPhaseAnalyzerCFM.cpp
		../rufus-common-sources/DopplerBasics/CFM/MaskCriteria.cpp
		../rufus-common-sources/DopplerBasics/CFM/WallFilters/WallFiltersInteractive.cpp
		../rufus-common-sources/DopplerBasics/CFM/WallFilters/EMD.cpp
		../rufus-common-sources/DopplerBasics/CFM/WallFilters/KarhunenLoeve.cpp
		../rufus-common-sources/DopplerBasics/CFM/WallFilters/WallFilters.cpp
		../rufus-common-sources/DopplerBasics/CFM/WallFilters/WallFiltersInterface.cpp
		../rufus-common-sources/DopplerBasics/FrameAcquisitionBuffer.cpp
		../rufus-common-sources/DopplerBasics/Utils/NeutralizeTissueMotion.cpp
		../rufus-common-sources/DopplerBasics/Utils/PhaseAnalyzerTimers.cpp
		../rufus-common-sources/DopplerBasics/Unknown/CalcTrueFalseDetectionRates.cpp
		../rufus-common-sources/DopplerBasics/Unknown/DopplerDataDisplayAdditional.cpp
		../rufus-common-sources/DopplerBasics/Display/ColorFlowPalettes.cpp
		../rufus-common-sources/DopplerBasics/Display/DuplexDisplayer.cpp
		../rufus-common-sources/DopplerBasics/Display/DisplayDopplerData.cpp
		../rufus-common-sources/DopplerBasics/CFDataAmplitudeAnalyzer.cpp
		../rufus-common-sources/RFDataImport/S500_CFMFrameSet.cpp
		../rufus-common-sources/RFDataImport/S500_CFMParamFileData.cpp
		../rufus-common-sources/RFDataImport/S500_CFMRawDataDisplay.cpp
		sources/CFMModesDetailedTest.cpp
		sources/DopplerTest.cpp
		sources/DummyHypothesis.cpp
		sources/SetOfPulses.cpp
		sources/SimulateSignal.cpp
		sources/pre.cpp
)

target_include_directories(doppler
    PRIVATE
		sources
		../rufus-common-sources
		$ENV{XRADRoot}
)

target_link_directories(doppler
    PRIVATE
		../../XRADInstall/lib/MSVC2019/x64-Debug
		$ENV{QTDIR5x64}/lib
		$ENV{QWTDIR5x64}/lib
)