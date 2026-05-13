#include "Pre.h"
#include "SyntheticApertureFocuserAina.h"
#include "BeamformerTesting.h"
#include <Attic/CalculateEntropy.h>
#include <XRADBasic/FFT2D.h>
#include "RawSFDataSourceTransEcho.h"
#include <XRADBasic/Sources/Utils/LeastSquares.h>
#include "DirectPhaseEstimation.h"

XRAD_BEGIN


ComplexFunctionF64 InterpolateComplexFunctionF64(size_t new_length, ComplexFunctionF64 c_function)
{
	ComplexFunctionF64 function_interpolated(new_length, complexF64(0));
	RealFunctionF64 re_function(real(c_function));
	RealFunctionF64 im_function(imag(c_function));
	for (size_t i = 0; i < new_length; ++i)
	{
		double coordinate = i * double(c_function.size()) / function_interpolated.size();
		double re = re_function.in(coordinate);
		function_interpolated[i].re = re;
		double im = im_function.in(coordinate);
		function_interpolated[i].im = im;
	}
	return function_interpolated;
}

RealFunctionF64 InterpolateRealFunctionF64(size_t new_length, RealFunctionF64 function)
{
	RealFunctionF64 function_interpolated(new_length, 0);
	for (size_t i = 0; i < new_length; ++i)
	{
		double coordinate = i * double(function.size()) / function_interpolated.size();
		double p = function.in(coordinate);
		function_interpolated[i] = p;
	}
	return function_interpolated;
}

void RearrangeLines(ComplexFunction2D_F64 &lines, size_t number_of_rays, size_t n_lines)
{
	ComplexFunctionF64	line_buffer;
	ComplexFunctionF64	line_rearranged(2 * number_of_rays, complexF32(0));
	for (int j = 0; j < n_lines; ++j)
	{
		line_buffer = InterpolateComplexFunctionF64(2*number_of_rays, lines.col(j));
		
		for (int i = 0; i < 2 * number_of_rays; i++)
		{
			if (i < (number_of_rays + 1))
			{
				line_rearranged[i] = line_buffer[i + (number_of_rays - 1)];
			}
			else
			{
				line_rearranged[i] = line_buffer[i - (number_of_rays + 1)];
			}
		}
		ComplexFunctionF64	line_result = InterpolateComplexFunctionF64(number_of_rays, line_rearranged);
		lines.col(j).CopyData(line_result);
	}
}

RealFunctionF64 PhaseCompute(ComplexFunction2D_F64 lines, size_t number_of_rays, size_t n_lines, size_t ray_no)
{
	RealFunctionF64		derivatives(number_of_rays - 1, 0);
	RealFunctionF64		phase(number_of_rays, 0);
	ComplexFunctionF64	line_buffer(number_of_rays, complexF32(0));
	ComplexFunctionF64	line_rearranged(number_of_rays, complexF32(0));
	ComplexFunctionF64	line_fft(number_of_rays, complexF32(0));
	for (int j = 0; j < n_lines; ++j)
	{
		line_fft.CopyData(lines.col(j));
		FFTf(line_fft, fftRevRollAfter);
//		FFTf(line_fft, fftRevRollBoth);
		for (size_t i = 1; i < number_of_rays; ++i)
		{
			derivatives[i - 1] += arg(line_fft[i] % line_fft[i - 1]) / n_lines;
		}
	}	
	phase.CopyData(derivatives);
	for (size_t i = 1; i < number_of_rays; ++i)
	{
		phase[i] += phase[i - 1];
	}
	return phase;
}

void	DisplayPhaseWithLimits(GraphSet	&GS, physical_length normalized_lambda, size_t number_of_rays, physical_length aperture_width, RealFunctionF64 phase)
{
	int int_coef = 4;
	RealFunctionF64 phase_int = InterpolateRealFunctionF64(number_of_rays*int_coef, phase);
	phase_int -= AverageValue(phase_int);
	GS.ChangeGraphUniform(0, phase_int*180/pi(), (-(double)number_of_rays / 2. + 1)* normalized_lambda.cm(), normalized_lambda.cm() / int_coef, L"");
	RealFunctionF32 borders(number_of_rays*int_coef, 0);
	borders[(number_of_rays / 2 - 1 - int(0.5*aperture_width.cm() / normalized_lambda.cm()))*int_coef] = 10;
	borders[(number_of_rays / 2 - 1 + int(0.5*aperture_width.cm() / normalized_lambda.cm()))*int_coef] = 10;
	GS.ChangeGraphUniform(1, -borders * 180 / pi(), (-(double)number_of_rays / 2. + 1)* normalized_lambda.cm(), normalized_lambda.cm() / int_coef, L"");
	GS.ChangeGraphUniform(2, borders * 180 / pi(), (-(double)number_of_rays / 2. + 1)* normalized_lambda.cm(), normalized_lambda.cm() / int_coef, L"");
}

