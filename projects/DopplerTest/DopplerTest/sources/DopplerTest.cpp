#include "pre.h"


#include <RFDataImport/S500_CFMFrameSet.h>
#include <RFDataImport/S500_CFMRawDataDisplay.h>

#include <DopplerBasics/CFM/WallFilters/WallFiltersInteractive.h>

// тесты универсальных методов доплерографии
#include "CFMModesDetailedTest.h"

// локальные тесты тонкостей алгоритмов, довольно беспорядочные
#include "SetOfPulses.h"
#include "DummyHypothesis.h"
#include "SimulateSignal.h"
//#include "BModeDetailedTest.h"
#include <Q:\projects\CommonSources\BBasics\BModeDetailedTest.h>
//#include <Q:\projects\DopplerTest\DopplerTest\sources\BModeDetailedTest.h>

//#include <Q:\projects\CommonSources\BBasics\RiceBasics.h>




XRAD_USING;

void	ChooseCFAnalysisTask(S500_CFMFrameSet &frames)
{
	size_t	frame_no = frames.n_frames / 2;
	enum 
	{ 
		cfm_modes_detailed_test, 
		b_mode_detailed_test,
//		find_blur_1d, 
//		find_blur_2d, 
		set_of_pulses, 
		exit, 
		n_options 
	} option;
	try
	{
		while(true)
		{
			option = GetButtonDecision("Choose CF analysis task",
			{
				MakeButton("CFM-modes detailed test",		cfm_modes_detailed_test),
				MakeButton("B-mode detailed test",		b_mode_detailed_test),
	//			MakeButton("Find Blur 1D", 	find_blur_1d),
	//			MakeButton("Find Blur 2D", 	find_blur_2d),
				MakeButton("Set of pulses", set_of_pulses),
				MakeButton("End with this file",			exit)
			});

			switch(option)
			{
				case cfm_modes_detailed_test:
					CFMModesDetailedTest(frames);
					break;
				case b_mode_detailed_test:
					BModeDetailedTest(frames);
					break;
	// 			case find_blur_1d:
	// 				CFMMode_find_blur_1D(frames);
	// 				break;
	// 			case find_blur_2d:
	// 				CFMMode_find_blur_2D(frames);
	// 				break;
				case set_of_pulses:
					frame_no = GetUnsigned("CFM-frame to display", MakeGUIValue(frames.n_frames / 2, saved_default_value), 0, frames.n_frames - 1);
					DisplayASetOfPulses(frames, frame_no);
					break;
				case exit:
					throw canceled_operation("Exit button");
					break;
			}
		}
	}
	catch(canceled_operation){}
	catch(quit_application){ throw; }
	catch(...) { Error(GetExceptionString()); }
}

void RawDataProcessingMenu(S500_CFMFrameSet &frames)
{
	try
	{
		enum
		{
			e_display,
			e_analyze, 
			e_exit
		};
		while (true)
		{
			auto answer = Decide(L"Choose option", 
				{ 
				MakeButton(L"Display", e_display),
				MakeButton(L"Analyze",  e_analyze), 
				MakeButton("End with this file", e_exit)
			});

			switch (answer)
			{
			case e_display:
				DisplayCFMFrameSet(frames, L"CFM frames");
				break;
			case e_analyze:
				ChooseCFAnalysisTask(frames);
				break;
			default:
				throw canceled_operation("End with this file");
			}
		}
	}
	catch (canceled_operation &) {}
	catch (quit_application &ex) { throw ex; }
	catch(...) { Error(GetExceptionString()); }
}


S500_CFMParamFileData	GetS500FileParams()
{
	wstring	filename = GetFileNameRead(L"Choose data files", saved_default_value, L"*.par");
	S500_CFMParamFileData	params;
	params.Init(filename);
	return params;
}

S500_CFMFrameSet GetS500Frames()
{
	S500_CFMParamFileData	params = GetS500FileParams();
	size_t	start_frame = 0;
	size_t	end_frame = params.NumOfFrames - 1;
	end_frame = GetUnsigned("How many frames to read?", MakeGUIValue(params.NumOfFrames, saved_default_value), 1, params.NumOfFrames) - 1;
	S500_CFMFrameSet	frames(params, start_frame, end_frame);

	bool unsweep = !CapsLock() ? true : YesOrNo("Unsweep data?", true);

	frames.ReadAllFrames(unsweep);

	return frames;
}

