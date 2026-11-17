#include "Pre.h"
#include "SyntheticApertureFocuserSectorProbe.h"
#include "BeamformerTesting.h"
#include <Attic/CalculateEntropy.h>
#include <XRADBasic/FFT2D.h>
#include <XRADBasic/Sources/Utils/LeastSquares.h>

#include "DirectPhaseEstimation.h"
#include "PolynomialNarrowing.h"
#include "RawSFDataSourceTransEcho.h"
#include "RawSFDataSourceSimIO.h"
#include "AberrationsCorrector.h"
//#include <Q:\projects\CommonSources\BBasics\BModeDetailedTest.h>"
#include <BBasics\RiceBasics.h>"

//for reading txt files with aberration profiles
#include <iostream>
#include <fstream>
#include <string>
//#include <charconv>
//#include <system_error>
#include <sstream>


XRAD_BEGIN


void SyntheticApertureBeamformer::CorrectionSoS(CorrectionTraits& traits, int j, double parametr_min_sos, double parametr_max_sos)
{
	if ((traits.last_inside_empirical_margin_flag == traits.inside_empirical_margin_flag) || (traits.inside_empirical_margin_flag == true))
	{
		traits.last_inside_empirical_margin_flag = traits.inside_empirical_margin_flag;
		traits.line = GetLine(traits.lineGS, traits.first_sample, traits.last_sample, traits.n_lines, traits.matrix_correction, traits.ray_to_correct);
		traits.lineGS.ChangeGraphUniform(0, traits.line, -scanning_sector.degrees() / 2, scanning_sector.degrees() / (number_of_rays - 1), ssprintf("current line for %g m/s", traits.current_frequency));
		traits.gauss_function = GaussFit(traits.best_fit_sq_error, traits.line, traits.ray_no);
		traits.error_line[j] = traits.best_fit_sq_error;
		if (traits.best_fit_sq_error < traits.empirical_margin) { traits.inside_empirical_margin_flag = true; }
		else { traits.inside_empirical_margin_flag = false; }
		if (traits.min_error[traits.ray_no] > traits.best_fit_sq_error)
		{
			traits.min_error[traits.ray_no] = traits.best_fit_sq_error;
			traits.errorGS.ChangeLabels(ssprintf(L"Min_error[%lu ray]=%g", traits.ray_no, traits.min_error[traits.ray_no]),  L"error", L"speed of sound");
			traits.best_fit_gauss_function = traits.gauss_function;
			traits.best_fit_matrix_correction.row(traits.ray_no) = traits.matrix_correction.row(traits.ray_no);
			traits.best_fit[traits.ray_no] = traits.current_frequency;
			traits.best_fit_line.row(traits.ray_no) = traits.line;
		}
		if(j == 0) traits.errorGS.SetScale(range2_F64(0, parametr_min_sos, 2 * traits.error_line[0], parametr_max_sos));
		traits.lineGS.ChangeGraphUniform(1, traits.best_fit_line.row(traits.ray_no), -scanning_sector.degrees() / 2, scanning_sector.degrees() / (number_of_rays - 1), ssprintf("best line for %g m/s", traits.best_fit[traits.ray_no]));
		traits.errorGS.ChangeGraphUniform(0, traits.error_line, traits.amplitude_margin, traits.amplitude_step, ssprintf("%g", traits.current_frequency));
	}
}

void SyntheticApertureBeamformer::PreparationforCorrectionSoS(CorrectionTraits& traits, double parametr_min_sos, double parametr_max_sos, double parametr_step_sos)
{
	double parametr_range_sos(parametr_max_sos - parametr_min_sos);
	traits.lineGS.Display(false);
	printf("\n Chosen ray_no = %lu", (int)traits.ray_no);
	printf("\n Chosen angle = %g", fixed_angle_transmit.degrees());
	traits.ray_to_correct.fill(-1);
	traits.ray_to_correct[traits.ray_no] = traits.ray_no;
	RealFunctionF64 line_original = GetLine(traits.lineGS, traits.first_sample, traits.last_sample, traits.n_lines, traits.best_fit_matrix_correction, traits.ray_to_correct);
	traits.lineGS.SetScale(range2_F64(0, double(-scanning_sector.degrees() / 2), MaxValue(line_original), double(scanning_sector.degrees() / 2)));
	GaussFit(traits.min_error[traits.ray_no], line_original, traits.ray_no);
	traits.rays_corrected[traits.ray_no] = traits.ray_no;
	traits.errorGS.Display(false);
	//traits.errorGS.SetScale(range2_F64(0, parametr_min_sos, 5, parametr_max_sos));
	traits.errorGS.ChangeLabels(L"Error for speed of sound", L"error", L"speed of sound");
	size_t sample_no = traits.first_sample + (traits.last_sample - traits.first_sample) / 2;
	printf("\n Minimal error for ray #%lu and sample #%lu is %g", (int)traits.ray_no, (int)sample_no, traits.min_error[traits.ray_no]);
	traits.amplitude_margin = parametr_min_sos;
	traits.amplitude_step = parametr_step_sos;
	traits.error_line.realloc(int(parametr_range_sos / parametr_step_sos) + 1, 0);
	size_t steps_to_process = (parametr_max_sos - parametr_min_sos) / parametr_step_sos;
	traits.prog.start("Adjusting Delays", steps_to_process);
	{
		for (double parametr_sos = parametr_min_sos; parametr_sos < parametr_max_sos; parametr_sos += parametr_step_sos)
		{
			traits.inside_empirical_margin_flag = false;
			traits.last_inside_empirical_margin_flag = traits.inside_empirical_margin_flag;
			traits.best_fit_sq_error = 100;
			traits.correction.MakeCopy(traits.best_fit_matrix_correction.row(traits.ray_no));
			source_original.sound_speed = m_sec(parametr_sos);
			traits.current_frequency = parametr_sos;
			CorrectionSoS(traits, int((parametr_sos - parametr_min_sos) / parametr_step_sos), parametr_min_sos, parametr_max_sos);
			source_original.sound_speed = m_sec(traits.best_fit[traits.ray_no]);
			++traits.prog;
			traits.matrix_correction.MakeCopy(traits.best_fit_matrix_correction);
		}
	}
	traits.prog.end();
	printf("\n Estimated SoS is %g m/s \n", traits.best_fit[traits.ray_no]);
	traits.matrix_sos.at(traits.ray_no, sample_no) = traits.best_fit[traits.ray_no];
}

