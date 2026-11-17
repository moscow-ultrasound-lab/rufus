#include "pre.h"


#include <DopplerBasics/Elasto/CFDataPhaseAnalyzerElasto.h>
#include <DopplerBasics/CFM/CFDataPhaseAnalyzerCFM.h>
#include <DopplerBasics/CFDataAmplitudeAnalyzer.h>

#include <DopplerBasics/CFM/WallFilters/WallFiltersInteractive.h>

#include <DopplerBasics/Display/DisplayDopplerData.h>


XRAD_BEGIN


shared_ptr<CFDataPhaseAnalyzer>	ChooseCFDataProcessMode()
{
	auto	result = GetButtonDecision("Choose Analyzer",
	{
		MakeButton("Color Flow", static_cast<CFDataPhaseAnalyzer*>(new CFDataPhaseAnalyzerCFM)),
		MakeButton("Elasto", static_cast<CFDataPhaseAnalyzer*>(new CFDataPhaseAnalyzerElasto)),
		MakeButton("Amplitude overlay", static_cast<CFDataPhaseAnalyzer*>(new CFDataAmplitudeAnalyzer)),
		MakeButton("Exit", static_cast<CFDataPhaseAnalyzer*>(nullptr))
	});

	if(!result) throw canceled_operation("Analyzing canceled");

	return shared_ptr<CFDataPhaseAnalyzer>(result);
}

void	CFMModesDetailedTest(S500_CFMFrameSet &original_frame_set)
{
	shared_ptr<CFDataPhaseAnalyzer> analyzer = ChooseCFDataProcessMode();

	size_t	n_frames = original_frame_set.n_frames;

//	XRAD_ASSERT_THROW(n_frames == n_frames1);//TODO понять, почему там два разых параметра обозначают одно и то же

	size_t	n_cfm_beams = original_frame_set.n_cfm_beams();
	size_t	n_cfm_shots = original_frame_set.n_cfm_shots();
	size_t	n_cfm_samples = original_frame_set.n_cfm_samples();
	size_t	n_cfm_beams_in_sweep = original_frame_set.n_cfm_beams_in_sweep();
	size_t	n_cfm_sweeps = original_frame_set.n_cfm_sweeps();
	size_t	ray_header_size = original_frame_set.ray_header_size_in_bytes();

//	size_t	n_cfm_beams_in_sweep2 = original_frame_set.file_params.NumOfCFMBeams / original_frame_set.file_params.NumOfSweeps;

	printf("\nn_cfm_shots = %zu, n_beams_in_sweep = %zu, n_sweeps = %zu, ray_header = %zu bytes\n", n_cfm_shots, n_cfm_beams_in_sweep, n_cfm_sweeps, ray_header_size);

	XRAD_ASSERT_THROW(n_cfm_beams_in_sweep==n_cfm_beams_in_sweep);

	// Очень важная находка: попытка учитывать этот параметр в анализаторе дает ошибку. Видимо, остался рудимент, который долгое время превращался в 0
	ray_header_size = 0;//test

	analyzer->PrepareBuffers({ n_cfm_beams, n_cfm_shots, n_cfm_samples }, n_cfm_beams_in_sweep, ray_header_size);
	
	try
	{
		auto	&cfm_analyzer = dynamic_cast<CFDataPhaseAnalyzerCFM &>(*analyzer);
		cfm_analyzer.wall_filter = GetWallFilterInteractive(n_cfm_shots);
	}
	catch(bad_cast){}
	
	analyzer->result_axial_blur = GetFloating("Axial blur", SavedGUIValue(1), 0, 100); //1;
	analyzer->frame_agility_factor = GetFloating("Frame agility", SavedGUIValue(0.25), 0, 1);// 0.25; //1 означает отсутствие накопления; 0.25 умеренное накопление; 0.1 значительное накопление; значения [1;0).
	analyzer->tissue_motion_compensation_flag = YesOrNo("TissueMotionCompensation", true);
	index_vector	cfm_sizes({ n_frames, n_cfm_beams, n_cfm_samples });

	RealFunctionMD_F32 std_frames(cfm_sizes, 0);
	RealFunctionMD_F32 correlation_frames(cfm_sizes, 0);
	RealFunctionMD_F32 correlation_re_im_frames(cfm_sizes, 0);
	RealFunctionMD_F32 amplitude_frames(cfm_sizes, 0);
	RealFunctionMD_F32 phase_frames(cfm_sizes, 0);
	RealFunctionMD_F32 phase_frames_dispersion(cfm_sizes, 0);
	RealFunctionMD_F32 mask_frames(cfm_sizes, 0);
	RealFunctionMD_F32 phase_frames_normalized(cfm_sizes, 0);

	GUIProgressBar	progress;
	progress.start(analyzer->message(), n_frames);


	TimeProfiler	total_tp;
	TimeProfiler	control_tp;

	for (size_t frame_no = 0; frame_no < n_frames; ++frame_no)
	{
		total_tp.Start();
		auto	iv ={frame_no, slice_mask(0), slice_mask(1)};

		auto mask_slice = mask_frames.GetSlice(iv);
		auto phase_normalized_slice = phase_frames_normalized.GetSlice(iv);

		analyzer->ImportRawData(&original_frame_set.cfm_shot_frames.at({ frame_no, 0, 0, 0 }), false);

		control_tp.Start();
		analyzer->AnalyzeFrame(phase_normalized_slice, mask_slice);
		control_tp.Stop();

		std_frames.GetSlice(iv).CopyData(analyzer->stddev());
		correlation_frames.GetSlice(iv).CopyData(analyzer->correlation());
		correlation_re_im_frames.GetSlice(iv).CopyData(analyzer->correlation_re_im());
		amplitude_frames.GetSlice(iv).CopyData(analyzer->amplitude());
		phase_frames.GetSlice(iv).CopyData(analyzer->phase());// это ненормированная фаза, которая где-то отличается от нормированной
		phase_frames_dispersion.GetSlice(iv).CopyData(analyzer->phase().dispersion());

		total_tp.Stop();
		++progress;
	}
	progress.end();

	string	time_consumption = analyzer->GetTimeConsumptionReport();
	auto	sum = analyzer->GetProcessingTime();

	time_consumption += ssprintf("\nControl time = %g msec", control_tp.MeanElapsed().msec());
	time_consumption += ssprintf("\nTotal time = %g msec, difference = %g msec", total_tp.MeanElapsed().msec(), (total_tp.MeanElapsed() - sum).msec());

	DisplayDopplerData(phase_frames, 
					phase_frames_dispersion, 
					phase_frames_normalized, 
					mask_frames, 
					correlation_frames, 
					correlation_re_im_frames, 
					std_frames, 
					amplitude_frames, 
					original_frame_set,
					time_consumption
					);
}

XRAD_END