S500_CFMFrameSet InitLargeDataset(S500_CFMParamFileData params, S500_CFMParamFileData &buffer_params)
{
	buffer_params.NumOfBBeams = 300;
	buffer_params.SizeofBBeamAtSamples = 1000;
	buffer_params.NumOfCFMBeams = 200;
	buffer_params.SizeofCFMBeamAtSamples = 500;
	buffer_params.NumOfFrames = GetUnsigned("How many frames there are in a cumulative dataset?", MakeGUIValue(params.NumOfFrames, saved_default_value), 1, 10000) - 1;
	S500_CFMFrameSet	buffer_frames(buffer_params, 0, buffer_params.NumOfFrames - 1);
	buffer_frames.b_frames.fill(complexF32(1));
	buffer_frames.cfm_frames.fill(complexF32(1));
	return buffer_frames;
}

void AddDataset(size_t local_end_frame, size_t &last_frame_no, S500_CFMFrameSet local_frames, S500_CFMFrameSet &buffer_frames, S500_CFMParamFileData local_params)
{
//	DisplayMathFunction3D(local_frames.b_frames, ssprintf("b_frames original"));
//	DisplayMathFunction3D(local_frames.cfm_frames, ssprintf("cfm_frames original"));

	for (size_t local_frame_no = 0; local_frame_no < (local_end_frame + 1); ++local_frame_no)
	{
		auto local_slice_b = local_frames.b_frames.GetSlice({ local_frame_no, slice_mask(0), slice_mask(1) });
		auto buffer_slice_b = buffer_frames.b_frames.GetSlice({ last_frame_no + local_frame_no, slice_mask(0), slice_mask(1) });

	//	DisplayMathFunction2D(local_slice_b, "local_slice_b");
		
		for (size_t local_beam_no = 0; local_beam_no < local_params.NumOfBBeams; ++local_beam_no)
		{
			for (size_t local_sample_no = 0; local_sample_no < local_params.SizeofBBeamAtSamples; ++local_sample_no)
			{
				buffer_slice_b.at(local_beam_no, local_sample_no) = local_slice_b.at(local_beam_no, local_sample_no);
			}
		}

	//	DisplayMathFunction2D(buffer_slice_b, "buffer_slice_b");

	//	DisplayMathFunction3D(buffer_frames.b_frames, ssprintf("b_frames"));

		auto local_slice_cfm = local_frames.cfm_frames.GetSlice({ local_frame_no, slice_mask(0), slice_mask(1) });
		auto buffer_slice_cfm = buffer_frames.cfm_frames.GetSlice({ last_frame_no + local_frame_no, slice_mask(0), slice_mask(1) });

		for (size_t local_beam_no = 0; local_beam_no < (local_params.NumOfCFMBeams * local_params.NumOfCFShots); ++local_beam_no)
		{
			for (size_t local_sample_no = 0; local_sample_no < local_params.SizeofCFMBeamAtSamples; ++local_sample_no)
			{
				//buffer_slice_cfm.at(local_sample_no, local_beam_no) = local_slice_cfm.at(local_sample_no, local_beam_no);
				buffer_slice_cfm.at(local_beam_no, local_sample_no) = local_slice_cfm.at(local_beam_no, local_sample_no);
			}
		}
	}
	last_frame_no += local_end_frame + 1;

//	DisplayMathFunction3D(buffer_frames.b_frames, ssprintf("b_frames"));
//	DisplayMathFunction3D(buffer_frames.cfm_frames, ssprintf("cfm_frames"));


}

