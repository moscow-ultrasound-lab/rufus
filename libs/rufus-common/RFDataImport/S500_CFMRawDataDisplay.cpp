#include "pre.h"
#include <XRADBasic/MathFunctionTypesMD.h>
#include <XRADGUI/Sources/Gui/MathFunctionGUIMD.h>
#include <XRADBasic/ThirdParty/nlohmann/json.hpp>
#include <XRADBasic/Sources/Utils/utf8_ofstream.h>
//#include <DopplerBasics/CFM/WallFilters/WallFiltersInteractive.h>

#include "S500_CFMFrameSet.h"
#include "S500_CFMRawDataDisplay.h"


//#if RUN_CFM_MODES
#include <DopplerBasics/CFM/WallFilters/WallFiltersInteractive.h>
//#endif

XRAD_BEGIN


void	ApplyWallFilter(cfm_container_t& cfm_frames, size_t n_cfm_shots)
{
#if RUN_CFM_MODES

	size_t	n_frames = cfm_frames.sizes(0);
	size_t	n_cfm_beams = cfm_frames.sizes(1) / n_cfm_shots;
	size_t	n_cfm_samples = cfm_frames.sizes(2);

	cfm_container_t	cfm_shot_frames;
	cfm_shot_frames.UseData(&cfm_frames.at({ 0,0,0 }), { n_frames, n_cfm_beams, n_cfm_shots, n_cfm_samples }, 1);

	auto	wf = GetWallFilterInteractive(n_cfm_shots);

	for (size_t frame = 0; frame < n_frames; ++frame)
	{
		for (size_t beam = 0; beam < n_cfm_beams; ++beam)
		{
			for (size_t sample = 0; sample < n_cfm_samples; ++sample)
			{
				auto	burst = cfm_shot_frames.GetRow({ frame, beam, slice_mask(0), sample });
				ComplexFunctionF32	cburst(burst);
				wf->Apply(cburst);
				burst.CopyData(cburst);
			}
		}
	}
#endif
}