void SyntheticApertureBeamformer::GetAreaForCorrectionSoS(CorrectionTraits& traits)
{
	traits.roi_first_ray = (int)GetUnsigned("Define ROI: first ray", MakeGUIValue(n_rays / 2, saved_default_value), 0, n_rays);
	traits.roi_first_sample = (int)GetUnsigned("Define ROI: first sample", MakeGUIValue(samples / 2, saved_default_value), 0, samples);
	traits.roi_last_ray = (int)GetUnsigned("Define ROI: last ray", MakeGUIValue(traits.roi_first_ray, saved_default_value), traits.roi_first_ray, n_rays);
	traits.roi_last_sample = (int)GetUnsigned("Define ROI: last sample", MakeGUIValue(traits.roi_first_sample, saved_default_value), traits.roi_first_sample + 1, samples);
	traits.roi_step_in_samples = (int)GetUnsigned("Define ROI: step in samples", MakeGUIValue(1, saved_default_value), 1, traits.roi_last_sample - traits.roi_first_sample);
	traits.n_lines = (int)GetUnsigned("Define number of lines to accumulate", MakeGUIValue(5, saved_default_value), 1, samples - traits.roi_first_sample);
	traits.distance_between_lines = (int)GetUnsigned("Choose distance (in samples) between lines for accumulation", MakeGUIValue(10, saved_default_value), 1, samples / traits.n_lines);
}

void SyntheticApertureBeamformer::Narrowing_SoS(CorrectionTraits& traits)
{
	GetAreaForCorrectionSoS(traits);
	traits.matrix_sos.realloc(n_rays, n_samples);

	size_t roi_width_in_rays = traits.roi_last_ray - traits.roi_first_ray + 1;
	size_t roi_width_in_samples = traits.roi_last_sample - traits.roi_first_sample + 1;
	size_t number_of_calculations = roi_width_in_rays * roi_width_in_samples / traits.roi_step_in_samples;
	double parametr_min_sos = GetFloating("Define min SoS value", MakeGUIValue(1000, saved_default_value), 1, 3000);
	double parametr_max_sos = GetFloating("Define max SoS value", MakeGUIValue(2000, saved_default_value), 1, 3000);
	double parametr_step_sos = GetFloating("Define step", MakeGUIValue(10, saved_default_value), 1, 500);

	printf("\nMin SoS=%g", parametr_min_sos);
	printf("\nMax SoS=%g", parametr_max_sos);
	printf("\nStep SoS=%g", parametr_step_sos);

	GUIProgressBar 	progress;
	progress.start("Processing ROI", number_of_calculations);
	{
		for (traits.ray_no = traits.roi_first_ray; traits.ray_no < traits.roi_last_ray + 1; traits.ray_no++)
		{
			for (size_t sample_no = traits.roi_first_sample; sample_no < traits.roi_last_sample + 1; sample_no += traits.roi_step_in_samples)
			{
				traits.first_sample = sample_no - traits.distance_between_lines * traits.n_lines / 2;
				traits.last_sample = traits.first_sample + traits.distance_between_lines * traits.n_lines;
				if (using_linear_array_probe)
				{
					dynamic_focal_angle_transmit = false;
					sector_angle_deg = 45;
					fixed_ray_no = traits.ray_no;
				}
				else
				{
					fixed_angle_transmit = degrees(scanning_sector.degrees() * traits.ray_no / number_of_rays);
				}
				PreparationforCorrectionSoS(traits, parametr_min_sos, parametr_max_sos, parametr_step_sos);
				progress++;
			}
		}		
	}
}