S500_CFMFrameSet LoadSeveralDatasets()
{
	S500_CFMParamFileData	params = GetS500FileParams();
	size_t end_frame = GetUnsigned("How many frames to read?", MakeGUIValue(params.NumOfFrames, saved_default_value), 1, params.NumOfFrames) - 1;
	S500_CFMFrameSet	frames(params, 0, end_frame);
	bool unsweep = !CapsLock() ? true : YesOrNo("Unsweep data?", true);
	frames.ReadAllFrames(unsweep);
	size_t last_frame_no(0);	
	S500_CFMParamFileData	buffer_params(params);
	S500_CFMFrameSet	buffer_frames = InitLargeDataset(params, buffer_params);
	AddDataset(end_frame, last_frame_no, frames, buffer_frames, params);

	while (YesOrNo("Read another dataset?", true))
	{
		S500_CFMParamFileData	local_params = GetS500FileParams();
		size_t local_end_frame = GetUnsigned("How many frames to read?", MakeGUIValue(local_params.NumOfFrames, saved_default_value), 1, local_params.NumOfFrames) - 1;
		S500_CFMFrameSet	local_frames(local_params, 0, local_end_frame);
		local_frames.ReadAllFrames(unsweep);
		AddDataset( local_end_frame, last_frame_no, local_frames, buffer_frames, local_params);
	}
	return buffer_frames;
}

void	WallFilterFrequencyResponse()
{
	size_t N = GetUnsigned("Burst Size", SavedGUIValue(16), 4, 1024);
	ComplexFunctionF32 pulse(N, complexF32(0));

	while(true)
	{
		pulse[pulse.size() / 2] = complexF32(1);
		ComplexFunctionF32	filtered(pulse);

		WallFilterPtr	wall_filter = GetWallFilterInteractive(pulse.size());
		if(wall_filter->filter_name()==L"WallFilterNone") break;

		wall_filter->Apply(filtered);

		while(true)
		{
			auto	fn = GetButtonDecision(
				L"Display option",
				{
					MakeButton(L"Original pulse", make_fn([&pulse](){DisplayMathFunction(pulse, 0, 1, L"Pulse");})),
					MakeButton(L"WallFilter result",make_fn([&filtered, &wall_filter](){DisplayMathFunction(filtered, 0, 1, L"Filter applied: "+ wall_filter->filter_name());})),
					MakeButton(L"Exit", function<void()>())
				}
			);

			if(fn) fn();
			else break;
		}
	}
}

void dummy()
{
	GraphSet	gs(L" 'histogram'", L"probability", L"value");
	double nu(0), sigma(0), step (0);
	do
	{
		int n = GetUnsigned("Get n", 1, 1, 100);
		n = n * 1024;
		double	xmax = GetFloating("Get x max", 10, 0.01, 10000000);
		double A = GetFloating("Get A", 1, 0, 100000);
		double sigma = GetFloating("Get sigma", .5, 0, 10000000);
		double x0 = GetFloating("Get x0", .3, 0, 10000000);
		double	dx = (xmax - x0) / n;

		RealFunctionF64 rice_function(TheoreticalRiceFunction(A, sigma, x0, dx, n));
		DisplayMathFunction(rice_function, 0, 1, L"probability", L"abs. value");
		gs.AddGraphUniform(rice_function, x0, dx, "Rice" + ssprintf(" A=%f, sigma=%f", nu, sigma));
		gs.Display();
	} while (YesOrNo("Repeat?", 1));
}