void	DisplayCFMDetailed(const cfm_container_t& in_cfm_frames, size_t n_cfm_shots, string title, const ScanConverterOptions& sco_cfm)
{
	auto	cfm_frames(in_cfm_frames);

	if (cfm_frames.sizes(1) % n_cfm_shots)
	{
		Error("Invalid n_cfm_shots");
		return;
	}
#if RUN_CFM_MODES
	if (YesOrNo("Apply wall filter before display?", saved_default_value))
	{
		ApplyWallFilter(cfm_frames, n_cfm_shots);
	}
#endif
	size_t	n_frames = cfm_frames.sizes(0);
	size_t	n_cfm_beams = cfm_frames.sizes(1) / n_cfm_shots;
	size_t	n_cfm_samples = cfm_frames.sizes(2);

	size_t	fixed_frame = n_frames / 2;
	size_t	fixed_ray = n_cfm_beams / 2;
	size_t	fixed_sample = n_cfm_samples / 2;
	size_t	fixed_shot = n_cfm_shots / 2;

	size_t answer(0);
	enum answers
	{
		e_everything,
		e_fixed_frame,
		e_fixed_ray,
		e_fixed_sample,
		e_fixed_shot,
		e_save_shots,
		e_exit
	};

	cfm_container_t::invariable	subset;
	cfm_container_t::invariable	cfm_shot_frames;
	cfm_shot_frames.UseData(&cfm_frames.at({ 0,0,0 }), { n_frames, n_cfm_beams, n_cfm_shots, n_cfm_samples }, 1);

	//	Эти строки появляются в качестве caption в разных диалогах.
	//	Надо, чтобы эти captions оставались одинаковы, т.к. по их названиям сохраняется default value в реестре

	string fixed_ray_string = "ray no";
	string fixed_sample_string = "sample no";


	while (answer != e_exit)
	{
		answer = GetButtonDecision(title, //e_exit+1,
/*
			<<<<<<< HEAD
			{
			"Everything",
			"Fixed frame",
			"Fixed ray",
			"Fixed sample",
			"Fixed shot",
			"Exit"
			}
		);

		switch (answer)
=======
*/
				{
				"Everything",
				"Fixed frame",
				"Fixed ray",
				"Fixed sample",
				"Fixed shot",
				"Export shots for position",
				"Exit"
				}
			);
		//quick_iv(n_frames, n_cfm_beams, n_cfm_shots, n_cfm_samples)
		switch(answer)
//>>>>>>> 54b369b39402bc4a88d3d8164e717f9adb6ae6f7
		{
		case e_everything:
			DisplayMathFunction3D(cfm_frames, title + " / CFM-frames", sco_cfm);
			break;
		case e_fixed_frame:
		{
			fixed_frame = GetUnsigned("Frame no", long(fixed_frame), 0, long(n_frames - 1));
			cfm_shot_frames.GetSubset(subset, { fixed_frame, slice_mask(2), slice_mask(0), slice_mask(1) });
			DisplayMathFunction3D(subset, title + ssprintf(" / Fixed frame #%d", fixed_frame));
		}
		break;
		case e_fixed_ray:
//<<<<<<< HEAD
		{
			fixed_ray = GetUnsigned("ray no", long(fixed_ray), 0, long(n_cfm_beams - 1));
			cfm_shot_frames.GetSubset(subset, { slice_mask(2), fixed_ray, slice_mask(0), slice_mask(1) });
			DisplayMathFunction3D(subset, title + ssprintf(" / Fixed ray #%d", fixed_ray));
		}
		break;
/*		case e_fixed_sample:
		{
			fixed_sample = GetUnsigned("sample no", long(fixed_sample), 0, long(n_cfm_samples - 1));
			cfm_shot_frames.GetSubset(subset, { slice_mask(1), slice_mask(2), slice_mask(0), fixed_sample });
			DisplayMathFunction3D(subset, title + ssprintf(" / Fixed sample #%d", fixed_sample));
		}
		break;

		case e_fixed_shot:
		{
			fixed_shot = GetUnsigned("shot no", long(fixed_shot), 0, long(n_cfm_shots - 1));
			cfm_shot_frames.GetSubset(subset, { slice_mask(0), slice_mask(1), fixed_shot, slice_mask(2) });
			DisplayMathFunction3D(subset, title + ssprintf(" / Fixed shot #%d", fixed_shot), sco_cfm);
=======
*/
			{
				fixed_ray = GetUnsigned(fixed_ray_string, long(fixed_ray), 0, long(n_cfm_beams-1));
				cfm_shot_frames.GetSubset(subset, {slice_mask(2), fixed_ray, slice_mask(0), slice_mask(1)});
				// порядок следования срезов такой, чтобы по умолчанию выдавалась
				// анимация по номеру выстрела, по горизонтали кадры, по вертикали отсчеты
				DisplayMathFunction3D(subset, title + ssprintf(" / Fixed ray #%d", fixed_ray));
			}
			break;
		case e_fixed_sample:
			{
				fixed_sample = GetUnsigned(fixed_sample_string, long(fixed_sample), 0, long(n_cfm_samples-1));
				cfm_shot_frames.GetSubset(subset, {slice_mask(1), slice_mask(2), slice_mask(0), fixed_sample});
				// порядок следования срезов такой, чтобы по умолчанию выдавалась анимация
				// по номеру выстрела, по горизонтали лучи, по вертикали кадры
				DisplayMathFunction3D(subset, title + ssprintf(" / Fixed sample #%d", fixed_sample));
			}
			break;
		case e_fixed_shot:
			{
				fixed_shot = GetUnsigned("shot no", long(fixed_shot), 0, long(n_cfm_shots-1));
				cfm_shot_frames.GetSubset(subset, {slice_mask(0), slice_mask(1), fixed_shot, slice_mask(2)});
				// порядок следования срезов такой, чтобы правильно работал сканконвертер
				DisplayMathFunction3D(subset, title + ssprintf(" / Fixed shot #%d", fixed_shot), sco_cfm);
			}
			break;
		case e_save_shots:
		{
			fixed_ray = GetUnsigned(fixed_ray_string, long(fixed_ray), 0, long(n_cfm_beams-1));
			fixed_sample = GetUnsigned(fixed_sample_string, long(fixed_sample), 0, long(n_cfm_samples-1));

			bool	e_normalize = YesOrNo("Normalize average power to 1?", true);

			wstring	filename = ssprintf(L"bursts_ray%zu_sample%zu.json", fixed_ray, fixed_sample);
			filename = GetFileNameWrite(L"Выберите имя файла", filename, L"*.json");

			nlohmann::json	jtotal;
			
			GraphSet	gs("Bursts", "re", "im");


			for(size_t frame_no = 0; frame_no < n_frames; ++frame_no)
			{
				nlohmann::json	jframe;
				nlohmann::json	jburst;
				ComplexFunctionF32	burst;
				burst.MakeCopy(cfm_shot_frames.GetRow({frame_no, fixed_ray, slice_mask(0), fixed_sample}));

				if(e_normalize)
				{
//					double	average = sqrt(cabs(AverageSquare(burst)));
					double	average = sqrt(cabs(MaxValue(burst)));
					burst /= average;



//					double	phase = 0;
// 					for(size_t i = 0; i < n_cfm_shots; ++i)
// 					{
// 						phase += arg(burst[i]);
// 					}
				
//					gs.AddGraphParametric(imag(burst), real(burst), ssprintf("f%d", frame_no));
					gs.ChangeGraphParametric(0, imag(burst), real(burst), ssprintf("f%d", frame_no));
					gs.Display();
				}

				for(size_t i = 0; i < n_cfm_shots; ++i)
				{
					jburst[i][0] = burst[i].re;
					jburst[i][1] = burst[i].im;
				}
				jtotal[frame_no] = jburst;
			}


			utf8_ofstream ofs(filename);
			ofs << jtotal;

//>>>>>>> 54b369b39402bc4a88d3d8164e717f9adb6ae6f7
		}
		break;
		};
	}
}



void	DisplayCFMFrameSet(const S500_CFMFrameSet& frames, wstring title)
{
	size_t	option = 0;
	ScanConverterOptions sco_cfm;

	enum { cfm_frames, b_frames, exit, n_options };
	do
	{
		option = GetButtonDecision(wstring_to_string(title, e_encode_literals), /*n_options,*/{ "CFM-frames", "B-frames", "Exit" });
		try
		{
			switch (option)
			{
			case cfm_frames:
				DisplayCFMDetailed(frames.cfm_frames, frames.n_cfm_shots(), "CFM-frames", frames.sco_cfm);
				break;
			case b_frames:
				DisplayMathFunction3D(frames.b_frames, "B-frames", frames.sco_b);
				break;
			case exit:
				break;
			}
		}
		catch (canceled_operation&) {}
	} while (option != n_options - 1);
}

XRAD_END


