#include "pre.h"



#include <XRADBasic/Sources/Utils/StatisticUtils.h>
#include <Attic/LABColorImage.h>
#include <XRADBasic/Sources/Utils/LeastSquares.h>

#include "SignalProcessing/GaussSignalTools.h"
#include "RangeSpectrumAnalyzer.h"
#include <Utils/SignalFilters.h>
#include <XRADBasic/Sources/SampleTypes/HLSColorSample.h>
#include "ColourEncode.h"
#include "ColourAnalyzerUtils.h"
//#include <MotionAnalysis.h>
//#include <ImageUtils.h>
//#include <NonUniformMotionAnalysis.h>

XRAD_BEGIN



void	RangeSpectrumAnalyzer :: InitWork()
	{
	//SetAngleUnits(DEGREES);
	//SetDepthUnits(CENTIMETRES);
	Display("Signal before");
	
	n_ranges = depth_range().cm()*10;
	range_step = float(n_samples)/n_ranges; //NB: float, not int
//	range_length = GetSigned("Range length", default_range_length, min_range_size, n_samples);
	
	ExportScanConverterOptions(sco);
	
//	bool	fl = true;//must be true
// 	int	h = 512;//must be 512
// 	range_brightness.SetFlip(fl);
// 	sample_brightness.SetFlip(fl);
// 	parametric_picture.SetFlip(fl);
// 
// 	sample_brightness.InitScanConverter(h);
// 	range_brightness.InitScanConverter(h);
// 	parametric_picture.InitScanConverter(h);
	
	range_brightness.realloc(n_rays, n_ranges);
//	range_brightness.SetImageTitle("Range brightness");

	sample_brightness.realloc(n_rays, n_samples);
//	sample_brightness.SetImageTitle("01_sample_brightness.pct");

	parametric_picture.realloc(n_rays, n_samples);
//	parametric_picture.SetImageTitle("01_parametric_picture.pct");

	
	
	sample_brightness.fill(0);
	for(size_t sub = 0; sub < n_frames; sub ++)
		{
//		SetCurrentFrame(sub);
		for(size_t j = 0; j < n_rays; j++)
			{
			ComplexFunctionF32	CurrentRay;
			focused_data.GetRow(CurrentRay, {sub, j, slice_mask(0)});
//			SetCurrentRay(j);
			for(size_t i = 0; i < n_samples; i++)
				{
				sample_brightness.at(j,i) += cabs(CurrentRay[i]);
				}
			}
		}

	LogCompress(sample_brightness);
	CutHistogramEdges(sample_brightness, range1_F64(0.001, 0.999));
	NormalizeImage(sample_brightness, 0,255);

	if(YesOrNo("Perform stabilization?", false))
		{
		DisplayMathFunction2D(sample_brightness, "Accumulated B-image before stabilization", sco);
		StabilizeSignalComponents(*this);
		}
	}






