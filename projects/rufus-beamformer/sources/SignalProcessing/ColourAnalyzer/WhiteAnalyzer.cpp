#include "pre.h"

#if 1
// Файл связан с ColourAnalyzer.h/cpp. см. комментарии в нем
#include <Attic/LABColorImage.h>
#include <XRADBasic/Sources/Utils/LeastSquares.h>

#include "SignalProcessing/GaussSignalTools.h"

#include "WhiteAnalyzer.h"
#include <Utils/SignalFilters.h>
#include <XRADBasic/Sources/SampleTypes/HLSColorSample.h>
#include "ColourEncode.h"
#include "ColourAnalyzerUtils.h"


XRAD_BEGIN


//void	WhiteAnalyzer :: ProcessInitDialog(){}

WhiteAnalyzer :: WhiteAnalyzer() : RangeSpectrumAnalyzer(32)
	{
	strcpy(SIMIO::Process_Name,"WhiteAnalyzer");
	//SetDepthUnits(CENTIMETRES);
	}

WhiteAnalyzer :: ~WhiteAnalyzer()
	{
	}


void	WhiteAnalyzer :: InitWork()
{
	parent :: InitWork();
	size_t min_range_size = max(n_samples/n_ranges, size_t(16));	
	range_length = GetSigned("Range length", default_range_length, min_range_size, n_samples);
	CalculateWhite(default_range_length, white_spectra);
	}

//--------------------------------------------------------------------------

void	WhiteAnalyzer :: EndWork()
	{
	}




//-------------------------------------------------------------

void	WhiteAnalyzer :: AnalyzeLocalSpectra()
	{
//	extern	Point	WhereMouseLocal;
//	Point	wm;
//	SetFrequencyUnits(M_HERZ);
	float	dw = sample_rate.MHz()/range_length;

	GraphSet	gs("Spectra", "W", "");
	GraphSet	gs1("Normalized Spectra", "W", "");
	GraphSet	gs2("Acquisition Spectra", "W", "");

	
	ComplexFunctionF32	fft_buffer(range_length);

	RealFunctionF32	white_component(range_length), colour_component(range_length), sample_spectrum(range_length);
	RealFunctionF32	acq_white_component(range_length), acq_colour_component(range_length), acq_sample_spectrum(range_length);
	
	ComplexFunctionF32	step_spectrum(range_length);
	RealFunctionF32	step_spectrum_abs(range_length/2);
	
//	double	a1 = 0.9;
//	double	a2 = 1. - a1;
	
	gs.AddGraphUniform(white_component, 0, dw, "White component");
	gs.AddGraphUniform(colour_component, 0, dw, "Colourized");
	gs.AddGraphUniform(white_spectra.row(0), 0, dw, "Original spectrum");
	
	gs1.AddGraphUniform(white_spectra.row(0), 0, dw, "Normalized white component");
	gs1.AddGraphUniform(white_spectra.row(0), 0, dw, "Normalized colour spectrum");
	gs1.AddGraphUniform(white_spectra.row(0), 0, dw, "Normalized original spectrum");

//	gs2.AddGraphUniform(white_spectra.row(0), 0, dw, "Acquisition white component");
//	gs2.AddGraphUniform(white_spectra.row(0), 0, dw, "Acquisition colour spectrum");
	gs2.AddGraphUniform(step_spectrum_abs, 0, 2*sound_speed.cm_sec()/(dw*1.e6*range_length), "Step spectrum");

	DisplayMathFunction2D(sample_brightness, "sample brightness", sco);
	
	#if 0
	SetCurrentFrame(0);
	while(!CapsLock())
		{
		if(wm.h != WhereMouseLocal.h || wm.v != WhereMouseLocal.v)
			{
			printf("row=%d\tcol=%d;\t", WhereMouseLocal.v, WhereMouseLocal.h);
			fflush(stdout);

			row_col_coord	rc(WhereMouseLocal.v, WhereMouseLocal.h);
			ray_sample_coord rs = sample_brightness.GetRaySampleCoords(rc);
			size_t ray = rs.ray_f;
			size_t sample = rs.sample_f;
			size_t range_no = sample/range_step;

			printf("ray = %d\tsample=%d\trange=%d\n", ray, sample, range_no);
			fflush(stdout);
			
			if(in_range(range_no, 0, n_ranges-1) && in_range(ray, 0, n_rays-1))
				{
				size_t j0 = range(sample - range_length/2, 0, n_samples-range_length);
				
				SetCurrentRay(ray);
				for(size_t j = 0; j < range_length; j++)
					{
					fft_buffer[j] = CurrentRay[j+j0];
					}
				ApplyWindowFunction(fft_buffer, cos2_win());
				fft_buffer.FFT(ftForward);

				CopyData(sample_spectrum, fft_buffer, cabs_functor());
				ExtractWhiteComponent(sample_spectrum, white_spectra[range_no], white_component, colour_component);
				if(1)
					{
					CopyData(step_spectrum, colour_component);
					ApplyFunction(step_spectrum, cabs_functor());
					step_spectrum /= white_spectra[range_no];
					ApplyWindowFunction(step_spectrum, cos2_win());
					step_spectrum.FFT(ftForward);
					
					CopyData(step_spectrum_abs, step_spectrum, cabs_functor());
					size_t n = 0;
					
					if(ControlPressed())
						{
						while(n < range_length/2 && step_spectrum_abs[n] > step_spectrum_abs[n+1])
							{
							n++;
							// выделение нулевого пика
							}
						for(size_t i = 0; i < n; i++) step_spectrum_abs[i] = 0;
						}
					
					gs2.ChangeGraph(0, step_spectrum_abs, 0, dw, "Step spectrum");
					}

				ApplyFunction(colour_component, fabs_functor());
				gs.ChangeGraph(0, white_component, 0, dw, "White component");
				gs.ChangeGraph(1, colour_component, 0, dw, "Colourized");
				gs.ChangeGraph(2, sample_spectrum, 0, dw, "Original spectrum");

				white_component /= white_spectra[range_no];
				colour_component /= white_spectra[range_no];
				sample_spectrum /= white_spectra[range_no];
				
				gs1.ChangeGraph(0, white_component, 0, dw, "Normalized white component");
				gs1.ChangeGraph(1, colour_component, 0, dw, "Normalized colour spectrum");
				gs1.ChangeGraph(2, sample_spectrum, 0, dw, "Normalized original spectrum");
				
				acq_white_component *= a1;
				white_component *= a2;
				acq_white_component += white_component;

				acq_colour_component *= a1;
				colour_component *= a2;
				acq_colour_component += colour_component;
				
				acq_sample_spectrum *= a1;
				sample_spectrum *= a2;
				acq_sample_spectrum += sample_spectrum;				
				
				
				/*
				gs2.ChangeGraph(0, acq_white_component, 0, dw);
				gs2.ChangeGraph(1, acq_colour_component, 0, dw);
				gs2.ChangeGraph(2, acq_sample_spectrum, 0, dw);
				*/
				}
			
			}
		wm = WhereMouseLocal;
		NextQMEvent();
		}
	#endif //0
	}

