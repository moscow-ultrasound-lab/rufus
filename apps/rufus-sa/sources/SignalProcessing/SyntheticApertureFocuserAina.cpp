#include "Pre.h"
#include "SyntheticApertureFocuserAina.h"
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


XRAD_BEGIN
/*
SyntheticApertureFocuserAina::SyntheticApertureFocuserAina()
{
}

SyntheticApertureFocuserAina :: ~SyntheticApertureFocuserAina()
{
}
*/


struct FocuserSettings
{
	bool	dynamic_focus_tx = true;
	bool	dynamic_focus_rx = false;
	bool	fixed_angle_tx = true;
	bool	fixed_angle_rx = false;
	double	F_tx = 10;
	double	F_rx = 10;
};

void	SyntheticApertureFocuserAina::InitWork()
{
	Modify_Options();

	source_buffer.reset(new RawSFDataSourceTransEcho);
	source_buffer->LoadOtherSource(source_original);

	source_buffer->sound_speed = source_original.sound_speed;
	source_buffer->raw_signal_sample_rate = source_original.raw_signal_sample_rate;
	source_buffer->first_raw_sample = source_original.first_raw_sample;
	source_buffer->samples_per_element = source_original.samples_per_element;
	source_buffer->recommended_angle = source_original.recommended_angle;
	source_buffer->recommended_n_rays = source_original.recommended_n_rays;
	source_buffer->n_tx_elements = source_original.n_tx_elements;
	source_buffer->n_rx_elements = source_original.n_rx_elements;
	source_buffer->array_pitch = source_original.array_pitch;
	plane_wave = source_original.plane_wave_flag;
	estimated_f0 = source_buffer->EstimateCentralFrequency();
	printf("\nEstimated frequency=%g MHz", estimated_f0.MHz());
	f0 = estimated_f0;
	//source_buffer->Display();
	source_buffer->HighPassFilter();	
	
	if (YesOrNo("Display raw data?", MakeGUIValue(false, saved_default_value))) {source_buffer->Display();}

 	focused_data.realloc({n_frames, n_rays, n_samples}, complexF32(0));

	strcpy(SIMIO::Process_Name, "Rays focuser");
	delayed_signals.realloc({data_source->n_tx_elements, data_source->n_rx_elements, n_samples});

	SetOutputFileName(data_source->GetFileName(), "focused");

	ComputeApodization();

	strcpy(SIMIO::Data_Complexity, ANALYTIC);

	// задаем шаг решетки в отсчетах
	XRAD_ASSERT_THROW_M(data_source->n_tx_elements == data_source->n_rx_elements, logic_error, "Invalid array configuration");

	elements_offsets.realloc(data_source->n_tx_elements);
	TX_Delays.realloc(data_source->n_tx_elements, n_samples);
	RX_Delays.realloc(data_source->n_rx_elements, n_samples);

	ComputeElementsOffsets();

//	ISignal_Write();

}

ComplexFunction2D_F32	CorrectDCOffset(ComplexFunction2D_F32 image)
{
	for (int i = 0; i < image.vsize(); ++i)
	{
		auto	blur = image.row(i);
		blur.FilterGauss(10);
		image.row(i) -= blur;
	}
	return image;
}

RealFunction2D_F64	RearrangeMatrixCorrection(RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected, int number_of_rays)
{
	RealFunctionF32 isoplanatic_patches(number_of_rays, 0), rays_corrected_rearanged(number_of_rays, 0);
	RealFunction2D_F64 matrix_correction_new(matrix_correction);
	int j = 0;
	for (size_t i = 0; i < number_of_rays; ++i)
	{
		if (rays_corrected[i] == -1) {}
		else {rays_corrected_rearanged[j] = rays_corrected[i];	j++;}
	}
	rays_corrected_rearanged.resize(j);
	for (size_t i = 0, j = 0; i < number_of_rays; ++i)
	{
		if (rays_corrected_rearanged.size() == 0) {}
		else
		{
			isoplanatic_patches[i] = rays_corrected_rearanged[j];
			if (rays_corrected_rearanged.size() > 1)
			{
				if (i == int(0.5*(rays_corrected_rearanged[j + 1] + rays_corrected_rearanged[j]))) {j++;}
			}
		}
	}
	for (size_t i = 0; i < number_of_rays; ++i)
	{
		matrix_correction_new.row(i) = matrix_correction.row(isoplanatic_patches[i]);
	}
	return matrix_correction_new;
}