/*
ComplexFunctionF32 GenerateVectorOfScatterers(size_t N, size_t step)  		// генерируем массив рассеивателей
{
	ComplexFunctionF32 vector_of_scatterers(N, complexF32(0));
	complexF32 c(0);
	double acc(0);
	for (size_t i = 0; i < N; i += step)
	{
		c = polar(1, RandomUniformF64(0, 2 * pi()));
		vector_of_scatterers[i] = c;
		acc += cabs(c);
	}
	vector_of_scatterers /= acc; // этим мы добиваемся того, что сумма амплитуд остается неизменной при изменении шага между рассеивателями
	return vector_of_scatterers;
}

ComplexFunctionF32 GenerateScanningPulse(size_t pulse_size) 		// генерируем зондирующий импульс
{
	ComplexFunctionF32 scanning_pulse(pulse_size, complexF32(0));
	size_t number_of_zeroes = pulse_size / 10;
	for (size_t i = number_of_zeroes; i < (pulse_size - number_of_zeroes); ++i)
	{
		scanning_pulse[i] = polar(1, pi() / 4);
	}
	return scanning_pulse;
}

RealFunctionF32 Conjugation(ComplexFunctionF32 &vector_of_scatterers, ComplexFunctionF32 &scanning_pulse)       // считаем свертку зондирующего импульса и массива рассеивателей
{
//	DisplayMathFunction(vector_of_scatterers, 0, 1, L"vector_of_scatterers_norm_Conj", L"i");
//	DisplayMathFunction(scanning_pulse, 0, 1, L"scanning_pulse_Conj", L"i");
	size_t N(vector_of_scatterers.size());
	size_t pulse_size(scanning_pulse.size());
	RealFunctionF32 conjugation(N - pulse_size, 0);
	ComplexFunctionF32 conjugation_c(N - pulse_size, complexF32(0));
	GUIProgressBar	prog;
	prog.start("Conjugate", conjugation.size());
	for (size_t i = 0; i < conjugation.size(); ++i)
	{
		// цикл по глубинам ЦДК
		complexF64	numerator(0);
		for (size_t p = 0; p < pulse_size; ++p)
		{
			numerator.add_multiply_conj(vector_of_scatterers[p + i], scanning_pulse[p]);
		}
		conjugation[i] = cabs(numerator);
		conjugation_c[i] = numerator;
		++prog;
	}
	prog.end();

	DisplayMathFunction(conjugation_c, 0, 1, L"conjugation_c", L"i");
//	DisplayMathFunction(conjugation, 0, 1, L"conjugation", L"i");
	return conjugation;
}

RealFunctionF64	ComputeHistogram(RealFunctionF32 &full_sample)   // строим гистограмму
{
	int hist_size = 100;
	double xmax = MaxValue(full_sample);
	RealFunctionF64	histogram(hist_size);
	Functors::absolute_value avf;
	//range1_F64 absolute_range(MinValueTransformed(full_sample, avf), MaxValueTransformed(full_sample, avf));
	range1_F64 absolute_range(0, xmax);
	ComputeHistogram(full_sample, histogram, absolute_range);
	printf("xmax = %g /n", xmax);
	return histogram;
}

void RiceMathModel()
{
	GraphSet	gs(L" 'histogram'", L"probability", L"value");
	do
	{
//		size_t N = 30000; // 524288 - соответствует глубине 5 см
		//size_t N = 524288; // 524288 - соответствует глубине 5 см
		size_t N = 500000; // 524288 - соответствует глубине 5 см
		//size_t pulse_size = 8192; // 8192 - соответствует длине зондирующего испульса 0,8 мм
		//size_t pulse_size = 10000; // 8192 - соответствует длине зондирующего испульса 0,8 мм
		size_t pulse_size = 1000; // 8192 - соответствует длине зондирующего испульса 0,8 мм
		size_t step = GetUnsigned("Get step", MakeGUIValue(10, saved_default_value), 10, 10000);
		N = N * GetUnsigned("Get n", MakeGUIValue(1, saved_default_value), 1, 100);
		ComplexFunctionF32 vector_of_scatterers = GenerateVectorOfScatterers(N, step);
		DisplayMathFunction(vector_of_scatterers, 0, 1, L"vector_of_scatterers_norm", L"i");
		ComplexFunctionF32 scanning_pulse = GenerateScanningPulse(pulse_size);
		DisplayMathFunction(scanning_pulse, 0, 1, L"scanning_pulse", L"i");
		RealFunctionF32 full_sample = Conjugation(vector_of_scatterers, scanning_pulse);
		RealFunctionF64	histogram = ComputeHistogram(full_sample);
		printf("step %g /n", (step / double(pulse_size)));
		gs.AddGraphUniform(histogram, 0, 1 / double(histogram.size()), ssprintf("%g", (step / double(pulse_size))));
		gs.Display();

	} while (YesOrNo("Repeat?", 1));
}
*/


ComplexFunctionF32 GenerateVectorOfScatterers(size_t N, double step)  		// генерируем массив рассеивателей
{
	ComplexFunctionF32 vector_of_scatterers(N, complexF32(0));
	complexF32 c(0);
	double acc(0);
	for (size_t i = 0; i < N; i += step)
	{
		c = polar(1, RandomUniformF64(0, 2 * pi()));
		vector_of_scatterers[i] = c;
		acc += cabs(c);
	}
	vector_of_scatterers /= acc; // этим мы добиваемся того, что сумма амплитуд остается неизменной при изменении шага между рассеивателями
	return vector_of_scatterers;
}

