#include "pre.h"


#include <RFDataImport/S500_CFMFrameSet.h>
#include <RFDataImport/S500_CFMRawDataDisplay.h>

#include <DopplerBasics/CFM/WallFilters/WallFiltersInteractive.h>

// тесты универсальных методов доплерографии
#include "CFMModesDetailedTest.h"

// локальные тесты тонкостей алгоритмов, довольно беспорядочные
#include "SetOfPulses.h"
#include "DummyHypothesis.h"
#include "SimulateSignal.h"
//#include "BModeDetailedTest.h"
#include <Q:\projects\CommonSources\BBasics\BModeDetailedTest.h>
//#include <Q:\projects\DopplerTest\DopplerTest\sources\BModeDetailedTest.h>

XRAD_USING;

void	ChooseCFAnalysisTask(S500_CFMFrameSet &frames)
{
	size_t	frame_no = frames.n_frames / 2;
	enum 
	{ 
		cfm_modes_detailed_test, 
		b_mode_detailed_test,
//		find_blur_1d, 
//		find_blur_2d, 
		set_of_pulses, 
		exit, 
		n_options 
	} option;
	try
	{
		while(true)
		{
			option = GetButtonDecision("Choose CF analysis task",
			{
				MakeButton("CFM-modes detailed test",		cfm_modes_detailed_test),
				MakeButton("B-mode detailed test",		b_mode_detailed_test),
	//			MakeButton("Find Blur 1D", 	find_blur_1d),
	//			MakeButton("Find Blur 2D", 	find_blur_2d),
				MakeButton("Set of pulses", set_of_pulses),
				MakeButton("End with this file",			exit)
			});

			switch(option)
			{
				case cfm_modes_detailed_test:
					CFMModesDetailedTest(frames);
					break;
				case b_mode_detailed_test:
					BModeDetailedTest(frames);
					break;
	// 			case find_blur_1d:
	// 				CFMMode_find_blur_1D(frames);
	// 				break;
	// 			case find_blur_2d:
	// 				CFMMode_find_blur_2D(frames);
	// 				break;
				case set_of_pulses:
					frame_no = GetUnsigned("CFM-frame to display", MakeGUIValue(frames.n_frames / 2, saved_default_value), 0, frames.n_frames - 1);
					DisplayASetOfPulses(frames, frame_no);
					break;
				case exit:
					throw canceled_operation("Exit button");
					break;
			}
		}
	}
	catch(canceled_operation){}
	catch(quit_application){ throw; }
	catch(...) { Error(GetExceptionString()); }
}

void RawDataProcessingMenu(S500_CFMFrameSet &frames)
{
	try
	{
		enum
		{
			e_display,
			e_analyze, 
			e_exit
		};
		while (true)
		{
			auto answer = Decide(L"Choose option", 
				{ 
				MakeButton(L"Display", e_display),
				MakeButton(L"Analyze",  e_analyze), 
				MakeButton("End with this file", e_exit)
			});

			switch (answer)
			{
			case e_display:
				DisplayCFMFrameSet(frames, L"CFM frames");
				break;
			case e_analyze:
				ChooseCFAnalysisTask(frames);
				break;
			default:
				throw canceled_operation("End with this file");
			}
		}
	}
	catch (canceled_operation &) {}
	catch (quit_application &ex) { throw ex; }
	catch(...) { Error(GetExceptionString()); }
}


S500_CFMParamFileData	GetS500FileParams()
{
	wstring	filename = GetFileNameRead(L"Choose data files", saved_default_value, L"*.par");
	S500_CFMParamFileData	params;
	params.Init(filename);
	return params;
}

S500_CFMFrameSet GetS500Frames()
{
	S500_CFMParamFileData	params = GetS500FileParams();
	size_t	start_frame = 0;
	size_t	end_frame = params.NumOfFrames - 1;
	end_frame = GetUnsigned("How many frames to read?", MakeGUIValue(params.NumOfFrames, saved_default_value), 1, params.NumOfFrames) - 1;
	S500_CFMFrameSet	frames(params, start_frame, end_frame);

	bool unsweep = !CapsLock() ? true : YesOrNo("Unsweep data?", true);

	frames.ReadAllFrames(unsweep);

	return frames;
}