ComplexFunction2D_F32	SyntheticApertureFocuserAina::Focus(int first_sample, int last_sample, int n_lines, RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected, RawSFDataSource &current_source)
{
	RealFunction2D_F64 matrix_correction_new = RearrangeMatrixCorrection(matrix_correction, rays_corrected, number_of_rays);
	ComplexFunction2D_F32 image(number_of_rays, n_lines, complexF32(0));
	GUIProgressBar	progress;
	std::mutex	m;
	progress.start("Focusing", number_of_transmit_elements);
	{
		auto	aperture_centre = 0.5*number_of_transmit_elements*pitch;

		for (int transmit_element = 0; transmit_element < number_of_transmit_elements; ++transmit_element)
		{
			auto	transmit_element_position = (transmit_element + 0.5)*pitch - aperture_centre;

			#pragma omp parallel for schedule (guided)
			for (int receive_element = 0; receive_element < number_of_receive_elements; ++receive_element)
			{
				auto	receive_element_position = (receive_element + 0.5)*pitch - aperture_centre;
				//auto	row = current_source.GetRow(transmit_element, receive_element);
				auto	row = current_source.GetRow(transmit_element + current_frame_no * number_of_transmit_elements, receive_element);
			
				ComplexFunctionF32	buffer(image.hsize(), complexF32(0));
				for (int i = 0; i < number_of_rays; ++i)
				{
					auto	fi = (0.5 + i - number_of_rays / 2)*dfi;
					// dynamic_focal_angle_transmit = true;  // отключаем фиксацию угла
					fi = dynamic_focal_angle_transmit ? fi : (fi + fixed_angle_transmit);
					double	transmit_angle = dynamic_focal_angle_transmit ? fi.radians() : fixed_angle_transmit.radians();
					double	receive_angle = dynamic_focal_angle_receive ? fi.radians() : angle_receive.radians();
					for (int j = 0; j < n_lines; j ++)
					{
					auto	CalculateInfluence = [&]()
						{
							int		j_depth = first_sample + j*((last_sample - first_sample) / n_lines);
							auto	depth = (start_depth + j_depth*step_depth);
							physical_length	transmit_focal_depth_dynamic = dynamic_focal_depth_transmit ? depth : focal_depth_transmit;
							auto	transmit_path = depth - transmit_element_position*sin(transmit_angle);
							if (!plane_wave) {transmit_path += transmit_element_position*transmit_element_position / (2 * transmit_focal_depth_dynamic); }
							physical_length	receive_focal_depth_dynamic = dynamic_focal_depth_receive ? depth : focal_depth_receive;
							auto	receive_path = depth - receive_element_position*sin(receive_angle);
							if (!plane_wave) { receive_path += receive_element_position*receive_element_position / (2 * receive_focal_depth_dynamic); }
							auto	path = receive_path + transmit_path;
							auto	t = path / current_source.sound_speed;
							t += sec(matrix_correction_new.at(i, transmit_element)) + sec(matrix_correction_new.at(i, receive_element));
							double	sample_no = double(current_source.raw_signal_sample_rate*t);
							return row->in(sample_no - (int)current_source.first_raw_sample);
						};
						auto increment = CalculateInfluence();
						buffer.at(j) = increment;
					}
					std::lock_guard<std::mutex>	lock(m);
					image.row(i) += buffer;
				}
			}
			++progress;
		}
	}
	progress.end();
	return image;
}


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

void DisplayCorrection(RealFunction2D_F64 best_fit_matrix_correction, RealFunctionF32 rays_corrected)
{
	size_t nuber_of_rays = best_fit_matrix_correction.vsize();
	size_t nuber_of_elements = best_fit_matrix_correction.hsize();
	RealFunctionF32 rays(rays_corrected);
	RealFunction2D_F64 correction(best_fit_matrix_correction);
	size_t j(0);
	for (int ray_no = 0; ray_no < nuber_of_rays; ray_no++)
	{
		if(rays_corrected[ray_no] == ray_no)
		{
			//rays[j] = rays_corrected[ray_no];
			rays[j] = 1;
			correction.row(j) = best_fit_matrix_correction.row(ray_no);
			j++;
		}
	}
	rays.resize(j);
	correction.resize(j, nuber_of_elements);
	DisplayMathFunction(rays_corrected, 0, 1, "rays_corrected");
//	DisplayMathFunction(rays, 0, 1, "rays");
	DisplayMathFunction2D(correction, "correction");
}
void SyntheticApertureFocuserAina::CalibrationSourceMethod(RealFunction2D_F64 &matrix_correction, RealFunction2D_F64 &best_fit_matrix_correction, RealFunctionF32 &rays_corrected, physical_angle half_scanning_sector)
{
	fixed_angle_transmit = degrees(0);
	size_t ray_no = int((fixed_angle_transmit.degrees() + half_scanning_sector.degrees()) * number_of_rays / scanning_sector.degrees());
	rays_corrected[ray_no] = ray_no;
	source_buffer->InitCalibrationPulses();
	matrix_correction.row(ray_no) = source_buffer->aberration_profile_function;
	SubstractLinearShift(matrix_correction.row(ray_no));
	best_fit_matrix_correction.row(ray_no) = matrix_correction.row(ray_no);
}

