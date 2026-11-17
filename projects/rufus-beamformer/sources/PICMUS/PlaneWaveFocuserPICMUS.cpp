#include "pre.h"
#include "PICMUS/PlaneWaveFocuserPICMUS.h"
#include "CUBDL/RawSFDataSourceCUBDL.h"
#include "SignalProcessing/SyntheticApertureFocuserSectorProbe.h"
#include "PICMUS/RawSFDataSourcePICMUS.h"
#include <mutex>
#include <atomic>

XRAD_BEGIN

ComplexFunction2D_F32 SyntheticApertureBeamformer::PlaneWaveDASFocus(RawSFDataSource& source, int angle_filter)
{
	size_t n_angles_total = source.n_tx_elements;
	size_t n_rx = data_source->n_rx_elements;
	

	std::vector<size_t> angles_to_process;
	if (angle_filter < 0)
	{
		n_angles_total = GetSigned("Choose number of angels to compound", MakeGUIValue(2, saved_default_value), 1, n_angles_total);
		angles_to_process.resize(n_angles_total);
		for (size_t a = 0; a < n_angles_total; ++a) angles_to_process[a] = a;
	}
	else
	{
		angles_to_process.push_back((size_t)angle_filter);
	}

	double pitch_cm = array_Pitch.cm();
	double c_cm_sec = sound_speed.cm_sec();
	double sample_rate_hz = sample_rate.Hz();
	int    first_raw = (int)data_source->first_raw_sample;
	int    n_source_samples = (int)data_source->samples_per_element;

	printf("\n\n=== PLANE WAVE DAS ===");
	printf("\n  mode = %s", angle_filter < 0 ? "compound (all angles)" : "single angle");
	printf("\n  angles processed = %zu / %zu", angles_to_process.size(), n_angles_total);
	printf("\n  n_rx = %zu, pitch = %.4f mm, speed = %.0f m/s, sample rate = %.2f MHz", n_rx, pitch_cm * 10, sound_speed.m_sec(), sample_rate.MHz());
	printf("\n  n_rays = %d, n_samples = %d, n_source_samples = %d", n_rays, n_samples, n_source_samples);
	fflush(stdout);

	double aperture_centre = 0.5 * n_rx * pitch_cm;

	ComplexFunction2D_F32 image(n_rays, n_samples, complexF32(0));
	GUIProgressBar progress;
	std::mutex m;
	progress.start("Plane Wave DAS", angles_to_process.size());

	std::atomic<long long> out_of_range_count(0);

	for (size_t angle_idx : angles_to_process)
	{

		double theta = source.GetAngle(angle_idx);
		double sin_t = sin(theta);
		double cos_t = cos(theta);

#pragma omp parallel for schedule(guided)
		for (int rx = 0; rx < (int)n_rx; ++rx)
		{
			double x_rx = (rx + 0.5) * pitch_cm - aperture_centre;
			auto row = source.GetRow(angle_idx, rx);

			for (int i = 0; i < n_rays; ++i)
			{
				double x = (i - n_rays / 2.0) * pitch_cm;
				double dx_rx = x_rx - x;

				ComplexFunctionF32 buffer(n_samples, complexF32(0));
				for (int j = 0; j < n_samples; ++j)
				{
					double z = j * c_cm_sec / (2.0 * sample_rate_hz);

					double t_tx = (x * sin_t + z * cos_t) / c_cm_sec;
					double t_rx = sqrt(dx_rx * dx_rx + z * z) / c_cm_sec;
					double t = t_tx + t_rx;

					double sample_no = sample_rate_hz * t;
					double idx = sample_no - first_raw;

					if (idx >= 0 && idx < (double)(n_source_samples - 1))
					{
						buffer.at(j) = row->in(idx);
					}
					else
					{
						buffer.at(j) = complexF32(0);
						++out_of_range_count;
					}
				}

				std::lock_guard<std::mutex> lock(m);
				image.row(i) += buffer;
			}
		}
		++progress;
	}
	progress.end();

	if (angle_filter < 0)
		image /= double(angles_to_process.size());

	printf("\n  out_of_range_count = %lld (of %lld total samples computed)",
		(long long)out_of_range_count,
		(long long)angles_to_process.size() * n_rx * n_rays * n_samples);
	printf("\n=== Plane Wave DAS complete ===\n");
	fflush(stdout);

	return image;
}

void SyntheticApertureBeamformer::PlaneWaveInitWork()
{
	Modify_Options();
	if (YesOrNo("Display raw plane wave data before focusing?", false))
	{
		data_source->Display();
	}
	focused_data.realloc({ size_t(1), size_t(n_rays), size_t(n_samples) }, complexF32(0));
	strcpy(SIMIO::Process_Name, "Plane wave focuser");
	strcpy(SIMIO::Data_Complexity, ANALYTIC);
}

void SyntheticApertureBeamformer::PlaneWaveBatch()
{
	physical_length pitch_original(array_Pitch);

	auto* picmus_source = dynamic_cast<RawSFDataSourcePICMUS*>(data_source.get());
	auto* cubdl_source = dynamic_cast<RawSFDataSourceCUBDL*>(data_source.get());
	RawSFDataSource* source = nullptr;

	if (picmus_source) source = picmus_source;
	else if (cubdl_source) source = cubdl_source;

	if (!source)
	{
		printf("ERROR: No valid data source!\n");
		return;
	}
	bool cycle(true);
	while (cycle)
	{
		/*
		if (CapsLock())
		{
			if (YesOrNo("Adjust pitch?", true))
			{
				printf("\n pitch=%g mm", pitch_original.mm());
				array_Pitch = pitch_original * GetFloating("Choose pitch multiplier (pitch*coef)", MakeGUIValue(1.0, saved_default_value), 0, 2);
				printf("\n New pitch=%g mm", array_Pitch.mm());
			}
		}
		*/

		size_t bf_choice = Decide("Plane wave beamforming", { "Compound all angles (DAS)", "Single angle (debug/compare)" }, SavedGUIValue(size_t(0)));
		int angle_filter(-1);   // -1 = компаунд по всем углам

		if (bf_choice == 1)
		{
			int max_angle = (int)source->NumAngles() - 1;
			angle_filter = GetSigned("Angle index to focus", MakeGUIValue(max_angle / 2, saved_default_value), 0, max_angle);
		}
		ComplexFunction2D_F32 image = PlaneWaveDASFocus(*source, angle_filter);
		ScanConverterOptions sco(ScanFrameRectangle(array_Pitch * data_source->n_rx_elements, (n_samples * sound_speed / sample_rate) / 2));
		string title = (angle_filter < 0) ? "Plane Wave Compound (PICMUS, all angles)" : ssprintf("Plane Wave Single Angle #%d (PICMUS)", angle_filter);
		DisplayMathFunction2D(image, title, sco);
		
		if (CapsLock()) cycle = true;
		else cycle = false;
	}
}

XRAD_END