RealFunctionF64 PhaseCut(GraphSet &GS, size_t number_of_rays, physical_angle scanning_sector, physical_speed sound_speed, physical_frequency f0, physical_length aperture_width, RealFunctionF64 phase, int n_elements)
{
	RealFunctionF64 phase_cut(n_elements, 0);

	physical_length	array_pitch = aperture_width / n_elements;
	physical_length lambda = cm(sound_speed.cm_sec() / f0.Hz());
	physical_length normalized_lambda = lambda / (scanning_sector.radians()*double(scanning_sector.degrees() - 1) / scanning_sector.degrees());
	double compression_ratio = array_pitch.cm() / normalized_lambda.cm();
	DisplayPhaseWithLimits(GS, normalized_lambda, number_of_rays, aperture_width, phase);
	double	j0 = double(n_elements - 1) / 2;
	double	c0 = number_of_rays / 2 - 1;

	for (int j = 0; j < n_elements; ++j)
	{
		double	component_no = c0 + (j - j0) * compression_ratio;
		phase_cut[j] = phase.in(component_no);
	}
	return phase_cut;
}

void SyntheticApertureFocuserAina::DirectPhaseEstimation(CorrectionTraits &traits)
{
	{
		double highest_amp(0);
		for (size_t i = 0; i < number_of_rays; i++) 
		{ 
			if (traits.best_result[i] > highest_amp) 
			{ 
				highest_amp = traits.best_result[i];
			} 
		}
		GraphSet	lineGS2(L"lineGS2", L"degrees", L"");
		size_t best_iteration(0);
		if (traits.best_correction_flag) { printf("\n best_iteration"); }
		for (size_t iteration_no = 1; iteration_no < traits.number_of_iterations; ++iteration_no, ++traits.prog)
		{
			ComplexFunction2D_F64 lines_for_correction = Focus(traits.first_sample, traits.last_sample, traits.n_lines, traits.matrix_correction, traits.ray_to_correct, *source_buffer);
			DisplayLines(traits.lineGS, lines_for_correction, number_of_rays, traits.n_lines, iteration_no, scanning_sector);
			RearrangeLines(lines_for_correction, number_of_rays, traits.n_lines);
			double result = DisplayLines(lineGS2, lines_for_correction, number_of_rays, traits.n_lines, iteration_no, scanning_sector);
			{
				if (traits.best_result[traits.ray_no] < result)
				{
					traits.best_result[traits.ray_no] = result;
					best_iteration = iteration_no;
					if (traits.best_correction_flag)
					{
						printf(" #%lu", (int)best_iteration);
						traits.best_fit_matrix_correction.row(traits.ray_no) = traits.matrix_correction.row(traits.ray_no);
					}
				}
			}
			if (highest_amp < result)
			{
				highest_amp = result;
				traits.lineGS.SetScale(range2_F64(0, double(-scanning_sector.degrees() / 2), highest_amp, double(scanning_sector.degrees() / 2)));
			}
			RealFunctionF64	phase = PhaseCompute(lines_for_correction, number_of_rays, traits.n_lines, traits.ray_no);
			RealFunctionF64 phase_cut = PhaseCut(traits.phaseGS, number_of_rays, scanning_sector, sound_speed, f0, aperture_width, phase, number_of_transmit_elements);
			RealFunctionF64 phase_int = phase_cut;
			phase_int -= AverageValue(phase_int);
			traits.matrix_correction.row(traits.ray_no) -= phase_int / (2 * pi() * f0.Hz());
			traits.matrix_correction.row(traits.ray_no) -= AverageValue(traits.matrix_correction.row(traits.ray_no));
			traits.errorGS.ChangeGraphUniform(iteration_no, (phase_int / (2 * pi() * f0.Hz()))*(180 / (1 / (2 * f0.Hz()))), -double(number_of_transmit_elements / 2. - 0.5), 1, ssprintf(L"#%lu", iteration_no));
			traits.approximationGS.ChangeGraphUniform(iteration_no, traits.matrix_correction.row(traits.ray_no)*(180 / (1 / (2 * f0.Hz()))), -double(number_of_transmit_elements / 2. - 0.5), 1, ssprintf(L"#%lu", iteration_no));
			if (!traits.best_correction_flag)
			{
				traits.best_fit_matrix_correction.row(traits.ray_no) = traits.matrix_correction.row(traits.ray_no);
			}
		}
		SubstractLinearShift(traits.best_fit_matrix_correction.row(traits.ray_no));
	}
}

