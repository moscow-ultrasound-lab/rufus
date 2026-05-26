#include "Pre.h"
#include "SyntheticApertureFocuserAina.h"
#include "BeamformerTesting.h"
#include <Attic/CalculateEntropy.h>
#include <XRADBasic/FFT2D.h>
#include "RawSFDataSourceTransEcho.h"
#include <XRADBasic/Sources/Utils/LeastSquares.h>
#include "PolynomialNarrowing.h"


XRAD_BEGIN


RealFunctionF64	PolynomBasis(int number_of_transmit_elements, size_t no, double magn)
{
	RealFunctionF64	vector_time_shift(number_of_transmit_elements, 0);
	for (int i = 0; i < number_of_transmit_elements; ++i)
	{
		double	x = 2.*double(i - number_of_transmit_elements / 2) / number_of_transmit_elements;
		vector_time_shift[i] = magn*ChebyshevPolynom(x, no);
	}
	return vector_time_shift;
}

RealFunctionF64	HarmonicBasis(int number_of_transmit_elements, double freq, double magn, double phi_0)
{
	RealFunctionF64	vector_time_shift(number_of_transmit_elements, 0);
	for (int i = 0; i < number_of_transmit_elements; ++i)
	{
		double x = i * freq * 2 * pi() / number_of_transmit_elements + phi_0 * pi() / 180;
		vector_time_shift[i] = magn*cos(x);
	}
	return vector_time_shift;
}

RealFunctionF64	CosX_1_Basis(int number_of_transmit_elements, double freq, double magn, int center)
{
	RealFunctionF64	vector(number_of_transmit_elements, 0);
	int half_length = 0.5 * number_of_transmit_elements / freq;
	for (int i = (center - half_length); i < (center + half_length); ++i)
	{
		if (!(i < 0))
		{
			if (i < number_of_transmit_elements)
			{
				double x = (i - center) * freq * 2 * pi() / number_of_transmit_elements;
				vector[i] = magn*(1 + cos(x)) / 2;
			}
		}
	}
	return vector;
}

void GetRayNarrowingBasis(GraphSet &errorGS, double &parametr_min, int &parametr_step, double &parametr_max, double &cf_min, double &cf_step,
	double &cf_max, bool &falg_cosx1, bool &flag_harmonic, bool &flag_polynom, physical_angle scanning_sector, size_t number_of_transmit_elements)
{
	size_t	answer(0);
	answer = Decide("Ray narrowing basis", { "Chebyshev", "Harmonic", "0.5(cos(x)+1)" }, SavedGUIValue(answer));
	switch (answer)
	{
	case 0:
	{
		errorGS.ChangeLabels(L"Error for polynom order #", L"value", L"amp");
		parametr_min = 2;
		parametr_step = 1;
		parametr_max = 1 + GetUnsigned("max_polynom_order", MakeGUIValue(7, saved_default_value), parametr_min + 1, 100);
		cf_min = 0;
		cf_step = 1;
		cf_max = 1;
		flag_polynom = true;
	}
	break;
	case 1:
	{
		errorGS.ChangeLabels(L"Error for frequency #", L"value", L"amp");
		parametr_min = 0;
		parametr_step = scanning_sector.degrees();
		parametr_max = parametr_step + 1;
		cf_min = 0.5;
		cf_step = 0.5;
		cf_max = cf_step + GetUnsigned("max_frequency", MakeGUIValue(4, saved_default_value), cf_min + 1, 100);
		flag_harmonic = true;
	}
	break;
	case 2:
	{
		errorGS.ChangeLabels(L"Error for frequency #", L"value", L"amp");
		parametr_min = 0;
		parametr_step = (int)number_of_transmit_elements / 2;
		parametr_max = parametr_step + 1;
		cf_min = 1;
		cf_step = 1;
		cf_max = cf_step + GetUnsigned("max_frequency", MakeGUIValue(4, saved_default_value), cf_min + 1, 100);
		parametr_step /= cf_max;
		falg_cosx1 = true;
	}
	}
}