void SyntheticApertureFocuserAina::Sharpness(RealFunction2D_F64 &matrix_correction, RealFunctionF32 &rays_corrected)
{
	CorrectionTraits	traits(number_of_transmit_elements, number_of_rays);
	traits.rays_corrected.MakeCopy(rays_corrected);
	traits.matrix_correction.MakeCopy(matrix_correction);


	GraphSet	errorGS(L"Error", L"value", L"amp");
	GraphSet	approximationGS(L"Polynom", L"value", L"element number");
	GraphSet	lineGS(L"Line", L"value", L"degrees");
	GraphSet	phaseGS(L"Phase", L"degrees", L"");

	RealFunctionF64		min_error(number_of_rays ,100);
	//RealFunction2D_F64	best_fit_matrix_correction(number_of_rays, number_of_transmit_elements, 0);
	traits.best_fit_matrix_correction.fill(0);
	RealFunctionF32		ray_to_correct(number_of_rays, -1);
	
	dynamic_focal_angle_transmit = false;
	samples = (int)source_original.samples_per_element / 2;
	img.realloc(number_of_rays, samples, complexF32(0));
	calculate_phase_correction = false;
	apply_phase_correction = false;
	
	RealFunctionF32 best_result(number_of_rays, 0);

	approximationGS.SetGraphStyle(solid_color_lines, 5.5);
	source_buffer->Normalize();
	//size_t ray_no(number_of_rays/2);
	bool go_on(true);
	while(go_on == true)
	{
		try
		{ 
#pragma message TODO проверить симметричность картины относительно нуля
			
			if (YesOrNo("Add aberration?", MakeGUIValue(false, saved_default_value)))
			{
				RealFunctionF32 aberration	= AddAberration(approximationGS, number_of_transmit_elements, f0);
				for(size_t ray_no = 0; ray_no < number_of_rays; ray_no++)
				{
					traits.matrix_correction.row(ray_no) = aberration;
					traits.best_fit_matrix_correction.row(ray_no) = traits.matrix_correction.row(ray_no);
				}
				if (YesOrNo("Display aberrated frame before correction?", MakeGUIValue(false, saved_default_value)))
				{
					DisplayFrame(traits.best_fit_matrix_correction, rays_corrected, "Aberrated Before Correction");
				}
			}
			GUIProgressBar	prog;
			traits.prog = prog;
			
			size_t	answer(0);
			answer = Decide("Aberration correction method", { "Polynomial ray narrowing", "Direct phase estimation", "Calibration source", "Correlation-based"}, SavedGUIValue(answer));
			switch (answer)
			{
			case 0:
			{
					traits.min_error.MakeCopy(min_error);
					
					PolynomialNarrowing(traits);

					min_error.MakeCopy(traits.min_error);
			}
			break;
			case 1:
			{
				traits.half_scanning_sector = half_scanning_sector;
				traits.scanning_sector = scanning_sector;
				traits.fixed_angle_transmit = fixed_angle_transmit;
				traits.number_of_rays = number_of_rays;
				traits.best_result.MakeCopy(best_result);

				DirectPhaseEstimationFullFrame(traits);

				best_result.MakeCopy(traits.best_result);
			}
			break;
			case 2:
			{
				CalibrationSourceMethod(traits.matrix_correction, traits.best_fit_matrix_correction, traits.rays_corrected, half_scanning_sector);
			}
			break;
			case 3:
			{
				AberrationsCorrector instance;
				instance.InitWork();
				instance.ComputeAberration();
				traits.best_fit_matrix_correction.MakeCopy(instance.displayer_slice);
				for (size_t ray_no = 0; ray_no < number_of_rays; ray_no++)
				{
					traits.rays_corrected[ray_no] = ray_no;
				}
			}
			break;
			}
			rays_corrected.MakeCopy(traits.rays_corrected);
			matrix_correction.MakeCopy(traits.matrix_correction);
			//best_fit_matrix_correction.MakeCopy(traits.best_fit_matrix_correction);
		}
		//catch (canceled_operation &){printf("\n min_error[%lu ray]=%g", (int)ray_no, min_error[ray_no]);}
		catch (canceled_operation &) { }
		if (YesOrNo("Display corrected frame?", MakeGUIValue(true, saved_default_value))) { DisplayFrame(traits.best_fit_matrix_correction, rays_corrected, "After"); }
		if (YesOrNo("Display matrix correction?", true)) { DisplayCorrection(traits.best_fit_matrix_correction*(2 * f0.Hz()) * 180, rays_corrected); }
		if (YesOrNo("Continue correction?", true)) {go_on = true;} else {go_on = false;}
	}
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

void	SyntheticApertureFocuserAina::DisplayFrame(RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected, string title)
{
	dynamic_focal_angle_transmit = YesOrNo("dynamic_focal_angle_transmit", MakeGUIValue(true, saved_default_value));
	
	if (dynamic_focal_angle_transmit == false)
	{
		fixed_angle_transmit = degrees(GetFloating("Choose transmit angle in degrees", MakeGUIValue(0, saved_default_value), -half_scanning_sector.degrees(), half_scanning_sector.degrees()));
	}
	
	img.realloc(number_of_rays, samples, complexF32(0));
	calculate_phase_correction = false;
	apply_phase_correction = false;
	int		first_sample_to_correct;
	int		last_sample_to_correct;
	int		n_lines;
	first_sample_to_correct = 0;
	last_sample_to_correct = samples;
	n_lines = last_sample_to_correct - first_sample_to_correct;
	
	index_vector	frame_sizes({ size_t(number_of_frames), size_t(number_of_rays), size_t(samples) });
	ComplexFunction2D_F32 image;

	image = Focus(first_sample_to_correct, last_sample_to_correct, n_lines, matrix_correction, rays_corrected, source_original);
		
	ScanConverterOptions sco(ScanFrameSector(start_depth * degrees(scanning_sector.degrees()).radians(), 2 * step_depth * n_lines, -half_scanning_sector, half_scanning_sector));

	ComplexFunction2D_F32 image_blured;
	image_blured = CorrectDCOffset(image);
	DisplayMathFunction2D(image_blured, title, sco);

	CaclAandSTD(image, half_scanning_sector.degrees());
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
		image = Focus(first_sample_to_correct, last_sample_to_correct, n_lines, matrix_correction, rays_corrected, source_original);
		images.GetSlice({ size_t(current_frame_no), slice_mask(0), slice_mask(1) }).CopyData(image);
	}

	ScanConverterOptions sco(ScanFrameSector(start_depth * degrees(scanning_sector.degrees()).radians(), 2 * step_depth * n_lines, -half_scanning_sector, half_scanning_sector));
	BModeRiceTestSyntheticAperture(images, sco);
}

