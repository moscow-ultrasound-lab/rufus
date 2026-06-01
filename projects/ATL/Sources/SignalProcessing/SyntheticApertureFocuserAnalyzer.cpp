#include "Pre.h"
#include "Pre.h"
#include "SyntheticApertureFocuserAina.h"
#include "SyntheticApertureFocuserAnalyzer.h"
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
#include <Q:\projects\CommonSources\BBasics\RiceBasics.h>"

//for reading txt files with aberration profiles
#include <iostream>
#include <fstream>
#include <string>
//#include <charconv>
//#include <system_error>
#include <sstream>

#include "Verasonics\RawSFDataSourceVerasonics.h"


XRAD_BEGIN


RealFunctionF64		SyntheticApertureFocuserAina::GaussFit(double &best_fit_sq_error, RealFunctionF64 line, size_t ray_no)
{
	best_fit_sq_error = 100;
	double power = 0;
	RealFunctionF64 gauss_function(number_of_rays, 0);
	RealFunctionF64 gauss_function_best(number_of_rays, 0);
	for (double std = 1; std < 4; std += 0.1)
	{
		double sq_error = 0;
		for (size_t i = 0; i < number_of_rays; ++i)
		{
			gauss_function[i] = gauss(i - ray_no, std);
			double	error = abs(line[i]) - gauss_function[i];
			double	a(1);
			for (int current_power = 0; current_power < power; current_power++)
			{
				a *= abs(int(i - ray_no));
			}
			sq_error += error*error*a;
		}
		if (best_fit_sq_error > sq_error)
		{
			gauss_function_best = gauss_function;
			best_fit_sq_error = sq_error;
		}
	}
	return gauss_function_best;
}

void SubstractLinearShift(RealFunctionF64 &phase_interpolated)
{
	RealVectorF64 coefficients(2);
	DetectLSPolynomUniformGrid(phase_interpolated, coefficients);
	for (int i = 0; i < phase_interpolated.size(); ++i)
	{
		phase_interpolated[i] -= coefficients[0] + coefficients[1] * i;
	}
}

RealFunctionF64 CalcLine(ComplexFunction2D_F64 lines, size_t number_of_rays, size_t n_lines)
{
	RealFunctionF64 line(number_of_rays, 0);
	for (int i = 0; i < number_of_rays; i++)
	{
		for (int j = 0; j < n_lines; j++)
		{
			line[i] += cabs(lines.at(i, j));
		}
	}
	return line;
}

double DisplayLines(GraphSet &GS, ComplexFunction2D_F64 lines, size_t number_of_rays, size_t n_lines, size_t iteration_no, physical_angle scanning_sector)
{
	auto	dfi = scanning_sector / (number_of_rays - 1);
	RealFunctionF64 line = CalcLine(lines, number_of_rays, n_lines);
	int int_factor = 40;
	RealFunctionF64 line_int = InterpolateRealFunctionF64(number_of_rays * int_factor, line);
	GS.ChangeGraphUniform(iteration_no, line_int, -scanning_sector.degrees() / 2, dfi.degrees() / int_factor, ssprintf(L"#%lu", iteration_no));
	return line_int[0];
}

void ReadTextFile(RealFunctionF32 &aberration, physical_frequency f0)
{
	aberration.fill(0);
	size_t length = size(aberration);
	string	filename = GetFileNameRead("Aberration profile file", SavedGUIValue("*.txt"), "*.txt");
	std::ifstream file(filename, std::ios::in);
	std::string line;
	float x(0), y(0), z(0), i(0);
	if (file.is_open())
	{
		std::cout << "File: " << filename << "\n\n";
		while (std::getline(file, line)) {
			std::cout << line << "\n\n";
			std::istringstream in(line);      //make a stream for the line itself	
			for (int i = 0; i < size(aberration); i++)
			{
				in >> aberration[i];			 //now read the whitespace-separated floats
			}
		}
	}
	else {
		std::cout << "Failed to open the file";
	}
	file.close();
/*
	std::cout << "\n\nFloats\n";
	for (int i = 0; i < size(aberration); i++)
	{
		std::cout << aberration[i] << " ";
	}
*/
	aberration /= 180 / (1 / (2 * f0.Hz()));
}