void	RangeSpectrumAnalyzer :: CalculateWhite(size_t range_length, RealFunction2D_F32 &white_spectra)
	{
	ComplexFunctionF32	fft_buffer(range_length);
	RealFunctionF32	window_func(range_length);
	white_spectra.realloc(n_ranges, range_length);
	
//	CreateWindowFunction(window_func, blackman_nuttall_win);
	CreateWindowFunction(window_func, hann_window());

//	CreateWindowFunction(window_func, blackman_nuttall_win);
		// используем наиболее "чистое" с точки зрениЯ 
		// боковых лепестков окно
	GUIProgressBar	progress;
	progress.start("White component analysis", n_ranges);
	for(size_t range_no = 0; range_no < n_ranges; range_no++)
		{
		int	j0 = range(float(range_no)*range_step - range_length/2, 0, n_samples-range_length-1);
		for(size_t sub = 0; sub < n_frames; sub++)
			{
//			SetCurrentFrame(sub);
			for(size_t ray = 0; ray < n_rays; ray++)
				{
				ComplexFunctionF32	CurrentRay;
				focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
//				SetCurrentRay(ray);
				ComplexFunctionF32::iterator	ft = fft_buffer.begin();
				ComplexFunctionF32::iterator	pb = CurrentRay.begin() + j0;
				RealFunctionF32::iterator	ws = white_spectra.row(range_no).begin();
				
				for(size_t j = 0; j < range_length; ++j, ++ft, ++pb)
					{
					*ft = *pb;
					}
				//ApplyWindowFunction(fft_buffer, hamming_win);
				fft_buffer *= window_func;
				FFT(fft_buffer, ftForward);
				
				ft = fft_buffer.begin();
				for(size_t j = 0; j < range_length; ++j, ++ft, ++ws)
					{
					//white_spectra.row(range_no)[j] += cabs(fft_buffer[j]);
					*ws += cabs(*ft);
					}
				}
			}
		white_spectra.row(range_no) /= MaxValue(white_spectra.row(range_no));
		++progress;
		}
	
//	EndProgress();
	for(size_t i = 0; i < range_length; i++) white_spectra.col(i).FilterGauss(4);
	for(size_t range_no = 0; range_no < n_ranges; range_no++) white_spectra.row(range_no).FilterGauss(2);
	}


void	RangeSpectrumAnalyzer :: NormalizeBrightness()
	{
	for(size_t sub = 0; sub < n_frames; sub++)
		{
//		SetCurrentFrame(sub);
		for(size_t ray = 0; ray < n_rays; ray++)
			{
			ComplexFunctionF32	CurrentRay;
			focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
//			SetCurrentRay(ray);
			for(size_t s = 0; s < n_samples; ++s)
				{
				double	brightness = cabs(CurrentRay[s]);
				if(brightness > 0.01)
					CurrentRay[s] /= brightness;
				}
			}
		}
	Display("Brightness normalized");

	for(size_t sub = 0; sub < n_frames; sub++)
		{
//		SetCurrentFrame(sub);
		for(size_t ray = 0; ray < n_rays; ray++)
			{
			ComplexFunctionF32	CurrentRay;
			focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
//			SetCurrentRay(ray);
			for(size_t s = 0; s < n_samples-1; ++s)
				{
				CurrentRay[s] %= CurrentRay[s+1];
				}
			CurrentRay[n_samples-1] = 1;
			
//			CurrentRay -= AverageValue(CurrentRay);
			CurrentRay.FilterGauss(3);

			}
		}

	Display("Phase derivative");

	if (0) if(YesOrNo("Subtract frames?", true))
		{
		ComplexFunction2D_F32	CurrentFrame;
		focused_data.GetSlice(CurrentFrame, {0, slice_mask(0), slice_mask(1)});
//		SetCurrentFrame(0);
		ComplexFunction2D_F32 frame_0(CurrentFrame);
		
		for(size_t sub = 0; sub < n_frames-1; sub++)
			{
//			SetCurrentFrame(sub+1);
			focused_data.GetSlice(CurrentFrame, {sub+1, slice_mask(0), slice_mask(1)});
			ComplexFunction2D_F32 f1(CurrentFrame);
//			SetCurrentFrame(sub);
			focused_data.GetSlice(CurrentFrame, {sub, slice_mask(0), slice_mask(1)});
			CurrentFrame -= f1;
			}
//		SetCurrentFrame(n_frames-1);
		focused_data.GetSlice(CurrentFrame, {n_frames-1, slice_mask(0), slice_mask(1)});
		CurrentFrame -= frame_0;
		
		Display("Differences");
		}
	}