bool	FindMaximum(RealFunctionF32 &colour_component, size_t &max_pos, size_t &max_width, float &E_res, float &E_other)
	{
	//	colour_component -- положительно определеннаЯ функциЯ.
	//	находим значение ее главного максимума и его энергию,
	//	после чего обнулЯем этот пик.
	//	если пиков не осталось, возвращаем false
	size_t range_length = colour_component.size();
	float	max_val = MaxValue(colour_component, &max_pos);
	if(!max_val) return false;
	size_t n1 = max_pos, n2 = max_pos;
	
	while(colour_component[n1] > colour_component[n1-1] && n1 > 0) n1--;
	while(colour_component[n2] > colour_component[n2+1] && n2 < range_length-1) n2++;
	
	E_res = E_other = 0;
	for(size_t i = 0; i < range_length; i++)
		{
		if(in_range(i, n1, n2)) E_res += colour_component[i];
		else E_other += colour_component[i];
		}
	for(size_t i = n1; i < n2; i++) colour_component[i] = 0;
	
	max_width = n2-n1;
	
	if(0)
		{
		E_res /= (n2-n1);
		E_other /= (range_length -(n2-n1));
		}
	
	return true;
	}


void	WhiteAnalyzer :: Batch()
	{

//	SetFrequencyUnits(M_HERZ);
//	float	dw = sample_rate.MHz()/range_length;
	ComplexFunctionF32	fft_buffer(range_length);
	RealFunctionF32	white_component(range_length), colour_component(range_length);	
	RealFunctionF32	sample_spectrum(range_length);
	RealFunctionF32	maxima_histogram(range_length);
	
	if(YesOrNo("Analyze spectra graphs?", true))
		AnalyzeLocalSpectra();

	GUIProgressBar	progress;
	progress.start("Processing white component", n_rays);
	for(size_t ray = 0; ray < n_rays; ray++)
		{
//		SetCurrentRay(ray);
		for(size_t sub = 0; sub < n_frames; sub++)
			{
//			SetCurrentFrame(sub);
			ComplexFunctionF32	CurrentRay;
			focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
			for(size_t range_no = 0; range_no < n_ranges; range_no++)
				{
				size_t j0 = range(range_no*range_step - range_length/2, 0, n_samples-range_length-1);
				for(size_t j = 0; j < range_length; j++)
					{
					fft_buffer[j] = CurrentRay[j+j0];
					}
				ApplyWindowFunction(fft_buffer, cos2_window());
				FFT(fft_buffer, ftForward);

				CopyData(sample_spectrum, fft_buffer, Functors::assign_f1(cabs_functor<float, complexF32>()));
				
//				float	white = ExtractWhiteComponent(sample_spectrum, white_spectra[range_no], white_component, colour_component);
				colour_component /= white_spectra.row(range_no);
				ApplyFunction(colour_component, fabs_functor<float,float>());
			//	ApplyWindowFunction(colour_component, cos2_win);
			//	range_brightness[ray][range_no] += colour_component.AverageValue()/white;
				

//				float	E_res(0), E_other(0);
				
				size_t	m[4];
				size_t w[4];
				float	e[4];
				
				FindMaximum(colour_component, m[1], w[1], e[1], e[0]);
				FindMaximum(colour_component, m[2], w[2], e[2], e[0]);
				FindMaximum(colour_component, m[3], w[3], e[3], e[0]);
				
				
				if(e[1]/e[2] < 2 && e[2]/e[3] > 2){ // выраженнаЯ двухпиковость
//					range_brightness[ray][range_no] += e1;
					float	exc = fabs(double(m[1])-double(m[2]))/min(m[1], m[2]);
					float	tolerance = 1.5;
					if(exc < tolerance && exc > 1./tolerance)
						range_brightness.at(ray,range_no) += (e[1]+e[2]);
					}
				
				}
			}
		++progress;
		}
	progress.end();
//	maxima_histogram.Display(0, dw, "Maxima histogram");
	LogCompress(range_brightness);
	CutHistogramEdges(range_brightness, range1_F64(0.001, 0.999));
	NormalizeImage(range_brightness, 0,255);
	DisplayMathFunction2D(range_brightness, "range brightness", sco);
	}