ComplexFunctionF32 GenerateScanningPulse(size_t pulse_size, size_t shape) 		// генерируем зондирующий импульс
{
	ComplexFunctionF32 scanning_pulse(pulse_size, complexF32(0));
	size_t number_of_zeroes = pulse_size / 10;
	//double denominator(5);
	//double std(scanning_pulse.size() / denominator);
	double std = 800 / sqrt(12);
	if (shape == 4) std *= 0.25;
	if ((shape==0) || (shape == 3))
	{
		for (size_t i = number_of_zeroes; i < (pulse_size - number_of_zeroes); ++i)
		{
			scanning_pulse[i] = polar(1, pi() / 4);
		}
	}
	if ((shape == 1) || (shape == 2) || (shape == 4))
	{
		for (size_t i = 0; i < scanning_pulse.size(); ++i)   //формируем огибающую формы Гаусса
		{
			if (i > scanning_pulse.size() / 2)
			{
				scanning_pulse[i].re = gauss(i - scanning_pulse.size() / 2, std);
				scanning_pulse[i].im = gauss(i - scanning_pulse.size() / 2, std);
			}
			else
			{
				scanning_pulse[i].re = gauss(-i + scanning_pulse.size() / 2, std);
				scanning_pulse[i].im = gauss(-i + scanning_pulse.size() / 2, std);
			}
		}
		//scanning_pulse /= sqrt(2);
	}
	if ((shape == 2) || (shape == 3) || (shape == 4))   //задаем ВЧ заполнение
	{
		for (size_t i = 0; i < scanning_pulse.size(); ++i)
		{
			scanning_pulse[i] *= polar(1, double(i)*2*pi()/ std);
		}
	}
	return scanning_pulse;
}

RealFunctionF32 Conjugation(ComplexFunctionF32& vector_of_scatterers, ComplexFunctionF32& scanning_pulse)       // считаем свертку зондирующего импульса и массива рассеивателей
{
	//	DisplayMathFunction(vector_of_scatterers, 0, 1, L"vector_of_scatterers_norm_Conj", L"i");
	//	DisplayMathFunction(scanning_pulse, 0, 1, L"scanning_pulse_Conj", L"i");
	size_t N(vector_of_scatterers.size());
	size_t pulse_size(scanning_pulse.size());
	RealFunctionF32 conjugation(N - pulse_size, 0);
	ComplexFunctionF32 conjugation_c(N - pulse_size, complexF32(0));
	GUIProgressBar	prog;
	prog.start("Conjugate", conjugation.size());
	for (size_t i = 0; i < conjugation.size(); ++i)
	{
		// цикл по глубинам ЦДК
		complexF64	numerator(0);
		for (size_t p = 0; p < pulse_size; ++p)
		{
			numerator.add_multiply_conj(vector_of_scatterers[p + i], scanning_pulse[p]);
		}
		conjugation[i] = cabs(numerator);
		conjugation_c[i] = numerator;
		++prog;
	}
	prog.end();

	//DisplayMathFunction(conjugation_c, 0, 1, L"conjugation_c", L"i");
	return conjugation;
}

RealFunctionF64	ComputeHistogram(RealFunctionF32& full_sample)   // строим гистограмму
{
	int hist_size = 100;
	double xmax = MaxValue(full_sample);
	RealFunctionF64	histogram(hist_size);
	Functors::absolute_value avf;
	//range1_F64 absolute_range(MinValueTransformed(full_sample, avf), MaxValueTransformed(full_sample, avf));
	range1_F64 absolute_range(0, xmax);
	ComputeHistogram(full_sample, histogram, absolute_range);

	/*
			int hist_size = 50;
		double xmax = MaxValue(full_sample);
		double xmin = MinValue(full_sample);
		RealFunctionF64	histogram(hist_size);
		Functors::absolute_value avf;
	//	range1_F64 values_range(MinValueTransformed(full_sample, avf), MaxValueTransformed(full_sample, avf));
		//ComputeHistogram(full_sample, histogram, absolute_range);

		double	index_factor = double(histogram.size()) / double(xmax-xmin);
		double	increment = 1. / (full_sample.size());
		for (size_t i = 0; i < full_sample.size(); ++i)
		{
			size_t	index = range(ptrdiff_t(index_factor * (full_sample[i] - xmin)), 0, ptrdiff_t(histogram.size()) - 1);
			histogram[index] += increment;
		}
	*/
	printf("xmax = %g /n", xmax);
	return histogram;
}