void	RangeSpectrumAnalyzer :: NormalizeSpectrum(size_t range_length, bool dynamic_normalization)
	{
	RealFunction2D_F32 white_spectra;
	
	if(dynamic_normalization)
		{
		// динамическаЯ нормализациЯ. если применЯть ее при наборе эталонов,
		// получаетсЯ худшаЯ чувствительность при последующей фильтрации. однако нормализацию
		// фильтруемых данных лучше делать этим алгоритмом, так как иначе картина получаетсЯ
		// неоднороднаЯ по глубине
		
		CalculateWhite(range_length, white_spectra);
		ComplexFunctionF32	fft_buffer(range_length);
		RealFunctionF32	window_func(range_length);
		size_t	rl2 = range_length/2;
		
		CreateWindowFunction(window_func, hann_window());
		// CreateWindowFunction(window_func, bartlett_win);
		// следует использовать только такие окна, которые при сдвиге на
		// половину длины в сумме дают константу: косинусное ханна или треугольное бартлетта.
		// причем треугольное Явно хуже (и умозрительно, и на деле)

		
		GUIProgressBar	progress;
		progress.start("Spectrum normalization", n_frames*n_rays);
		for(size_t sub = 0; sub < n_frames; sub++)
			{
//			SetCurrentFrame(sub);
			for(size_t ray = 0; ray < n_rays; ray++)
				{
				ComplexFunctionF32	CurrentRay;
				focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
//				SetCurrentRay(ray);
				ComplexFunctionF32	ray_buffer(n_samples);
				for(size_t s0 = 0; s0 < n_samples; s0 += rl2)
					{
					for(size_t s = 0; s < range_length; s++)
						{
						if(s0+s < n_samples) fft_buffer[s] = CurrentRay[s0+s];
						else fft_buffer[s] = 0;
						}
					fft_buffer *= window_func;
					FFT(fft_buffer, ftForward);
					size_t	current_range = range(float(s0 + rl2)/range_step, 0, n_ranges-1);
					fft_buffer /= white_spectra.row(current_range);
						
					FFT(fft_buffer, ftReverse);
					for(size_t s = 0; s < range_length; s++)
						{
						if(s0+s < n_samples) ray_buffer[s0+s] += fft_buffer[s];
						}
					}
				CurrentRay.CopyData(ray_buffer);
				++progress;
				}
			}
//		EndProgress();
		}

	else
		{
		//	статическаЯ нормализациЯ (без учета частотно-динамического ослаблениЯ)
		size_t	fft_length = ceil_fft_length(n_samples);
		ComplexFunctionF32	fft_buffer(fft_length);
		ComplexFunctionF32	divisor(fft_length);
		
		GUIProgressBar	progress;
		progress.start("Spectrum normalization", n_frames*n_rays*2);
		
		for(size_t sub = 0; sub < n_frames; sub++)
			{
//			SetCurrentFrame(sub);
			for(size_t ray = 0; ray < n_rays; ray++)
				{
				ComplexFunctionF32	CurrentRay;
				focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
//				SetCurrentRay(ray);
				//fft_buffer.fill(0);
				fft_buffer.CopyData(CurrentRay);
				FFT(fft_buffer, ftForward);
				for(size_t sample = 0; sample < fft_length; sample++)
					{
					divisor[sample] += cabs(fft_buffer[sample]);
					}
				++progress;
				}
			}
		divisor.FilterGauss(n_samples/16);
		divisor /= MaxValue(divisor);
		for(size_t sub = 0; sub < n_frames; sub++)
			{
//			SetCurrentFrame(sub);
			for(size_t ray = 0; ray < n_rays; ray++)
				{
				ComplexFunctionF32	CurrentRay;
				focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
//				SetCurrentRay(ray);
				fft_buffer.CopyData(CurrentRay);
				FFT(fft_buffer, ftForward);
				fft_buffer /= divisor;
				FFT(fft_buffer, ftReverse);
				CurrentRay.CopyData(fft_buffer);
				++progress;
				}
			}
		progress.end();
//		EndProgress();
	//	divisor.Display(0,1,"Mean spectrum");
		}
	if(YesOrNo("Display normalized white spectra?", false))
		{
		CalculateWhite(range_length, white_spectra);
		// длЯ отображениЯ спектра используем больший размер
		// окна спектроанализатора, чем длЯ фильтрации
		DisplayMathFunction2D(white_spectra, "Normalized white spectra");
		}
	}




XRAD_END