S500_CFMFrameSet InitLargeDataset(S500_CFMParamFileData params, S500_CFMParamFileData &buffer_params)
{
	buffer_params.NumOfBBeams = 300;
	buffer_params.SizeofBBeamAtSamples = 1000;
	buffer_params.NumOfCFMBeams = 200;
	buffer_params.SizeofCFMBeamAtSamples = 500;
	buffer_params.NumOfFrames = GetUnsigned("How many frames there are in a cumulative dataset?", MakeGUIValue(params.NumOfFrames, saved_default_value), 1, 10000) - 1;
	S500_CFMFrameSet	buffer_frames(buffer_params, 0, buffer_params.NumOfFrames - 1);
	buffer_frames.b_frames.fill(complexF32(1));
	buffer_frames.cfm_frames.fill(complexF32(1));
	return buffer_frames;
}

void AddDataset(size_t local_end_frame, size_t &last_frame_no, S500_CFMFrameSet local_frames, S500_CFMFrameSet &buffer_frames, S500_CFMParamFileData local_params)
{
//	DisplayMathFunction3D(local_frames.b_frames, ssprintf("b_frames original"));
//	DisplayMathFunction3D(local_frames.cfm_frames, ssprintf("cfm_frames original"));

	for (size_t local_frame_no = 0; local_frame_no < (local_end_frame + 1); ++local_frame_no)
	{
		auto local_slice_b = local_frames.b_frames.GetSlice({ local_frame_no, slice_mask(0), slice_mask(1) });
		auto buffer_slice_b = buffer_frames.b_frames.GetSlice({ last_frame_no + local_frame_no, slice_mask(0), slice_mask(1) });

	//	DisplayMathFunction2D(local_slice_b, "local_slice_b");
		
		for (size_t local_beam_no = 0; local_beam_no < local_params.NumOfBBeams; ++local_beam_no)
		{
			for (size_t local_sample_no = 0; local_sample_no < local_params.SizeofBBeamAtSamples; ++local_sample_no)
			{
				buffer_slice_b.at(local_beam_no, local_sample_no) = local_slice_b.at(local_beam_no, local_sample_no);
			}
		}

	//	DisplayMathFunction2D(buffer_slice_b, "buffer_slice_b");

	//	DisplayMathFunction3D(buffer_frames.b_frames, ssprintf("b_frames"));

		auto local_slice_cfm = local_frames.cfm_frames.GetSlice({ local_frame_no, slice_mask(0), slice_mask(1) });
		auto buffer_slice_cfm = buffer_frames.cfm_frames.GetSlice({ last_frame_no + local_frame_no, slice_mask(0), slice_mask(1) });

		for (size_t local_beam_no = 0; local_beam_no < (local_params.NumOfCFMBeams * local_params.NumOfCFShots); ++local_beam_no)
		{
			for (size_t local_sample_no = 0; local_sample_no < local_params.SizeofCFMBeamAtSamples; ++local_sample_no)
			{
				//buffer_slice_cfm.at(local_sample_no, local_beam_no) = local_slice_cfm.at(local_sample_no, local_beam_no);
				buffer_slice_cfm.at(local_beam_no, local_sample_no) = local_slice_cfm.at(local_beam_no, local_sample_no);
			}
		}
	}
	last_frame_no += local_end_frame + 1;

//	DisplayMathFunction3D(buffer_frames.b_frames, ssprintf("b_frames"));
//	DisplayMathFunction3D(buffer_frames.cfm_frames, ssprintf("cfm_frames"));


}

S500_CFMFrameSet LoadSeveralDatasets()
{
	S500_CFMParamFileData	params = GetS500FileParams();
	size_t end_frame = GetUnsigned("How many frames to read?", MakeGUIValue(params.NumOfFrames, saved_default_value), 1, params.NumOfFrames) - 1;
	S500_CFMFrameSet	frames(params, 0, end_frame);
	bool unsweep = !CapsLock() ? true : YesOrNo("Unsweep data?", true);
	frames.ReadAllFrames(unsweep);
	size_t last_frame_no(0);	
	S500_CFMParamFileData	buffer_params(params);
	S500_CFMFrameSet	buffer_frames = InitLargeDataset(params, buffer_params);
	AddDataset(end_frame, last_frame_no, frames, buffer_frames, params);

	while (YesOrNo("Read another dataset?", true))
	{
		S500_CFMParamFileData	local_params = GetS500FileParams();
		size_t local_end_frame = GetUnsigned("How many frames to read?", MakeGUIValue(local_params.NumOfFrames, saved_default_value), 1, local_params.NumOfFrames) - 1;
		S500_CFMFrameSet	local_frames(local_params, 0, local_end_frame);
		local_frames.ReadAllFrames(unsweep);
		AddDataset( local_end_frame, last_frame_no, local_frames, buffer_frames, local_params);
	}
	return buffer_frames;
}