void EstimateNakagamiMM24(const RealFunctionF64& sample, double& nu, double& sigma)
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

	nu = m2 * m2 / (m4 - m2 * m2);
	sigma = m2;
}

void EstimateDistributionParams(size_t& count, RealFunctionF32& full_sample, RealFunctionF32& vec_A, RealFunctionF32& vec_sigma, RealFunctionF32& vec_A_div_sigma, RealFunctionF32& vec_sigma_div_A)
{
	double A(0), sigma(0);
	EstimateRiceMM24(full_sample, A, sigma);
	//EstimateNakagamiMM24(full_sample, A, sigma);
	vec_A[count-1] = A;
	vec_sigma[count-1] = sigma;
	vec_A_div_sigma[count-1] = A / sigma;
	if (!(A == 0)) vec_sigma_div_A[count - 1] = sigma / A;
	else vec_sigma_div_A[count - 1] = 0;
}

void RiceMathModel()
{
	GraphSet	gs(L"histogram", L"probability", L"value");

	GraphSet	gs_step(L"step", L"step", L"value");
	GraphSet	gs_A(L"A", L"A", L"value");
	GraphSet	gs_sigma(L"sigma", L"sigma", L"value");
	GraphSet	gs_A_div_sigma(L"A/sigma", L"A/sigma", L"value");
	GraphSet	gs_sigma_div_A(L"sigma/A", L"sigma/A", L"value");
	RealFunctionF32 vec_step(1, 0), A(1, 0), sigma(1, 0), A_div_sigma(1, 0), sigma_div_A(1, 0);
	size_t count(1);
	size_t pulse_shape(2); // 0 - прямоугольный, 1 - гаус, 2 - гаус с ВЧ заполнением длительностью 2 сигмы, 3 - прямоугольник с заполнением, 4 - гаус с ВЧ заполнением длительностью 3 сигмы

	do
	{
		size_t N = 50000000; // 524288 - соответствует глубине 5 см
		size_t pulse_size = 1000; // 8192 - соответствует длине зондирующего испульса 0,8 мм
		if (!((pulse_shape == 0)|| (pulse_shape == 3))) pulse_size = 1400;
		
		
		size_t step = GetUnsigned("Get step", MakeGUIValue(10, saved_default_value), 10, 10000);
		N = N * GetUnsigned("Get n", MakeGUIValue(1, saved_default_value), 1, 100);
		ComplexFunctionF32 vector_of_scatterers = GenerateVectorOfScatterers(N, step);
		DisplayMathFunction(vector_of_scatterers, 0, 1, L"vector_of_scatterers_norm", L"i");
		ComplexFunctionF32 scanning_pulse = GenerateScanningPulse(pulse_size, pulse_shape);
		DisplayMathFunction(scanning_pulse, 0, 1, L"scanning_pulse", L"i");
		RealFunctionF32 full_sample = Conjugation(vector_of_scatterers, scanning_pulse);

//		traits.lineGS.SetScale(range2_F64(0, double(-scanning_sector.degrees() / 2), MaxValue(line_original), double(scanning_sector.degrees() / 2)));
//		traits.approximationGS.Display(false);
//		traits.approximationGS.ChangeGraphUniform(0, traits.matrix_correction.row(traits.ray_no), 0, 1, ssprintf("current=%g", traits.current_frequency));


		vec_step[count-1] = step;
		EstimateDistributionParams(count, full_sample, A, sigma, A_div_sigma, sigma_div_A);

		gs_step.ChangeGraphUniform(0, vec_step, 0, 1, L"vec_step");
		gs_step.Display();

		gs_A.ChangeGraphUniform(0, A, 0, 1, L"A");
		gs_A.Display();

		gs_sigma.ChangeGraphUniform(0, sigma, 0, 1, L"sigma");
		gs_sigma.Display();

		gs_A_div_sigma.ChangeGraphUniform(0, A_div_sigma, 0, 1, L"A_div_sigma");
		gs_A_div_sigma.Display();

		gs_sigma_div_A.ChangeGraphUniform(0, sigma_div_A, 0, 1, L"sigma_div_A");
		gs_sigma_div_A.Display();


		RealFunctionF64	histogram = ComputeHistogram(full_sample);
		printf("step %g /n", (step / double(pulse_size)));
		gs.AddGraphUniform(histogram, 0, 1 / double(histogram.size()), ssprintf("%g", (step / double(pulse_size))));
		gs.Display();

		count++;

		RealFunctionF32 vec_step_buf(vec_step);
		vec_step.realloc(count); 
		for (size_t i = 0; i < vec_step_buf.size(); ++i)
		{
			vec_step[i] = vec_step_buf[i];
		}
		
		RealFunctionF32 A_buf(A);
		A.realloc(count); 
		for (size_t i = 0; i < vec_step_buf.size(); ++i)
		{
			A[i] = A_buf[i];
		}

		RealFunctionF32 sigma_buf(sigma);
		sigma.realloc(count); 
		for (size_t i = 0; i < vec_step_buf.size(); ++i)
		{
			sigma[i] = sigma_buf[i];
		}

		RealFunctionF32 A_div_sigma_buf(A_div_sigma);
		A_div_sigma.realloc(count);
		for (size_t i = 0; i < vec_step_buf.size(); ++i)
		{
			A_div_sigma[i] = A_div_sigma_buf[i];
		}

		RealFunctionF32 sigma_div_A_buf(sigma_div_A);
		sigma_div_A.realloc(count); 
		for (size_t i = 0; i < vec_step_buf.size(); ++i)
		{
			sigma_div_A[i] = sigma_div_A_buf[i];
		}

	} while (YesOrNo("Repeat?", 1));
}