RealFunctionF64		SyntheticApertureFocuserAina::GetLine(GraphSet &GS, int first_sample, int last_sample, int n_lines, RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected)
{
	ComplexFunction2D_F32 lines_for_correction = Focus(first_sample, last_sample, n_lines, matrix_correction, rays_corrected, *source_buffer);
	DisplayLines(GS, lines_for_correction, number_of_rays, n_lines, 0, scanning_sector);
	lines_for_correction -= AverageValue(lines_for_correction);
	RealFunctionF64 line = CalcLine(lines_for_correction, number_of_rays, n_lines);
	line /= MaxAbsoluteValue(line);
	return line;
}

void SyntheticApertureFocuserAina::Correction(CorrectionTraits &traits, int j, double min)
{
	if ((traits.last_inside_empirical_margin_flag == traits.inside_empirical_margin_flag) || (traits.amplitude == (-traits.amplitude_margin + traits.amplitude_step)) || (traits.inside_empirical_margin_flag == true))
	{
		traits.last_inside_empirical_margin_flag = traits.inside_empirical_margin_flag;
		traits.line = GetLine(traits.lineGS, traits.first_sample, traits.last_sample, traits.n_lines, traits.matrix_correction, traits.ray_to_correct);
		traits.lineGS.ChangeGraphUniform(0, traits.line, -scanning_sector.degrees() / 2, scanning_sector.degrees() / (number_of_rays - 1), L"current line");
		traits.gauss_function = GaussFit(traits.best_fit_sq_error, traits.line, traits.ray_no);
		traits.error_line[j] = traits.best_fit_sq_error;
		traits.approximationGS.ChangeGraphUniform(0, traits.matrix_correction.row(traits.ray_no), 0, 1, ssprintf("current=%g", traits.current_frequency));
		if (traits.best_fit_sq_error < traits.empirical_margin) { traits.inside_empirical_margin_flag = true; }
		else { traits.inside_empirical_margin_flag = false; }
		if (traits.min_error[traits.ray_no] > traits.best_fit_sq_error)
		{
			traits.min_error[traits.ray_no] = traits.best_fit_sq_error;
			traits.errorGS.ChangeLabels(ssprintf(L"Min_error[%lu ray]=%g", traits.ray_no, traits.min_error[traits.ray_no]), L"value", L"amp");
			traits.best_fit_gauss_function = traits.gauss_function;
			traits.best_fit_matrix_correction.row(traits.ray_no) = traits.matrix_correction.row(traits.ray_no);
			traits.best_fit[traits.ray_no] = traits.current_frequency;
			traits.best_fit_line.row(traits.ray_no) = traits.line;
		}
		traits.approximationGS.ChangeGraphUniform(1, traits.best_fit_matrix_correction.row(traits.ray_no), 0, 1, ssprintf("best=%g", traits.best_fit[traits.ray_no]));
		traits.lineGS.ChangeGraphUniform(1, traits.best_fit_line.row(traits.ray_no), -scanning_sector.degrees() / 2, scanning_sector.degrees() / (number_of_rays - 1), L"best fit line");
		traits.errorGS.ChangeGraphUniform(traits.current_frequency - min, traits.error_line, -traits.amplitude_margin, traits.amplitude_step, ssprintf("%g", traits.current_frequency));
	}
}