void SyntheticApertureBeamformer::DisplaySoSMap(RealFunction2D_F64& data)
{

	if (using_linear_array_probe)
	{
		physical_length scan_width = cm(pitch.cm() * number_of_rays);
		physical_length scan_depth = cm(0.5 * source_original.samples_per_element * (source_original.sound_speed.cm_sec() / source_original.raw_signal_sample_rate.Hz()));

		DisplayMathFunction2D(data, "Map SoS interpolated before Scanconversion");
		ComplexFunction2D_F32 data_complex(data.vsize(), data.hsize(), complexF32(0));
		for (size_t r = 0; r < data.vsize(); ++r)
		{
			for (size_t c = 0; c < data.hsize(); ++c)
			{
				data_complex.at(r, c) = complexF32(data.at(r, c), 0);
			}
		}
		DisplayTrapezoid(data_complex, "Map SoS interpolated", scan_width, scan_depth, sector_angle_deg);
	}
	else
	{
		ScanConverterOptions sco(ScanFrameSector(start_depth * degrees(scanning_sector.degrees()).radians(), 2 * step_depth * samples, -half_scanning_sector, half_scanning_sector));
		DisplayMathFunction2D(data, "Map SoS interpolated", sco);
	}
}

void SyntheticApertureBeamformer::Estimation_SoS(RealFunction2D_F64& matrix_correction, RealFunctionF32& rays_corrected)
{
	RealFunction2D_F64 matrix_sos;
	CorrectionTraits    traits(number_of_transmit_elements, number_of_rays);

	traits.rays_corrected.MakeCopy(rays_corrected);
	traits.matrix_correction.MakeCopy(matrix_correction);
	traits.matrix_sos.realloc(n_rays, n_samples);

	GraphSet    errorGS(L"Error", L"value", L"amp");
	GraphSet    lineGS(L"Line", L"value", L"degrees");

	RealFunctionF64        min_error(number_of_rays, 100);
	traits.best_fit_matrix_correction.fill(0);
	RealFunctionF32        ray_to_correct(number_of_rays, -1);

	dynamic_focal_angle_transmit = false;
	samples = (int)source_original.samples_per_element / 2;
	img.realloc(number_of_rays, samples, complexF32(0));
	calculate_phase_correction = false;
	apply_phase_correction = false;

	RealFunctionF32 best_result(number_of_rays, 0);

	if (!using_linear_array_probe)  // нормировка смазывает луч. Пока отключил для линейного датчика. Надо разобраться в ее необходимости 
	{
		source_buffer->Normalize();
	}

	bool go_on(true);
	while (go_on == true)
	{
		try
		{
			GUIProgressBar    prog;
			traits.prog = prog;
			traits.min_error.MakeCopy(min_error);
			Narrowing_SoS(traits);
			matrix_sos = traits.matrix_sos;
			min_error.MakeCopy(traits.min_error);
			rays_corrected.MakeCopy(traits.rays_corrected);
			matrix_correction.MakeCopy(traits.matrix_correction);
		}
		catch (canceled_operation&) {}
		double mean_sos(0);
		if (YesOrNo("Display map of SoS?", MakeGUIValue(true, saved_default_value)))
		{
			if (YesOrNo("Perform interpolation?", MakeGUIValue(true, saved_default_value)))
			{
				RealFunction2D_F64 matrix_sos_interpolated(matrix_sos);

				for (traits.ray_no = traits.roi_first_ray; traits.ray_no < traits.roi_last_ray + 1; traits.ray_no++)
				{
					for (size_t sample_no = traits.roi_first_sample; sample_no < traits.roi_last_sample + 1; sample_no += traits.roi_step_in_samples)
					{
						double buffer = matrix_sos.at(traits.ray_no, sample_no);
						for (size_t i = 0; i < traits.roi_step_in_samples; i++)
						{
							matrix_sos_interpolated.at(traits.ray_no, sample_no + i) = buffer;
							mean_sos += buffer/((traits.roi_last_ray - traits.roi_first_ray + 1) * (traits.roi_last_sample - traits.roi_first_sample + 1)/ traits.roi_step_in_samples);
						}
					}
				}
				DisplaySoSMap(matrix_sos_interpolated);
			}
			else
			{
				DisplaySoSMap(matrix_sos);
			}

		}
		printf("\nmean_sos = %g m/s", mean_sos);
		source_original.sound_speed = m_sec(mean_sos);
		if (YesOrNo("Display corrected frame?", MakeGUIValue(true, saved_default_value))) 
		{ 
			if (using_linear_array_probe) {DisplayFrameLinearProbe(traits.best_fit_matrix_correction, rays_corrected, "After");}
			else {DisplayFrameSectorProbe(traits.best_fit_matrix_correction, rays_corrected, "After");}
		}
		if (YesOrNo("Continue correction?", true)) { go_on = true; }
		else { go_on = false; }
	}
}

XRAD_END