RealFunctionF32 AddAberration(GraphSet &GS,size_t length, physical_frequency f0)
{
	RealFunctionF32 aberration(length, 0);
	bool quit_cycle(false);
	while (quit_cycle == false)
	{
		aberration.fill(0);
		size_t	answer(0);
		answer = Decide("Choose aberration", { "Chebyshev -pi+pi", "Symmetrical 0+0.8pi  - Average", "Step 0+0.8pi", "Step (-0.5-1)pi", "Step - Average", "Step 0+0.9pi", "Linear", "Comb", "Read from file" }, SavedGUIValue(answer));
		switch (answer)
		{
			case 0:
			{
				size_t order = GetUnsigned("polynom order", MakeGUIValue(2, saved_default_value), 1, (int)length/2);
				double magnitude = GetFloating("polynom magnitude", MakeGUIValue(0.9, saved_default_value), 0, 1) / (2 * f0.Hz());
				aberration = PolynomBasis((int)length, order, magnitude);
			}
			break;
			case 1:
			{
				for (int i = 0.15*length; i < 0.85*length; i++)
				{
					aberration[i] = 0.8 / (2 * f0.Hz());
				}
				aberration -= AverageValue(aberration);

			}
			break;
			case 2:
			{
				for (int i = 0.75*length; i < length; i++)
				{
					aberration[i] = 0.8 / (2 * f0.Hz());
				}
			}
			break;
			case 3:
			{
				aberration.fill(-0.5 / (2 * f0.Hz()));
				for (int i = 0.75*length; i < length; i++)
				{
					aberration[i] = -1 / (2 * f0.Hz());
				}
			}
			break;
			case 4:
			{
				for (int i = 0; i < 0.25*length; i++)
				{
					aberration[i] = 1 / (2 * f0.Hz());
				}
			}
			aberration -= AverageValue(aberration);
			break;
			case 5:
			{
				for (int i = 0.75*length; i < length; i++)
				{
					aberration[i] = 0.9 / (2 * f0.Hz());
				}
			}
			break;
			case 6:
				{
					physical_angle delta = degrees(GetFloating("Choose phase difference in degrees between the first and the last elements", MakeGUIValue(0, saved_default_value), 0, 1e3));
					for (int i = 0; i < length; i++)
						 {
						aberration[i] = i * delta.degrees() / ((length - 1) * 2 * 180 * f0.Hz());
						}
					aberration -= AverageValue(aberration);
					}
				break;
			case 7:
			{
				int j(0);
				aberration.fill(0);
				for (int i = 0; i < length; i++)
				{
					if (i>(j*double(length) / 10))
					{
						aberration[i] = 0.8 / (2 * f0.Hz());
					}
					if (i > ((j+0.25)*double(length) / 10))
					{
						j++;
					}
				}
				aberration -= AverageValue(aberration);
			}
			case 8:
			{
				ReadTextFile(aberration, f0);
			}
			break;
		}
		GS.ChangeGraphUniform(0, aberration*(180/(1 / (2 * f0.Hz()))), -double(length / 2.), 1, "Given");
		GS.SetScale(range2_F64(-180, -double(length/2.), 180, double(length/2.)));
		GS.Display(false);
		if (YesOrNo("Use this aberration?", MakeGUIValue(false, saved_default_value))) { quit_cycle = true; }
	}
	return aberration;
}

