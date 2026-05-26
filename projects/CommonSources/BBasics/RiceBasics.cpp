#include "pre.h"

//#include "Q:\projects\ATL\Sources\SignalProcessing\SyntheticFocusingOptions.cpp"

//#include "Q:\projects\ATL\Sources\SignalProcessing\SyntheticApertureFocuserAina.h"

#include "Q:\projects\CommonSources\BBasics\RiceBasics.h"

//#include <DopplerBasics/Elasto/CFDataPhaseAnalyzerElasto.h>
//#include <DopplerBasics/CFM/CFDataPhaseAnalyzerCFM.h>
//#include <DopplerBasics/CFDataAmplitudeAnalyzer.h>

//#include <DopplerBasics/CFM/WallFilters/WallFiltersInteractive.h>

//#include <DopplerBasics/Display/DisplayDopplerData.h>
//#include <DopplerBasics/Display/DuplexDisplayer.h>


XRAD_BEGIN


void EstimateRiceMM24(const RealFunctionF64& sample, double& nu, double& sigma)
{
	double	m2(0), m4(0);
	int	s = sample.size();

	for (int i = 0; i < s; ++i)
	{
		double	vv = square(sample[i]);
		m2 += vv;
		m4 += square(vv);
	}
	m2 /= s;
	m4 /= s;

	double	nu4 = 2 * m2 * m2 - m4;
	double	nu2 = nu4 > 0 ? sqrt(nu4) : 0;
	nu = sqrt(nu2);
	sigma = sqrt(0.5 * (m2 - nu2));
}

void DisplaceSpectrum(size_t samples, ComplexFunctionMD_F32& images)
{
	double	anti_heterodyne = 0.5;
	anti_heterodyne = GetFloating("Spectral displacement factor", anti_heterodyne, -1, 1);

	for (size_t i = 0; i < samples; ++i)
	{
		ComplexFunction2D_F32	slice;
		images.GetSlice(slice, { slice_mask(0), slice_mask(1), i });

		double	phase_offset = anti_heterodyne * pi();
		complexF64	phasor = polar(1, phase_offset * i);
		slice *= phasor;
		//slice *= 0;
	}
}

RealFunctionF64	TheoreticalRiceFunction(double A, double sigma, double x0, double dx, size_t n)
{
	RealFunctionF64	rice_function(n);
	double acc(0);
	for (int i = 0; i < n; ++i)
	{
		double	x = x0 + i * dx;
		//rice_function[i] = In(x * A / (sigma * sigma), 0) * exp(-(x * x + A * A) / (2 * sigma * sigma)) * x / (sigma * sigma);
		rice_function[i] = log(x * A / (sigma * sigma)) * exp(-(x * x + A * A) / (2 * sigma * sigma)) * x / (sigma * sigma);
		acc += rice_function[i];
	}
	rice_function /= acc;
	return rice_function;
}


void CalcSampleForWindowIter(ComplexFunction2D_F32& region, RealFunctionF64& sample, ComplexFunction2D_F32 slicet, size_t first_beam_no, size_t first_sample_no)
{
	auto sample_it = sample.begin();
	for (size_t beam_no = 0; beam_no < region.vsize(); ++beam_no)
	{
		auto slice_row_it = slicet.row(first_beam_no + beam_no).begin();
		slice_row_it += first_sample_no;
		for (size_t sample_no = 0; sample_no < region.hsize(); ++sample_no, ++slice_row_it)
		{
			*sample_it = cabs(*slice_row_it);
			++sample_it;
		}
	}
}
/*
// альтернативная версия расчёта (без итератора)
void CalcSampleForWindow(ComplexFunction2D_F32& region, RealFunctionF64& sample, ComplexFunction2D_F32 slicet, size_t first_beam_no, size_t first_sample_no)
{
	size_t i(0);
	for (size_t beam_no = 0; beam_no < region.vsize(); ++beam_no)
	{
		for (size_t sample_no = 0; sample_no < region.hsize(); ++sample_no)
		{
			sample[i] = cabs(slicet.at(first_beam_no + beam_no, first_sample_no + sample_no));
			i++;
		}
	}
}
*/




