#include "Pre.h"
#include "SyntheticApertureFocuserSectorProbe.h"
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
#include "BBasics\RiceBasics.h"
#include "Verasonics\RawSFDataSourceVerasonics.h"
#include "Verasonics\RawSFDataSourceVerasonicsMat.h"
#include "UlaOp\RawSFDataSourceUlaOp.h"
#include "PICMUS\PlaneWaveFocuserPICMUS.h"

//for reading txt files with aberration profiles
#include <iostream>
#include <fstream>
#include <string>
//#include <charconv>
//#include <system_error>
#include <sstream>

XRAD_BEGIN

struct FocuserSettings
{
	bool	dynamic_focus_tx = true;
	bool	dynamic_focus_rx = false;
	bool	fixed_angle_tx = true;
	bool	fixed_angle_rx = false;
	double	F_tx = 10;
	double	F_rx = 10;
};

void	SyntheticApertureBeamformer::InitWork()
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

	auto* ulaop = dynamic_cast<RawSFDataSourceUlaOp*>(data_source.get());
	int decimation_factor = 1;
	if (ulaop) decimation_factor = 2;
	//printf("\n  Estimated frequency=%g MHz", estimated_f0.MHz());
	printf("\n  Estimated frequency=%g MHz", decimation_factor * estimated_f0.MHz());

	f0 = estimated_f0;
	//source_buffer->Display();
	source_buffer->HighPassFilter();

	if (!(source_original.n_rx_elements == source_original.n_tx_elements))
	{
		/*
		PlaneWaveFocuser instance;
		instance.InitWork();
		instance.Batch();
		*/
		PlaneWaveInitWork();
		PlaneWaveBatch();
	}
	else
	{
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


ComplexFunction2D_F32	SyntheticApertureBeamformer::SectorProbeDASFocus(int first_sample, int last_sample, int n_lines, RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected, RawSFDataSource &current_source)
{
	RealFunction2D_F64 matrix_correction_new = RearrangeMatrixCorrection(matrix_correction, rays_corrected, number_of_rays);
	ComplexFunction2D_F32 image(number_of_rays, n_lines, complexF32(0));
	GUIProgressBar	progress;
	std::mutex	m;
	progress.start("Focusing", number_of_transmit_elements);
	{
		auto	aperture_centre = 0.5*number_of_transmit_elements*pitch;

		for (int transmit_element_no = 0; transmit_element_no < number_of_transmit_elements; ++transmit_element_no)
		{
			auto	transmit_element_position = (transmit_element_no + 0.5)*pitch - aperture_centre;

			#pragma omp parallel for schedule (guided)
			for (int receive_element_no = 0; receive_element_no < number_of_receive_elements; ++receive_element_no)
			{
				auto	receive_element_position = (receive_element_no + 0.5)*pitch - aperture_centre;
				//auto	row = current_source.GetRow(transmit_element, receive_element);
				auto	row = current_source.GetRow(transmit_element_no + current_frame_no * number_of_transmit_elements, receive_element_no);

				ComplexFunctionF32	buffer(image.hsize(), complexF32(0));
				for (int ray_no = 0; ray_no < number_of_rays; ++ray_no)
				{
					auto	fi = (0.5 + ray_no - number_of_rays / 2)*dfi;
					// dynamic_focal_angle_transmit = true;  // отключаем фиксацию угла
					fi = dynamic_focal_angle_transmit ? fi : (fi + fixed_angle_transmit);
					double	transmit_angle = dynamic_focal_angle_transmit ? fi.radians() : fixed_angle_transmit.radians();
					double	receive_angle = dynamic_focal_angle_receive ? fi.radians() : angle_receive.radians();
					for (int line_no = 0; line_no < n_lines; line_no ++)
					{
						auto	CalculateInfluence = [&]()
						{
							int		j_depth = first_sample + line_no*((last_sample - first_sample) / n_lines);
							auto	depth = start_depth + j_depth*step_depth;
							physical_length	transmit_focal_depth_dynamic = dynamic_focal_depth_transmit ? depth : focal_depth_transmit;
							auto	transmit_path = depth - transmit_element_position*sin(transmit_angle);
							if (!plane_wave) {transmit_path += transmit_element_position*transmit_element_position / (2 * transmit_focal_depth_dynamic); }
							physical_length	receive_focal_depth_dynamic = dynamic_focal_depth_receive ? depth : focal_depth_receive;
							auto	receive_path = depth - receive_element_position*sin(receive_angle);
							if (!plane_wave) { receive_path += receive_element_position*receive_element_position / (2 * receive_focal_depth_dynamic); }
							auto	path = receive_path + transmit_path;
							//auto	t = path / current_source.sound_speed;
							auto		t = path / source_original.sound_speed;
							t += sec(matrix_correction_new.at(ray_no, transmit_element_no)) + sec(matrix_correction_new.at(ray_no, receive_element_no));
							double	sample_no = double(current_source.raw_signal_sample_rate*t);
							return row->in(sample_no - (int)current_source.first_raw_sample);
						};
						auto increment = CalculateInfluence();
						buffer.at(line_no) = increment;
					}
					std::lock_guard<std::mutex>	lock(m);
					image.row(ray_no) += buffer;
				}
			}
			++progress;
		}
	}
	progress.end();
	return image;
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
void SyntheticApertureBeamformer::CalibrationSourceMethod(RealFunction2D_F64 &matrix_correction, RealFunction2D_F64 &best_fit_matrix_correction, RealFunctionF32 &rays_corrected, physical_angle half_scanning_sector)
{
	fixed_angle_transmit = degrees(0);
	size_t ray_no = int((fixed_angle_transmit.degrees() + half_scanning_sector.degrees()) * number_of_rays / scanning_sector.degrees());
	rays_corrected[ray_no] = ray_no;
	source_buffer->InitCalibrationPulses();
	matrix_correction.row(ray_no) = source_buffer->aberration_profile_function;
	SubstractLinearShift(matrix_correction.row(ray_no));
	best_fit_matrix_correction.row(ray_no) = matrix_correction.row(ray_no);
}

void SyntheticApertureBeamformer::Sharpness(RealFunction2D_F64 &matrix_correction, RealFunctionF32 &rays_corrected)
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
					DisplayFrameSectorProbe(traits.best_fit_matrix_correction, rays_corrected, "Aberrated Before Correction");
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
		}
		catch (canceled_operation &) { }
		if (YesOrNo("Display corrected frame?", MakeGUIValue(true, saved_default_value)))
		{
			if (using_linear_array_probe) { DisplayFrameLinearProbe(traits.best_fit_matrix_correction, rays_corrected, "After"); }
			else { DisplayFrameSectorProbe(traits.best_fit_matrix_correction, rays_corrected, "After"); }
		}
		if (YesOrNo("Display matrix correction?", true)) { DisplayCorrection(traits.best_fit_matrix_correction*(2 * f0.Hz()) * 180, rays_corrected); }
		if (YesOrNo("Continue correction?", true)) {go_on = true;} else {go_on = false;}
	}
}