void	SyntheticApertureFocuserAina::ChooseOption(RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected)
{

	enum
	{
		correct_aberrations,
		calculate_rice,
		display_exit
	} option;
	while (true)
	{
		option = GetButtonDecision("Choose option",
			{
				MakeButton(L"Correct Aberrations",				correct_aberrations),
				MakeButton(L"Calculate Rice params",			calculate_rice),
				MakeButton("Exit",								display_exit)
			});
		try
		{
			switch (option)
			{

				case correct_aberrations:
				{
					if (YesOrNo("Display frame before correction?", MakeGUIValue(false, saved_default_value))) 
					{ 
						DisplayFrame(matrix_correction, rays_corrected, "Before"); 
					}
					Sharpness(matrix_correction, rays_corrected);
					if (YesOrNo("Display frame?", MakeGUIValue(false, saved_default_value))) 
					{ 
						DisplayFrame(matrix_correction, rays_corrected, "After"); 
					}
				}
				break;

				case calculate_rice:
				{
					CalculateFrameRice(matrix_correction, rays_corrected, "Rice");
				}
				break;

				default:
					throw canceled_operation("");
			}
		}
		catch (canceled_operation&) { throw; }
		catch (quit_application&) { throw; }
		catch (...) { Error(GetExceptionString()); }
	}
}


void	SyntheticApertureFocuserAina::Batch()
{
	bool display_raw_data(false);
	if (display_raw_data == true) { data_source->Display(); }
	number_of_receive_elements = (int)source_original.n_rx_elements;
	number_of_transmit_elements = (int)source_original.n_tx_elements;
	number_of_frames = source_original.n_frames;
	aperture_width = number_of_transmit_elements*(data_source->array_pitch);
	number_of_rays = 128;
	samples = (int)source_original.samples_per_element / 2;
	pitch = source_original.array_pitch;
	auto	sample_rate = source_original.raw_signal_sample_rate / 2;
	step_depth = 0.5*source_original.sound_speed / sample_rate;
	start_depth = 0.5*(int)source_original.first_raw_sample*source_original.sound_speed / source_original.raw_signal_sample_rate;
	dfi = scanning_sector / number_of_rays;
	RealFunction2D_F64 matrix_correction(number_of_rays, number_of_transmit_elements, 0);
	RealFunctionF32 rays_corrected(number_of_rays, -1);
	ChooseOption(matrix_correction, rays_corrected);
}

XRAD_END