void	DisplayRiceDataSyntheticAperture(ComplexFunctionMD_F32 fs, RealFunctionMD_F32 a_values, RealFunctionMD_F32 sigma_values, RealFunctionMD_F32 a_sigma_frames, size_t length_in_beams, size_t length_in_samples, ScanConverterOptions sco)
{
	
	enum
	{
		
		display_rice,
		display_a,
		display_sigma,
		display_a_sigma,
		display_b_frames,
		displace_spectrum,
		display_exit
	} option;
	while (true)
	{
		option = GetButtonDecision("Choose data to display",
			{
				MakeButton(L"B-frames",							display_b_frames),
				MakeButton(L"A",								display_a),
				MakeButton(L"SIGMA",							display_sigma),
				MakeButton(L"A/SIGMA",							display_a_sigma),
				MakeButton(L"B-fragment (Distribution curve)",	display_rice),
				MakeButton(L"Displace spectrum",				displace_spectrum),
				MakeButton("Exit",								display_exit)
			});

		//auto& sco = fs.sco_b;

		try
		{
			switch (option)
			{

			case display_b_frames:
			{
				DisplayMathFunction3D(fs, "B-frames", sco);
			}
			break;

			case display_rice:
			{
				ComplexFunctionMD_F32 rice_frames;
				MakeCopy(rice_frames, fs);
				size_t first_frame_no = GetUnsigned("first_frame_no", MakeGUIValue(30, saved_default_value), 0, fs.sizes(0) - 1);
				size_t last_frame_no = GetUnsigned("last_frame_no", MakeGUIValue(30, saved_default_value), 0, fs.sizes(0) - 1);
				size_t length_in_frames = last_frame_no - first_frame_no + 1;
				ComplexFunction2D_F32	slicet;
				rice_frames.GetSlice(slicet, { 0, slice_mask(0), slice_mask(1) });
				size_t first_beam_no = GetUnsigned("Get First Beam No", MakeGUIValue(30, saved_default_value), 0, slicet.vsize() - 1);
				size_t last_beam_no = GetUnsigned("Get Last Beam No", MakeGUIValue(first_beam_no + 30, saved_default_value), 0, slicet.vsize() - 1);
				size_t length_in_beams = last_beam_no - first_beam_no + 1;
				size_t first_sample_no = GetUnsigned("Get First Sample No", MakeGUIValue(200, saved_default_value), 0, slicet.hsize() - 1);
				size_t last_sample_no = GetUnsigned("Get Last Sample No", (first_sample_no + 200, saved_default_value), 0, slicet.hsize() - 1);
				size_t length_in_samples = last_sample_no - first_sample_no + 1;
				RealFunctionF64 full_sample(length_in_samples * length_in_beams * length_in_frames);
				ComplexFunctionMD_F32 region_3D({ length_in_frames, length_in_beams, length_in_samples }, complexF32(0));
				RealFunctionF64 sample(length_in_samples * length_in_beams);
				ComplexFunction2D_F32 region(length_in_beams, length_in_samples);
				int j(0);
				for (size_t frame_no = first_frame_no; frame_no < last_frame_no + 1; ++frame_no)
				{
					//size_t frame_no(0);
					auto	iv = { frame_no, slice_mask(0), slice_mask(1) };
					rice_frames.GetSlice(slicet, { frame_no, slice_mask(0), slice_mask(1) });
					CalcSampleForWindowIter(region, sample, slicet, first_beam_no, first_sample_no);
					for (size_t beam_no = 0; beam_no < region.vsize(); ++beam_no)
					{
						auto slice_row_it = slicet.row(first_beam_no + beam_no).begin();
						auto region_row_it = region.row(beam_no).begin();
						slice_row_it += first_sample_no;
						for (size_t sample_no = 0; sample_no < region.hsize(); ++sample_no, ++slice_row_it)
						{
							*region_row_it = *slice_row_it;
							++region_row_it;
						}
					}
					region_3D.GetSlice(iv).CopyData(region);
					for (size_t i = 0; i < sample.size(); ++i, ++j)
					{
						full_sample[j] = sample[i];
					}
				}
				double nu(0), sigma(0);
				EstimateRiceMM24(full_sample, nu, sigma);
				printf("\nfirst_beam_no = %zu, last_beam_no = %zu, \nfirst_sample_no = %zu, last_sample_no = %zu, \nA=%f, sigma=%f\n",  first_beam_no, last_beam_no, first_sample_no, last_sample_no, nu, sigma);
				DisplayMathFunction3D(region_3D, ssprintf("(%zu-%zu), (%zu-%zu),A=%f,sigma=%f", first_beam_no, last_beam_no, first_sample_no, last_sample_no, nu, sigma), sco);

				GraphSet	gs(L"histogram" + ssprintf(L" A=%f, sigma=%f", nu, sigma), L"probability", L"value");
				int n = GetUnsigned("Get n", MakeGUIValue(30, saved_default_value), 1, j);
				double xmax = MaxValue(full_sample);
				double x0 = 0;
				double	dx = (xmax - x0) / n;
				RealFunctionF64 rice_function(TheoreticalRiceFunction(nu, sigma, x0, dx, n));
				//DisplayMathFunction(rice_function, x0, dx, "TheoreticalRiceFunction");
				
				
				RealFunctionF64	histogram(n);
				Functors::absolute_value avf;
				range1_F64 absolute_range(MinValueTransformed(full_sample, avf), MaxValueTransformed(full_sample, avf));
				ComputeHistogramTransformed(region_3D, histogram, absolute_range, avf);
				gs.AddGraphUniform(histogram, absolute_range.x1() + dx / 2, dx, "Data");
				gs.Display();
				if (YesOrNo("Add Theoretical Curve?", true))
				{
					gs.AddGraphUniform(rice_function, x0, dx, "Rice");
					gs.Display();
				}		
			}
			break;

			case display_a:
			{
				DisplayMathFunction3D(a_values, ssprintf("A - Sliding window length in beams=%zu, in samples=%zu", length_in_beams, length_in_samples), sco);
			}
			break;

			case display_sigma:
			{
				DisplayMathFunction3D(sigma_values, ssprintf("SIGMA - Sliding window length in beams=%zu, in samples=%zu", length_in_beams, length_in_samples), sco);
			}
			break;

			case display_a_sigma:
			{
				DisplayMathFunction3D(a_sigma_frames, ssprintf("A/sigma - Sliding window length in beams=%zu, in samples=%zu", length_in_beams, length_in_samples), sco);
			}
			break;

			case displace_spectrum:
			{
				DisplaceSpectrum(size_t(fs.sizes(2)), fs);
				BModeRiceTestSyntheticAperture(fs, sco);
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


void ApplyThreshold(size_t n_frames, RealFunctionMD_F32 a_frames, RealFunctionMD_F32& a_sigma_frames, double a_threthold, double mean_nu, size_t v_size, size_t h_size)
{
	RealFunction2D_F32 a_slice(v_size, h_size, 0);
	RealFunction2D_F32 a_sigma_slice(v_size, h_size, 0);
	TimeProfiler	a_sigma_time;
	GUIProgressBar	progress_a_sigma;
	progress_a_sigma.start("Computing Rice Statistics", n_frames);
	a_sigma_time.Start();
	for (size_t first_frame_no = 0; first_frame_no < n_frames; ++first_frame_no)
	{
		auto	iv = { first_frame_no, slice_mask(0), slice_mask(1) };
		a_frames.GetSlice(a_slice, iv);
		a_sigma_frames.GetSlice(a_sigma_slice, iv);
		for (size_t rice_beam_no = 0; rice_beam_no < a_slice.vsize(); rice_beam_no++)
		{
			for (size_t rice_sample_no = 0; rice_sample_no < a_slice.hsize(); rice_sample_no++)
			{
				if (a_slice.at(rice_beam_no, rice_sample_no) < (a_threthold * mean_nu)) { a_sigma_slice.at(rice_beam_no, rice_sample_no) = 0; }
			}
		}
		a_sigma_frames.GetSlice(iv).CopyData(a_sigma_slice);
		++progress_a_sigma;
	}
	progress_a_sigma.end();
	a_sigma_time.Stop();
	printf("\nca_sigma_time = %g msec", a_sigma_time.MeanElapsed().msec());
	printf("\nmean_nu = %g", mean_nu);
}


void CalcRiceParams(ComplexFunctionMD_F32 b_frames, size_t n_frames, RealFunctionMD_F32& a_frames, RealFunctionMD_F32& sigma_frames, RealFunctionMD_F32& a_sigma_frames, double& mean_nu, size_t v_size, size_t h_size, size_t last_beam_no, size_t last_sample_no, size_t step_in_beams, size_t step_in_samples, size_t length_in_beams, size_t length_in_samples)
{
	RealFunction2D_F32 a_slice(v_size, h_size, 0);
	RealFunction2D_F32 sigma_slice(v_size, h_size, 0);
	RealFunction2D_F32 a_sigma_slice(v_size, h_size, 0);
	//b_slice_t::invariable	slicet;
	ComplexFunction2D_F32	slicet;

	ComplexFunction2D_F32 region(length_in_beams, length_in_samples, complexF32(0));
	RealFunctionF64 sample(length_in_samples * length_in_beams, 0);

	double nu(0), sigma(0);

	TimeProfiler	total_time, frame_time, control_time_calc_sample, control_time_estimate_rice;
	GUIProgressBar	progress;
	//progress.start("Computing Rice Statistics", n_frames);
	progress.start("Computing Rice Statistics", n_frames* last_beam_no/ step_in_beams);
	total_time.Start();
	for (size_t first_frame_no = 0; first_frame_no < n_frames; ++first_frame_no)
	{
		frame_time.Start();
		b_frames.GetSlice(slicet, { first_frame_no, slice_mask(0), slice_mask(1) });
		auto	iv = { first_frame_no, slice_mask(0), slice_mask(1) };
		for (size_t first_beam_no = 0, rice_beam_no = 0; first_beam_no < last_beam_no; rice_beam_no++, first_beam_no += step_in_beams)
		{
			for (size_t first_sample_no = 0, rice_sample_no = 0; first_sample_no < last_sample_no; rice_sample_no++, first_sample_no += step_in_samples)
			{
				control_time_calc_sample.Start();
				CalcSampleForWindowIter(region, sample, slicet, first_beam_no, first_sample_no);
				control_time_calc_sample.Stop();

				control_time_estimate_rice.Start();
				EstimateRiceMM24(sample, nu, sigma);
				control_time_estimate_rice.Stop();

				a_slice.at(rice_beam_no, rice_sample_no) = nu;
				sigma_slice.at(rice_beam_no, rice_sample_no) = sigma;
				a_sigma_slice.at(rice_beam_no, rice_sample_no) = nu / sigma;
				mean_nu += nu;
			}
			++progress;
		}
		a_frames.GetSlice(iv).CopyData(a_slice);
		sigma_frames.GetSlice(iv).CopyData(sigma_slice);
		a_sigma_frames.GetSlice(iv).CopyData(a_sigma_slice);

		frame_time.Stop();
		//++progress;
	}
	progress.end();
	total_time.Stop();

	mean_nu /= n_frames * a_slice.vsize() * a_slice.hsize();

	printf("\nTotal time of a frameset = %g sec", total_time.MeanElapsed().sec());
	printf("\nTime for a single frame = %g msec", frame_time.MeanElapsed().msec());
	printf("\ncontrol_time_calc_sample = %g msec", control_time_calc_sample.MeanElapsed().msec());
	printf("\ncontrol_time_estimate_rice = %g msec", control_time_estimate_rice.MeanElapsed().msec());
}

void GetSlidingWindowParams(double& a_threthold, size_t& sliding_window_length_in_beams, size_t& sliding_window_length_in_samples, size_t& step_in_beams, size_t& step_in_samples, ComplexFunction2D_F32 slicet)
{
	a_threthold = GetFloating("Get Threshold for A", MakeGUIValue(0.1, saved_default_value), 0, 10);
	sliding_window_length_in_beams = GetUnsigned("Get Width in Beams of the Sliding Window", MakeGUIValue(3, saved_default_value), 1, slicet.vsize() - 1);
	sliding_window_length_in_samples = GetUnsigned("Get Height in Samples of the Sliding Window", MakeGUIValue(7, saved_default_value), 1, slicet.hsize() - 1);
	printf("\n\nlength_in_beams = %zu, length_in_samples = %zu\n", sliding_window_length_in_beams, sliding_window_length_in_samples);
	step_in_beams = GetUnsigned("Get Step in Beams of the Sliding Window", MakeGUIValue(sliding_window_length_in_beams, saved_default_value), 1, sliding_window_length_in_beams);
	step_in_samples = GetUnsigned("Get Step in Samples of the Sliding Window", MakeGUIValue(sliding_window_length_in_samples, saved_default_value), 1, sliding_window_length_in_samples);
	printf("step_in_beams = %zu, step_in_samples = %zu\n", step_in_beams, step_in_samples);
}




void	BModeRiceTestSyntheticAperture(ComplexFunctionMD_F32 original_frames, ScanConverterOptions sco)
{
	//size_t n_frames(1);
	size_t n_frames(original_frames.sizes(0));
	//index_vector	frame_sizes({ n_frames, frame.vsize(), frame.hsize() });
	//ComplexFunctionMD_F32 b_frames(frame_sizes, complexF32(0));
	ComplexFunctionMD_F32 b_frames(original_frames);
	//MakeCopy(b_frames, frames.b_frames);
	//size_t	n_frames = frames.n_frames;
	//DisplayMathFunction3D(b_frames, "b_frames 0");
	//b_slice_t::invariable	slicet;
	ComplexFunction2D_F32 slicet;
	//MakeCopy(slicet, frame);
	b_frames.GetSlice(slicet, { 0, slice_mask(0), slice_mask(1) });
	//b_frames.GetSlice({ 0, slice_mask(0), slice_mask(1) }).CopyData(slicet);

	
	//DisplayMathFunction2D(slicet, "slicet");
	//DisplayMathFunction2D(frame, "frame");
//	DisplayMathFunction3D(b_frames, "b_frames", sco);
	
	double a_threthold, mean_nu(0);
	size_t sliding_window_length_in_beams, sliding_window_length_in_samples, step_in_beams, step_in_samples;
	GetSlidingWindowParams(a_threthold, sliding_window_length_in_beams, sliding_window_length_in_samples, step_in_beams, step_in_samples, slicet);
	
	size_t last_beam_no = slicet.vsize() - sliding_window_length_in_beams;
	size_t last_sample_no = slicet.hsize() - sliding_window_length_in_samples;


	index_vector	sizes({ n_frames, slicet.vsize() / step_in_beams, slicet.hsize() / step_in_samples });
	RealFunctionMD_F32 a_frames(sizes, 0);
	RealFunctionMD_F32 sigma_frames(sizes, 0);
	RealFunctionMD_F32 a_sigma_frames(sizes, 0);
	
	CalcRiceParams(b_frames, n_frames, a_frames, sigma_frames, a_sigma_frames, mean_nu, slicet.vsize() / step_in_beams, slicet.hsize() / step_in_samples, last_beam_no, last_sample_no, step_in_beams, step_in_samples, sliding_window_length_in_beams, sliding_window_length_in_samples);
	ApplyThreshold(n_frames, a_frames, a_sigma_frames, a_threthold, mean_nu, slicet.vsize() / step_in_beams, slicet.hsize() / step_in_samples);
	
	
	DisplayRiceDataSyntheticAperture(b_frames, a_frames, sigma_frames, a_sigma_frames, sliding_window_length_in_beams, sliding_window_length_in_samples, sco);
	

//	DisplayMathFunction3D(a_frames, "a_frames", sco);
//	DisplayMathFunction3D(sigma_frames, "sigma_frames", sco);
//	DisplayMathFunction3D(a_sigma_frames, "a_sigma_frames", sco);
	
	
	/*
	//	DisplayRiceData(frames, a_frames, sigma_frames, a_sigma_frames, sliding_window_length_in_beams, sliding_window_length_in_samples);
*/
}


XRAD_END