void RiceMathModelBatch()
{
	GraphSet	gs_step(L"step", L"step", L"value");
	GraphSet	gs_A(L"A", L"A", L"value");
	GraphSet	gs_sigma(L"sigma", L"sigma", L"value");
	GraphSet	gs_A_div_sigma(L"A/sigma", L"A/sigma", L"value");
	GraphSet	gs_sigma_div_A(L"sigma/A", L"sigma/A", L"value");

	gs_step.Display();
	gs_A.Display();
	gs_sigma.Display();
	gs_A_div_sigma.Display();
	gs_sigma_div_A.Display();
	
	size_t pulse_shape(4); // 0 - прямоугольный, 1 - гаус, 2 - гаус с ВЧ заполнением длительностью 2 сигмы, 3 - прямоугольник с заполнением, 4 - гаус с ВЧ заполнением длительностью 3 сигмы
	RealFunctionF32 vec_step(1, 0), A(1, 0), sigma(1, 0), A_div_sigma(1, 0), sigma_div_A(1, 0);
	size_t count(1), N(5000000), pulse_size(1000);
	//size_t count(1), N(500000), pulse_size(1000);
	if (((pulse_shape == 1) || (pulse_shape == 2))) pulse_size = 1400;
	

	double	increment_koef = GetFloating("Get increment_koef", 1.01, 1.001, 10);
	N = N * GetUnsigned("Get n", MakeGUIValue(1, saved_default_value), 1, 100);
	size_t init_step(10);
	size_t max_step = 3 * pulse_size;
	size_t number_of_steps = log(max_step / init_step) / log(double(increment_koef));
	gs_step.SetScale(range2_F64(0, 0, max_step, number_of_steps));
	//gs_A.SetScale(range2_F64(0, 0, 2, number_of_steps));
	GUIProgressBar	prog_rice_batch;
	prog_rice_batch.start("RiceBatch", number_of_steps);
	//for (size_t step = init_step; step < max_step; step *= increment_koef)
	for (double step = init_step; step < max_step; step *= increment_koef)
	{

		ComplexFunctionF32 vector_of_scatterers = GenerateVectorOfScatterers(N, step);
		//DisplayMathFunction(vector_of_scatterers, 0, 1, L"vector_of_scatterers_norm", L"i");
		ComplexFunctionF32 scanning_pulse = GenerateScanningPulse(pulse_size, pulse_shape);
		//DisplayMathFunction(scanning_pulse, 0, 1, L"scanning_pulse", L"i");
		RealFunctionF32 full_sample = Conjugation(vector_of_scatterers, scanning_pulse);

		vec_step[count-1] = step;
		EstimateDistributionParams(count, full_sample, A, sigma, A_div_sigma, sigma_div_A);

		gs_step.ChangeGraphUniform(0, vec_step, 0, 1, L"vec_step");
		gs_A.ChangeGraphUniform(0, A, 0, 1, L"A");
		gs_sigma.ChangeGraphUniform(0, sigma, 0, 1, L"sigma");
		gs_A_div_sigma.ChangeGraphUniform(0, A_div_sigma, 0, 1, L"A_div_sigma");
		gs_sigma_div_A.ChangeGraphUniform(0, sigma_div_A, 0, 1, L"sigma_div_A");

		count++;

		RealFunctionF32 vec_step_buf(vec_step);
		vec_step.realloc(count); 
		for (size_t i = 0; i < vec_step_buf.size(); ++i)
		{
			vec_step[i] = vec_step_buf[i];
		}

		RealFunctionF32 A_buf(A);
		A.realloc(count); 
		for (size_t i = 0; i < vec_step_buf.size(); ++i)
		{
			A[i] = A_buf[i];
		}

		RealFunctionF32 sigma_buf(sigma);
		sigma.realloc(count); 
		for (size_t i = 0; i < vec_step_buf.size(); ++i)
		{
			sigma[i] = sigma_buf[i];
		}

		RealFunctionF32 A_div_sigma_buf(A_div_sigma);
		A_div_sigma.realloc(count); 
		for (size_t i = 0; i < vec_step_buf.size(); ++i)
		{
			A_div_sigma[i] = A_div_sigma_buf[i];
		}

		RealFunctionF32 sigma_div_A_buf(sigma_div_A);
		sigma_div_A.realloc(count);  
		for (size_t i = 0; i < vec_step_buf.size(); ++i)
		{
			sigma_div_A[i] = sigma_div_A_buf[i];
		}
		++prog_rice_batch;
	}
	prog_rice_batch.end();
}