void SyntheticApertureFocuserAina::GetAreaForCorrection(CorrectionTraits &traits)
{
	traits.first_sample = (int)GetUnsigned("first_sample_to_correct", MakeGUIValue(300, saved_default_value), 0, samples);
	traits.last_sample = (int)GetUnsigned("last_sample_to_correct", MakeGUIValue(traits.first_sample + 100, saved_default_value), traits.first_sample, samples);
	traits.n_lines = (int)GetUnsigned("number of samples to correct", MakeGUIValue(5, saved_default_value), 0, traits.last_sample - traits.first_sample);
}

void SyntheticApertureFocuserAina::DirectPhaseEstimationFullFrame(CorrectionTraits &traits)
{
	GetAreaForCorrection(traits);

	traits.lineGS.SetGraphStyle(solid_color_lines, 5.5);
	traits.lineGS.Display(false);
	traits.errorGS.SetScale(range2_F64(-180, -double(number_of_transmit_elements / 2.), 180, double(number_of_transmit_elements / 2.)));
	traits.errorGS.ChangeLabels(L"Last - Estimated", L"degrees", L"element number");
	traits.errorGS.Display(false);
	traits.approximationGS.ChangeLabels(L"Estimated", L"degrees", L"element number");
	traits.approximationGS.SetScale(range2_F64(-180, -double(number_of_transmit_elements / 2.), 180, double(number_of_transmit_elements / 2.)));
	traits.approximationGS.Display(false);
	traits.phaseGS.SetGraphStyle(solid_color_lines, 5.5);
	traits.phaseGS.SetScale(range2_F64(double(-90), -((double)number_of_rays / 2. + 1) * 0.5 * normalized_lambda.cm(), double(90), ((double)number_of_rays / 2. + 1) * 0.5 * normalized_lambda.cm()));
	traits.phaseGS.Display(false);

	traits.rays_corrected.fill(-1);
	physical_angle angle_trasmit_left = degrees(GetFloating("Define the left boarder of the correction zone", MakeGUIValue(0, saved_default_value), -half_scanning_sector.degrees(), half_scanning_sector.degrees()));
	physical_angle angle_trasmit_right = degrees(GetFloating("Define the right boarder of the correction zone", angle_trasmit_left.degrees(), angle_trasmit_left.degrees(), half_scanning_sector.degrees()));
	size_t number_of_patches = GetUnsigned("Number of isoplanatic patches", MakeGUIValue(9, saved_default_value), 1, number_of_rays) + 2;
	traits.number_of_iterations = GetUnsigned("Number of iterations", MakeGUIValue(1, saved_default_value), 0, 100) + 1;
	traits.best_correction_flag = YesOrNo("Keep only the best corrections?", MakeGUIValue(1, saved_default_value));
	traits.prog.start("Adjusting Delays", (number_of_patches - 2)*(traits.number_of_iterations - 1));
	for (int s = 1; s < number_of_patches - 1; s++, traits.prog++)
	{
		fixed_angle_transmit = angle_trasmit_left + degrees(s * (angle_trasmit_right.degrees() - angle_trasmit_left.degrees()) / number_of_patches);
		traits.ray_no = int((fixed_angle_transmit.degrees() + half_scanning_sector.degrees()) * number_of_rays / scanning_sector.degrees());
		traits.ray_to_correct.fill(-1);
		traits.ray_to_correct[traits.ray_no] = traits.ray_no;
		traits.rays_corrected[traits.ray_no] = traits.ray_no;
		printf("\n Chosen ray_no = %lu", (int)traits.ray_no);
		printf("\n Chosen angle = %g", fixed_angle_transmit.degrees());
		DirectPhaseEstimation(traits);
	}
	traits.prog.end();
}

XRAD_END