void CaclAandSTD(ComplexFunction2D_F32 image, double half_sector)
{
	if (YesOrNo("Calculate A, STD and FWHM?", MakeGUIValue(true, saved_default_value)))
	{
		do
		{
			
		size_t sample_number = GetSigned("Choose sample", MakeGUIValue(0, saved_default_value), 0, image.sizes(1));
		size_t std_range(14);// = GetSigned("STD range in beams", MakeGUIValue(0, saved_default_value), 0, image.sizes(0));  //в статье Фурье и Полиномы этот параметр равен 14, что соответствует +-5 градусам
		double max(0), std(0);
		size_t coord_max(0);
		RealFunctionF64 line(image.sizes(0), 0);
		for (int i = 0; i < image.sizes(0); i++)
		{
			line[i] = cabs(image.at(i, sample_number));
			if (max < cabs(image.at(i, sample_number)))
			{
				max = cabs(image.at(i, sample_number));
				coord_max = i;
			}
		}



		double half_max(max / 2), FWHM_1(0), FWHM_2(0);
		size_t coord_half_max_left_1(0), coord_half_max_left_2(0), coord_half_max_right_1(0), coord_half_max_right_2(0);
		for (int i = 1; i < image.sizes(0); i++)
		{
			if (coord_half_max_left_1 == 0)
			{
				if ((line[i-1] < half_max) && (line[i] >= half_max))
				{
					coord_half_max_left_1 = i;
				}
			}
			if (i < coord_max)
			{
				if ((line[i - 1] < half_max) && (line[i] >= half_max))
				{
					coord_half_max_left_2 = i;
				}
			}
			
			if ((line[i - 1] > half_max) && (line[i] <= half_max))
			{
				coord_half_max_right_1 = i;
			}
		}


		for (int i = image.sizes(0); i > 0; i--)
		{
			if (i > coord_max)
			{
				if ((line[i - 1] > half_max) && (line[i] <= half_max))
				{
					coord_half_max_right_2 = i;
				}
			}
		}
		 

		FWHM_1 = (coord_half_max_right_1 - coord_half_max_left_1) * 90/ image.sizes(0);
		FWHM_2 = (coord_half_max_right_2 - coord_half_max_left_2) * 90 / image.sizes(0);


		double numerator(0), denominator(0), ph(0), ph_max(0);
		//double numerator_ph(0);
		if (CapsLock()) { coord_max = GetSigned("Enter coordinate max", MakeGUIValue(0, saved_default_value), 0, image.sizes(0));}
		RealFunctionF64 line_norm(line/ max);
		double phase_position_max = half_sector * (-1 + double(coord_max) * 2 / double(image.sizes(0)));
		for (int i = (coord_max - std_range/2); i < (1+coord_max + std_range / 2); i++)
		{
			ph = half_sector*(-1 + double(i) * 2 / double(image.sizes(0)));
			ph_max = ph - phase_position_max;
			//numerator_ph += ph_max * ph_max;
			numerator += line_norm[i]* ph_max * ph_max;
			denominator += line_norm[i];
		}
		std = sqrt(numerator / denominator);

		//DisplayMathFunction(line, 0, 1, ssprintf(L"Sample #%lu, STD range = %lu, coord_max = %lu, A = %g, STD = %g, phposmax = %g, denom=%g, numerator_ph=%g, num=%g", sample_number, std_range, coord_max, max, std, phase_position_max, denominator, numerator_ph, numerator));
		DisplayMathFunction(line, 0, 1, ssprintf(L"Sample #%lu, A = %g, STD = %g, left %lu, right %lu, FWHM = %g deg, left %lu, right %lu, FWHM = %g deg", sample_number, line[coord_max], std, coord_half_max_left_1, coord_half_max_right_1, FWHM_1, coord_half_max_left_2, coord_half_max_right_2, FWHM_2));
		} while (YesOrNo("Calculate for another sample?", MakeGUIValue(true, saved_default_value)));
	}
}


void	SyntheticApertureFocuserAina::CalculateFrameRice(RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected, string title)
{
	dynamic_focal_angle_transmit = true;
	img.realloc(number_of_rays, samples, complexF32(0));
	calculate_phase_correction = false;
	apply_phase_correction = false;
	int		first_sample_to_correct(0);
	int		last_sample_to_correct(samples);
	int		n_lines(last_sample_to_correct - first_sample_to_correct);

	index_vector	frame_sizes({ size_t(number_of_frames), size_t(number_of_rays), size_t(samples) });
	ComplexFunctionMD_F32 images(frame_sizes, complexF32(0));
	ComplexFunction2D_F32 image;

	for (current_frame_no = 0; current_frame_no < size_t(number_of_frames); ++current_frame_no)
	{
		image = SectorDASFocus(first_sample_to_correct, last_sample_to_correct, n_lines, matrix_correction, rays_corrected, source_original);
		images.GetSlice({ size_t(current_frame_no), slice_mask(0), slice_mask(1) }).CopyData(image);
	}

	ScanConverterOptions sco(ScanFrameSector(start_depth * degrees(scanning_sector.degrees()).radians(), 2 * step_depth * n_lines, -half_scanning_sector, half_scanning_sector));
	BModeRiceTestSyntheticAperture(images, sco);
}


XRAD_END