void	WallFilterFrequencyResponse()
{
	size_t N = GetUnsigned("Burst Size", SavedGUIValue(16), 4, 1024);
	ComplexFunctionF32 pulse(N, complexF32(0));

	while(true)
	{
		pulse[pulse.size() / 2] = complexF32(1);
		ComplexFunctionF32	filtered(pulse);

		WallFilterPtr	wall_filter = GetWallFilterInteractive(pulse.size());
		if(wall_filter->filter_name()==L"WallFilterNone") break;

		wall_filter->Apply(filtered);

		while(true)
		{
			auto	fn = GetButtonDecision(
				L"Display option",
				{
					MakeButton(L"Original pulse", make_fn([&pulse](){DisplayMathFunction(pulse, 0, 1, L"Pulse");})),
					MakeButton(L"WallFilter result",make_fn([&filtered, &wall_filter](){DisplayMathFunction(filtered, 0, 1, L"Filter applied: "+ wall_filter->filter_name());})),
					MakeButton(L"Exit", function<void()>())
				}
			);

			if(fn) fn();
			else break;
		}
	}
}

void dummy()
{
	GraphSet	gs(L" 'histogram'", L"probability", L"value");
	double nu(0), sigma(0), step (0);
	do
	{
		int n = GetUnsigned("Get n", 1024, 1, 100000);
		double	xmax = GetFloating("Get x max", 10, 0.01, 10000000);
		double A = GetFloating("Get A", 1, 0, 100000);
		double sigma = GetFloating("Get sigma", .5, 0, 10000000);
		double x0 = GetFloating("Get x0", .3, 0, 10000000);
		double	dx = (xmax - x0) / n;

		RealFunctionF64 rice_function(TheoreticalRiceFunction(A, sigma, x0, dx, n));
		DisplayMathFunction(rice_function, 0, 1, L"probability", L"abs. value");
		gs.AddGraphUniform(rice_function, x0, dx, "Rice" + ssprintf(" A=%f, sigma=%f", nu, sigma));
		gs.Display();
	} while (YesOrNo("Repeat?", 1));
}


int xrad::xrad_main(int, char** const)
{

	/*
	RealFunctionF32 a(2);
	a[0] = 2;
	a[1] = 4;
	size_t c = (a[1] - a[0])* (a[1] - a[0])* (a[1] - a[0]);
	ShowText("Results", ssprintf("\nNumber %d, %g", c, c));
	*/

//	dummy();

	XRAD_USING

	try
	{
		size_t	answer(0);
		while (true)
		{
			answer = Decide("Choose source", { "Process single raw dataset", "Process several raw datasets","Process simulated signal", "Wall filter frequency response", "Test dummy hypothesis", "Exit" });
			switch (answer)
			{
				case 0:
				{
					try
					{
						S500_CFMFrameSet frames = GetS500Frames();
						RawDataProcessingMenu(frames);
					}
					catch(canceled_operation){}
					catch(...){ Error(GetExceptionStringOrRethrow()); }
				}
				break;

				case 1:
				{
					try
					{
						S500_CFMFrameSet frames = LoadSeveralDatasets();
						RawDataProcessingMenu(frames);
					}
					catch (canceled_operation) {}
					catch (...) { Error(GetExceptionStringOrRethrow()); }
				}
				break;

				case 2:
				{
					try
					{
						S500_CFMFrameSet frames = SimulateSignal();
						RawDataProcessingMenu(frames);
					}
					catch(canceled_operation){}
					catch(...){ Error(GetExceptionStringOrRethrow()); }
				}
				break;

				case 3:
					WallFilterFrequencyResponse();
					break;
				break;

				case 4:
					DummyHypothesis();
					break;

				default:
					throw canceled_operation("");
			}
		}
	}
	catch (canceled_operation &) {}
	catch (quit_application &) {}
	catch (...) 
	{ 
		Error(GetExceptionString()); 
		return 1;
	}
	return 0;
}