void SyntheticApertureFocuserAina::PolynomialNarrowing(CorrectionTraits &traits)
{
	GetAreaForCorrection(traits);

	int	parametr_step = 1;
	double parametr_min = 0;
	double parametr_max = 1;
	double cf_min = 0;
	double cf_step = 1;
	double cf_max = 1;
	bool falg_cosx1 = false;
	bool flag_harmonic = false;
	bool flag_polynom = false;

	traits.lineGS.Display(false);
	fixed_angle_transmit = degrees(GetFloating("Choose transmit angle in degrees", MakeGUIValue(0, saved_default_value), -half_scanning_sector.degrees(), half_scanning_sector.degrees()));
	traits.ray_no = int((fixed_angle_transmit.degrees() + half_scanning_sector.degrees()) * number_of_rays / scanning_sector.degrees());
	printf("\n Chosen ray_no = %lu", (int)traits.ray_no);
	printf("\n Chosen angle = %g", fixed_angle_transmit.degrees());
	traits.ray_to_correct.fill(-1);
	traits.ray_to_correct[traits.ray_no] = traits.ray_no;
	if (traits.rays_corrected[traits.ray_no] == traits.ray_no)
	{
		if (!YesOrNo("Use previously calculated correction for this ray", MakeGUIValue(false, saved_default_value)))
		{
			traits.matrix_correction.row(traits.ray_no).fill(0);
			traits.best_fit_matrix_correction.row(traits.ray_no).fill(0);
		}
	}
	RealFunctionF64 line_original = GetLine(traits.lineGS, traits.first_sample, traits.last_sample, traits.n_lines, traits.best_fit_matrix_correction, traits.ray_to_correct);
	traits.lineGS.SetScale(range2_F64(0, double(-scanning_sector.degrees() / 2), MaxValue(line_original), double(scanning_sector.degrees() / 2)));
	GaussFit(traits.min_error[traits.ray_no], line_original, traits.ray_no);
	printf("\n current_error[%lu ray]=%g", (int)traits.ray_no, traits.min_error[traits.ray_no]);
	traits.rays_corrected[traits.ray_no] = traits.ray_no;
	traits.approximationGS.Display(false);
	traits.errorGS.Display(false);
	GetRayNarrowingBasis(traits.errorGS, parametr_min, parametr_step, parametr_max, cf_min, cf_step, cf_max, falg_cosx1, flag_harmonic, flag_polynom, scanning_sector, number_of_transmit_elements);
	size_t steps_to_process = ((cf_max - cf_min) / cf_step)*(parametr_max - parametr_min)/parametr_step * 2 * traits.amplitude_margin/ traits.amplitude_step;
	traits.prog.start("Adjusting Delays", steps_to_process);
	{
		for (double cf = cf_min; cf < cf_max; cf += cf_step)
		{
		//	if (falg_cosx1) { parametr_step /= cf; }
			for (int parametr = parametr_min; parametr < parametr_max; parametr += parametr_step)
			{
				traits.inside_empirical_margin_flag = false;
				traits.last_inside_empirical_margin_flag = traits.inside_empirical_margin_flag;
				traits.best_fit_sq_error = 100;
				traits.correction.MakeCopy(traits.best_fit_matrix_correction.row(traits.ray_no));
				traits.error_line.realloc(int(2 * traits.amplitude_margin / traits.amplitude_step) + 1, 0);
				double j;
				for (traits.amplitude = -traits.amplitude_margin, j = 0; traits.amplitude < traits.amplitude_margin; traits.amplitude += traits.amplitude_step, j++, traits.prog++)
				{
					if (flag_polynom) { traits.matrix_correction.row(traits.ray_no) = PolynomBasis(number_of_transmit_elements, parametr, traits.amplitude) + traits.correction; traits.current_frequency = parametr; }
					if (flag_harmonic) { traits.matrix_correction.row(traits.ray_no) = HarmonicBasis(number_of_transmit_elements, cf, traits.amplitude, parametr) + traits.correction; traits.current_frequency = cf; }
					if (falg_cosx1) { traits.matrix_correction.row(traits.ray_no) = CosX_1_Basis(number_of_transmit_elements, cf, traits.amplitude, parametr) + traits.correction; traits.current_frequency = cf; }
				
					Correction(traits, j, parametr_min);
	
				}
				traits.matrix_correction.MakeCopy(traits.best_fit_matrix_correction);
			}
		}
	}
	traits.prog.end();
}


XRAD_END