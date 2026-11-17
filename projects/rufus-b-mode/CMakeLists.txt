cmake_minimum_required(VERSION 3.23)

project(b-mode)

if(MSVC)
	add_compile_options(/bigobj)
endif()

add_executable(b-mode app.rc)

target_sources(b-mode
	PRIVATE
		../rufus-common-sources/DopplerBasics/CFDataPhaseAnalyzer.cpp
		../rufus-common-sources/DopplerBasics/CFDataAmplitudeAnalyzer.cpp
		../rufus-common-sources/DopplerBasics/CFM/CFDataPhaseAnalyzerCFM.cpp
		../rufus-common-sources/DopplerBasics/CFM/MaskCriteria.cpp
		../rufus-common-sources/DopplerBasics/Elasto/CFDataPhaseAnalyzerElasto.cpp
		../rufus-common-sources/DopplerBasics/CFM/WallFilters/EMD.cpp
		../rufus-common-sources/DopplerBasics/CFM/WallFilters/KarhunenLoeve.cpp
		../rufus-common-sources/DopplerBasics/CFM/WallFilters/WallFilters.cpp
		../rufus-common-sources/DopplerBasics/CFM/WallFilters/WallFiltersInterface.cpp
		../rufus-common-sources/DopplerBasics/CFM/WallFilters/WallFiltersInteractive.cpp
		../rufus-common-sources/DopplerBasics/FrameAcquisitionBuffer.cpp
		../rufus-common-sources/DopplerBasics/Utils/NeutralizeTissueMotion.cpp
		../rufus-common-sources/DopplerBasics/Utils/PhaseAnalyzerTimers.cpp
		../rufus-common-sources/DopplerBasics/Unknown/CalcTrueFalseDetectionRates.cpp
		../rufus-common-sources/DopplerBasics/Unknown/DopplerDataDisplayAdditional.cpp
		../rufus-common-sources/DopplerBasics/Display/ColorFlowPalettes.cpp
		../rufus-common-sources/DopplerBasics/Display/DuplexDisplayer.cpp
		../rufus-common-sources/DopplerBasics/Display/DisplayDopplerData.cpp
		../rufus-common-sources/RFDataImport/S500_CFMFrameSet.cpp
		../rufus-common-sources/RFDataImport/S500_CFMParamFileData.cpp
		../rufus-common-sources/RFDataImport/S500_CFMRawDataDisplay.cpp
		sources/b-mode.cpp
		sources/CFMModesDetailedTest.cpp
		sources/DummyHypothesis.cpp
		sources/OASBUDFrameSet.cpp
		sources/pre.cpp
		sources/SetOfPulses.cpp
		sources/SimulateSignal.cpp
)

target_include_directories(b-mode
    PRIVATE
		sources
		../rufus-common-sources
		$ENV{XRADRoot}
)
target_link_directories(b-mode
    PRIVATE
		../../XRADInstall/lib/MSVC2019/x64-Debug
		$ENV{QTDIR5x64}/lib
		$ENV{QWTDIR5x64}/lib
)