#include "pre.h"
#include <XRADBasic/MathFunctionTypes2D.h>
#include <XRADGUI/Sources/GUI/MathFunctionGUI2D.h>
#include <XRADBasic/Sources/Utils/TimeProfiler.h>

#include "AnalyzeCFMFrameSet.h"
#include <DopplerBasics/Display/DuplexDisplayer.h>

#include "..\rufus-elasto\ElastoGrafica\rufus-elasto.h"


/********************************************************************
	created:	2016/10/19
	created:	19:10:2016   17:05
	filename: 	q:\programs\ElastoGrafica\sources\AnalyzeCFMFrameSet.cpp
	file path:	q:\programs\ElastoGrafica\sources
	file base:	AnalyzeCFMFrameSet
	file ext:	cpp
	author:		kns
	
	purpose:	
*********************************************************************/

XRAD_BEGIN

//using namespace ElastoGrafica;

#define CallElastoFunction(function_name,...){\
		ElastoGrafica::elasto_status status = ElastoGrafica::function_name(__VA_ARGS__);\
		if (status != ElastoGrafica::elasto_status::ok) throw status;}



void	AnalyzeCFMFrameSet(S500_CFMFrameSet &frames)
{
	RealFunctionF32	offsets_graph(frames.n_frames);

	RealFunctionMD_F32	offset_frames({frames.n_frames, frames.n_cfm_beams(), frames.n_cfm_samples()});
	RealFunctionMD_F32	mask_frames({frames.n_frames, frames.n_cfm_beams(), frames.n_cfm_samples()});

	//DuplexDisplayer displayer(frames, offset_frames, mask_frames);
	
	DuplexDisplayer displayer(frames, offset_frames, mask_frames, false, "Elasto");

	//displayer.DisplayCFMResults();

//	DuplexDisplayer	dd(frames, phase_frames_normalized, mask_frames, true, "Elasto");
	//dd.DisplayDuplex();

//	displayer.DisplayDuplex();
//	DisplayMathFunction3D(mask_frames, "mask", frames.sco_cfm);
//	DisplayMathFunction3D(offset_frames, "offset_frames", frames.sco_cfm);



	
	auto	mode_no = GetButtonDecision("Select mode",
			{
				MakeButton("CFM", cfm_mode::cfm),
				MakeButton("Elastography", cfm_mode::elastography),
				MakeButton("Cancel", cfm_mode::none)
		
			}	
		);
	if (mode_no == cfm_mode::none) throw canceled_operation("");

	string	settings_filename = GetFileNameRead("ElastographyLib.ini file", saved_default_value, "*.ini");

// 	static double	elastogram_agility = 0.1;	//(mode_no == 2 ? 0.1:0.25);
// 	static double	axial_blur =	 3;		//(mode_no == 2 ? 3 : 1);

//	CallElastoFunction(InitElastographyLib, "c:\\temp\\ElastographyLib.ini");
	CallElastoFunction(InitElastographyLib, settings_filename.c_str());
	CallElastoFunction(ChangeMode, mode_no);

	double elastogram_agility = GetFloating("Acquisition factor (Agility)", SavedGUIValue(0.25), 0, 2);
	double axial_blur = GetFloating("Axial blur value (Blur)", SavedGUIValue(1), 0.1, 10);


	CallElastoFunction(SetElastogramAgilityDirect, elastogram_agility);
	CallElastoFunction(SetElastogramBlurDirect, axial_blur);

//	ProgressBar	progress;
	GUIProgressBar	progress;
	progress.start("Analyzing elastograms", frames.n_frames);

	TimeProfiler	total_tp;
	total_tp.Start();

	CallElastoFunction(SetFrameSizes, frames.n_cfm_shots(), frames.n_cfm_beams(), frames.n_cfm_samples(), frames.n_cfm_beams_in_sweep(), 0, 0, frames.n_cfm_samples());
	CallElastoFunction(ResetElastogram);
	for(size_t frame_no = 0; frame_no < frames.n_frames; ++frame_no)
	{
		int	*cfm_ptr = (int*)&frames.cfm_shot_frames.at({ frame_no, 0, 0, 0 });
		// указатель исходных данных ЦДК
		
		float *elastogram_ptr = &offset_frames.at({ frame_no, 0, 0 });
		// указатель на итоговую эластограмму (массив n_cfm_beams строк по n_cfm_samples отсчетов)
		
		float *graph_sample_ptr = &offsets_graph[frame_no];
		// указатель на переменную, в которую мы помещаем среднюю скорость сжатия образца
		
		float *mask_ptr = &mask_frames.at({ frame_no, 0, 0 });
		// указатель на маску прозрачности
		
		CallElastoFunction(BuildElastogram, cfm_ptr, elastogram_ptr, graph_sample_ptr, mask_ptr);
		++progress;
	}
	total_tp.Stop();
	progress.end();

	string	time_consumption = ElastoGrafica::GetTimeConsumptionReport();

	CallElastoFunction(FinishElastographyLib);


	time_consumption += ssprintf("\nFull elapsed time %g sec", total_tp.LastElapsed().sec());

	static GraphSet	offsets_gs("Average offsets", "offset", "frame no");
	if(mode_no == cfm_mode::elastography)
	{
		offsets_gs.ChangeGraphUniform(0, offsets_graph, 0, 1, "total frames offsets");
		offsets_gs.Display(false);
	}
	else
	{
		offsets_gs.Hide();
	}
	static TextDisplayer	td("Time consumption report");
	td.SetText(time_consumption);
	td.Display(false);


	displayer.DisplayDuplex();
	//DisplayMathFunction3D(offset_frames, "Offsets", frames.sco_cfm);
	//DisplayMathFunction3D(mask_frames, "mask", frames.sco_cfm);
	displayer.DisplayCFMResults();
}


XRAD_END