int xrad::xrad_main(int, char** const)
{


	//dummy();
	//RiceMathModel();
	//RiceMathModelBatch();

	XRAD_USING

	try
	{
		size_t	answer(0);
		while (true)
		{
			answer = Decide("Choose source", { "Process single raw dataset", "Process several raw datasets","Process simulated signal", "Wall filter frequency response", "Test dummy hypothesis", "Exit" });
			switch (answer)
			{
				case 0:
				{
					try
					{
						S500_CFMFrameSet frames = GetS500Frames();
						RawDataProcessingMenu(frames);
					}
					catch(canceled_operation){}
					catch(...){ Error(GetExceptionStringOrRethrow()); }
				}
				break;

				case 1:
				{
					try
					{
						S500_CFMFrameSet frames = LoadSeveralDatasets();
						RawDataProcessingMenu(frames);
					}
					catch (canceled_operation) {}
					catch (...) { Error(GetExceptionStringOrRethrow()); }
				}
				break;

				case 2:
				{
					try
					{
						S500_CFMFrameSet frames = SimulateSignal();
						RawDataProcessingMenu(frames);
					}
					catch(canceled_operation){}
					catch(...){ Error(GetExceptionStringOrRethrow()); }
				}
				break;

				case 3:
					WallFilterFrequencyResponse();
					break;
				break;

				case 4:
					DummyHypothesis();
					break;

				default:
					throw canceled_operation("");
			}
		}
	}
	catch (canceled_operation &) {}
	catch (quit_application &) {}
	catch (...) 
	{ 
		Error(GetExceptionString()); 
		return 1;
	}
	return 0;
}

/*
void rufus_doppler_menu(int, char** const)
{
	int arg1;
	char** arg2;
	xrad_main(arg1, arg2);
}


int run_rufus_doppler(int argc, char** argv)
{
	XRAD_USING

		try {
		int arg1;
		char** arg2;
		xrad_main(arg1, arg2);
		ShowText("rufus-doppler", "Doppler mode (not fully integrated yet)");
		return 0;
	}
	catch (canceled_operation&) {
		return 0;
	}
	catch (...) {
		Error("Unknown exception");
		return 1;
	}
}
*/