//--------------------------------------------------------------------------

float	WhiteAnalyzer :: ExtractWhiteComponent(const RealFunctionF32 &spectrum, const RealFunctionF32 &white, RealFunctionF32 &white_component, RealFunctionF32 &colour_component) const{
//	size_t range_length = spectrum.size();
	RealFunctionF32	f_approx(spectrum);
	RealFunctionF32	normalized_white(white);
	
	#define debug_approximation 0
	
	#if debug_approximation	
		bool	normalize_white = YesOrNo("Normalize white?", true);
		size_t n_peaks_simplify = GetSigned("N peaks to simplify", range_length/8, 1, range_length/2);
	#else
		bool	normalize_white	= true;
		if(OptionPressed())
			normalize_white	= false;
		size_t n_peaks_simplify = range_length/8;
	#endif	

	if(normalize_white)
		{
		f_approx /= white;
		normalized_white /= white;
		}
	white_component.MakeCopy(normalized_white);


	#if debug_approximation	
		GraphSet	gs("Initial state", "sample", "");
		gs.AddGraphUniform(f_approx, 0, 1, "Sample");
		gs.AddGraphUniform(normalized_white, 0, 1, "White");
		gs.AddGraphUniform(f_approx, 0, 1, "Original");
		gs.Display();
	#endif	

	float	white_weight(0);
	
	#if debug_approximation	
		while(CommandPressed())
	#else
		for(size_t n = 0; n < 8; n++)
	#endif
		{
		white_component.CopyData(normalized_white);
		float	v1(0), v0(0);
		for(size_t i = 0; i < range_length; i++)
			{
			v0 +=square(normalized_white[i]);
			v1 += f_approx[i]*normalized_white[i];
			}
		if(v0) white_weight = v1/v0;
		else	white_weight = 0;
		white_component *= white_weight;
				
		for(size_t peak_no = 0; peak_no < n_peaks_simplify; peak_no++)
			{
			float	delta = 0;
			size_t n1 = 0;
			for(size_t i = 0; i < range_length; i++)
				{
				float	v = fabs(f_approx[i] - white_component[i]);
				if(v > delta) n1 = i, delta = v;
				}
			f_approx[n1] = white_component[n1];
			}
		
		#if debug_approximation	
			gs.ChangeGraph(0, f_approx, 0, 1);
			gs.ChangeGraph(1, white_component, 0, 1);
			gs.ChangeTitle("Approximation 1");
			gs.Display();
		#endif
		}
		
	if(normalize_white)
		{
		f_approx *= white;
		white_component *= white;
		}

	colour_component.MakeCopy(spectrum);
	colour_component -= white_component;
	
	colour_component.FilterGauss(range_length/4,extrapolation::by_last_value);
	
	#if debug_approximation	
		gs.ChangeGraph(0, f_approx, 0, 1);
		gs.ChangeGraph(1, white_component, 0, 1);
		gs.ChangeGraph(2, spectrum, 0, 1);
		
		gs.AddGraphUniform(colour_component, 0, 1, "Deviation signed");
		ApplyFunction(colour_component, fabs);
		gs.AddGraphUniform(colour_component, 0, 1, "Deviation abs");
		
		gs.ChangeTitle("Approximation final");
		gs.Display();
	#endif
	
	return white_weight;
	}



XRAD_END
#endif