void	SyntheticApertureBeamformer::DisplayFrameSectorProbe(RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected, string title)
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
	image = SectorProbeDASFocus(first_sample_to_correct, last_sample_to_correct, n_lines, matrix_correction, rays_corrected, source_original);
	ScanConverterOptions sco(ScanFrameSector(start_depth * degrees(scanning_sector.degrees()).radians(), 2 * step_depth * n_lines, -half_scanning_sector, half_scanning_sector));
	ComplexFunction2D_F32 image_blured;
	image_blured = CorrectDCOffset(image);
	DisplayMathFunction2D(image_blured, title, sco);
	CaclAandSTD(image, half_scanning_sector.degrees());
}


void	SyntheticApertureBeamformer::ChooseOption(RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected)
{
	enum 
	{
		correct_aberrations, 
		calculate_rice, 
		estimate_local_sos, 
		display_exit
	} 
	option;
	while (true)
	{
		option = GetButtonDecision("Choose option",
			{
				MakeButton(L"Correct Aberrations",				correct_aberrations),
				MakeButton(L"Calculate Rice params",			calculate_rice),
				MakeButton(L"Estimate Local SoS",			estimate_local_sos),
				MakeButton("Exit",								display_exit)
			});
		try
		{
			switch (option)
			{
				case correct_aberrations:
				{
					Sharpness(matrix_correction, rays_corrected);
					if (YesOrNo("Display frame?", MakeGUIValue(false, saved_default_value))) 
					DisplayFrameSectorProbe(matrix_correction, rays_corrected, "After"); 
				}
				break;
				case calculate_rice:
				CalculateFrameRice(matrix_correction, rays_corrected, "Rice");
				break;
				case estimate_local_sos:
				Estimation_SoS(matrix_correction, rays_corrected);
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


void	SyntheticApertureBeamformer::Batch()
{
	if (!(source_original.n_rx_elements == source_original.n_tx_elements)) return;
	bool display_raw_data(false);
	if (display_raw_data == true) { data_source->Display(); }
	number_of_receive_elements = (int)source_original.n_rx_elements;
	number_of_transmit_elements = (int)source_original.n_tx_elements;
	number_of_frames = source_original.n_frames;
	aperture_width = number_of_transmit_elements*(data_source->array_pitch);
	
	if (number_of_receive_elements < 128)
	{
		number_of_rays = 128;
	}
	else
	{
		number_of_rays = number_of_receive_elements;
	}
	samples = (int)source_original.samples_per_element / 2;
	pitch = source_original.array_pitch;
	auto	sample_rate = source_original.raw_signal_sample_rate / 2;
	step_depth = 0.5*source_original.sound_speed / sample_rate;
	start_depth = 0.5*(int)source_original.first_raw_sample*source_original.sound_speed / source_original.raw_signal_sample_rate;

	dfi = scanning_sector / number_of_rays;
	RealFunction2D_F64 matrix_correction(number_of_rays, number_of_transmit_elements, 0);
	RealFunctionF32 rays_corrected(number_of_rays, -1);

	auto* verasonics = dynamic_cast<RawSFDataSourceVerasonics*>(&source_original);
	auto* verasonicsmat = dynamic_cast<RawSFDataSourceVerasonicsMat*>(&source_original);
	auto* ulaop = dynamic_cast<RawSFDataSourceUlaOp*>(&source_original);

	using_linear_array_probe = ((verasonics && verasonics->IsLinearArray()) || (ulaop && ulaop->IsLinearArray()) || (verasonicsmat && verasonicsmat->IsLinearArray()));
	if (YesOrNo("Display frame?", MakeGUIValue(false, saved_default_value)))
	{
		if (using_linear_array_probe) 
		{
			DisplayFrameLinearProbe(matrix_correction, rays_corrected, "Frame");            // для линейного датчика
		}
		else
		{
			DisplayFrameSectorProbe(matrix_correction, rays_corrected, "Frame");        // для секторного датчика
		}
	}
	ChooseOption(matrix_correction, rays_corrected);
}


XRAD_END