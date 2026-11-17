#include "pre.h"





#include <ColorImage.h>
#include <XRADBasic/Sources/Utils/StatisticUtils.h>
#include <Attic/LABColorImage.h>
#include <XRADBasic/Sources/Utils/LeastSquares.h>

#include "SignalProcessing/GaussSignalTools.h"
#include "SubbandAnalyzer.h"
#include <Utils/SignalFilters.h>
#include <XRADBasic/Sources/SampleTypes/HLSColorSample.h>
#include "ColourEncode.h"
#include "ColourAnalyzerUtils.h"
#include <MotionAnalysis.h>
#include <ImageUtils.h>
#include <NonUniformMotionAnalysis.h>

XRAD_BEGIN








void	SubbandAnalyzer :: InitWork()
	{
	//SetAngleUnits(DEGREES);
	//SetDepthUnits(CENTIMETRES);
//	Display("Signal before");
	
	fft_length = CeilFFTLength(n_samples);

	}


void	SubbandAnalyzer::InitFilters()
	{
	n_bands = 45;
//	n_bands = 225;
//	n_bands = 128;
	n_subbands = 1;
	bool	direct_frequency_scale = false;
	
	filters.realloc(n_bands, n_subbands);
	for(int i = 0; i < n_bands; ++i)
		{
		for(int j = 0; j < n_subbands; ++j)
			{
			filters.at(i,j).realloc(fft_length);
			}
		}

	filters_carriers.realloc(n_bands);// in samples, rel. to fft_length
	filters_carriers.fill(0);	
	
	double	full_band = fft_length;
	filters_bandwidth = full_band * max(1./108., (1./(n_bands*n_subbands*3)));

	double	min_scale = double(fft_length)/double(fft_length - 3*filters_bandwidth);
	
	double	max_scale = 25;
	double	d_scale = (max_scale - min_scale)/n_bands;
	
	double	f0 = 0*fft_length/4;
	double	d_band = full_band/n_bands;
	double	d_subband = d_band/n_subbands;
	
	
	for(int i = 0; i < n_bands; ++i)
		{
		for(int j = 0; j < n_subbands; ++j)
			{
			double	center_f;
			if(direct_frequency_scale)
				{
				center_f = f0 + i*d_band + j*d_subband + d_subband/2;
				}
			else
				{			
				double	current_scale = min_scale + i*d_scale;
				center_f = fft_length/current_scale;				
				}
			
			filters_carriers[i] += center_f;
			for(int k = 0; k < fft_length; ++k)
				{
				filters.at(i,j)[k] = gauss(k-center_f, filters_bandwidth);
				}
			}
		filters_carriers[i] /= n_subbands;
		if(n_bands<=9)
			{
			filter_gs.AddGraph(filters[i][n_subbands/2], 0,1, ssprintf("f%d", i));
			}
		else
			{
			int	step = n_bands/9;
			if(step==1) ++step;
			if(!(i%step)) filter_gs.AddGraph(filters[i][n_subbands/2], 0,1, ssprintf("f%d", i));
			}
		}
	
	//SetDepthUnits(CENTIMETRES);
//	SetFrequencyUnits(HERZ);
	double	sample_size = (0.5*sound_speed.mm_sec()/sample_rate.Hz());//sample size in millimetres
	
	graph_d_scale = sample_size*d_scale;
	graph_scale_0 = sample_size*min_scale + graph_d_scale;
	}

//200--1086


void	SubbandAnalyzer::PrepareRayForProcessing()
	{
	int	s =  128;
	int	s2 = s/2;
	int	patch_start = n_samples-2*s;
	
	ComplexFunctionF32 patch(s);
	RealFunctionF32	window(s);
//	CreateWindowFunction(window, cos2_win);
	CreateWindowFunction(window, bartlett_win());
	
//	for(int i = 0; i < n_samples; ++i) CurrentRay[i] = i;

	for(int i = 0; i < s; ++i)
		{
		CurrentRay[patch_start + s + i] = patch[i] = CurrentRay[patch_start+i];
		}
	for(int i = 0; i < s; ++i)
		{
		int	i0 = s2+i;
		int	i1 = i0 % s;
		CurrentRay[patch_start + i0] *= window[i1];
		CurrentRay[patch_start + i0] += patch[i]*window[i];
		}
//#error continue from here
	RealFunctionF32	divisor(n_samples);
	CopyData(divisor, CurrentRay, cabs2_functor());
	divisor.FilterGauss(15);
	CurrentRay /= divisor;
	if(CapsLock())CurrentRay.Display(0,1,"CR");
	}


struct plus_assign_cabs2 : public binary_function<float,complexF32,float> //functor for operator+=
	{
	float& operator()(float &x, const complexF32 &y) const {return (x+=cabs2(y));}
	};


void	SubbandAnalyzer :: Batch()
	{
	InitFilters();
	
	int	n_subapertures_used = n_frames;

	
//	int	s = n_samples;
	
//	index_vector	sb_sizes = quick_iv(n_bands, n_rays, n_samples);
	index_vector	sb_sizes = quick_iv(n_samples, n_rays, n_bands);
	MathFunctionMD<RealFunction2D_F32> sb(sb_sizes);
	DataArray<RealFunction2D_F32>	sb_images(n_bands);
//	DataArray2D<DataArray<RealFunctionF32> > sb_rows(n_rays, n_samples);
	
	for(int i = 0; i < n_bands; ++i)
		{
		index_vector	slice_indices = quick_iv(slice_mask(1), slice_mask(0), i);
		sb.GetSlice(sb_images[i], slice_indices);
		sb_images[i].fill(0);
		}
	
/*
	for(int i = 0; i < n_rays; ++i)
		{
		for(int j = 0; j < n_samples; ++j)
			{
			index_vector	row_indices = quick_iv(j, i, slice_mask(0));
			sb.GetRow(sb_rows.at(i,j), row_indices);
			}
		}
	
*/		
	
	RealFunctionF32	sample_spectrum(fft_length);
	sample_spectrum.fill(0);
	
	
	StartProgress("Preparing data", n_rays);
	for(int ray = 0; ray < n_rays; ++ray)
		{
		SetCurrentRay(ray);
		for(int sub = 0; sub < n_subapertures_used; ++sub)
			{
			SetCurrentFrame(sub);
			PrepareRayForProcessing();
			}
		NextProgress();
		}
	EndProgress();

	StartProgress("Filtering", n_bands*n_subbands*n_frames);
	for(int band = 0; band < n_bands; ++band)
		{
		for(int sb = 0; sb < n_subbands; ++sb)
			{
			for(int sub = 0; sub < n_subapertures_used; ++sub)
				{
				SetCurrentFrame(sub);
				
				ComplexFunctionF32	fft_buffer(fft_length);
				
				for(int ray = 0; ray < n_rays; ++ray)
					{
					SetCurrentRay(ray);					
									
					fft_buffer.CopyData(CurrentRay);
					fft_buffer.FFT(ftForward);
					
					AA_1D_OpEq(sample_spectrum, fft_buffer, plus_assign_cabs2());
								
					fft_buffer *= filters[band][sb];
					fft_buffer.FFT(ftReverse);
					
					DifferentAA_1D_OpEq(sb_images[band].row(ray), fft_buffer, plus_assign_cabs2());
					}
				NextProgress();
				}
			}
		}
	EndProgress();

	
	ApplyFunction(sample_spectrum, sqrt_functor());
	sample_spectrum.FilterGauss(2*filters_bandwidth);
	
	sample_spectrum /= MaxValue(sample_spectrum);

	filter_gs.AddGraph(sample_spectrum, 0,1,"ss");
		
	StartProgress("Preparing plates", 2*n_bands);

	float	global_min;
	float	global_max;

	for(int i = 0; i < n_bands; ++i)
		{
		double	fw = (1./16.)*double(fft_length)/filters_carriers[i];
//		double	fw = (1./16.)*double(fft_length)/filters_carriers[n_bands/2];
		double	fl = 0;//almost nothing
		sb_images[i].FilterGaussSeparate(fw,fl);
		
		double	divisor = sample_spectrum[int(filters_carriers[i])];
		sb_images[i] /= divisor;
		CutHistogramEdges(sb_images[i], 0.1, 0.01);
		/*
		if(!i)
			{
			global_min = MinValue(sb_images[i]);
			global_max = MaxValue(sb_images[i]);
			}
		else
			{
			global_min = min(global_min, MinValue(sb_images[i]));
			global_max = max(global_max, MaxValue(sb_images[i]));
			}
		*/
		NextProgress();
		}
	
	global_max = MaxValue(sb);
	global_min = MinValue(sb);
	
	for(int i = 0; i < n_bands; ++i)
		{
		sb_images[i].at(0,0) = global_min;
		sb_images[i][0][1] = global_max;
		LogCompress(sb_images[i]);
		NormalizeImage(sb_images[i], 0, 255);
		NextProgress();
		}
	EndProgress();


	AnimateResult(sb_images);
	}

void	SubbandAnalyzer :: AnimateResult(DataArray<RealFunction2D_F32> &sb_images)
	{
	static bool	draw_grid = false;
	static bool	flip_image = true;
	string	title = "Spectrum animation";
	
	
	DataArray<GrayRaster>	frames(sb_images.size());
	DataArray<GrayPixel*>	pointers(sb_images.size());

	AcousticFrameDisplayer frame;
	GrayPixelScanConverter	SC(sb_images[0].vsize(), sb_images[0].hsize());

//	Get_Checkbox_Decision("Scan converter options", 2, 
//		"Draw grid", &draw_grid,
//		"Flip image", &flip_image);

	// последующие вызовы только длЯ нахождениЯ правильных размеров
	// детектированного кадра, вычисленные данные оказываютсЯ не нужны.
	// не совсем хорошо, но лучше не придумал.


	ExportScanConverterOptions(SC);
	SC.SetBackground(80);
	SC.SetGrid(draw_grid, 160, cm(2));
	SC.SetFlip(flip_image);

	SC.SetImageTitle(title);
	SC.InitScanConverter(384);

	StartProgress("Creating animation", sb_images.size());
	for(int i = 0; i < sb_images.size(); ++i)
		{				
		CopyData(SC, sb_images[i]);
		SC.BuildRaster();
		frames[i].MakeCopy(SC.GetRaster());
		pointers[i] = &frames[i].at(0,0);
		NextProgress();
		}
	EndProgress();
	
	QuickDisplayRasterAnimation(title.c_str(), sb_images.size(), frames[0].vsize(), frames[0].hsize(), (const uint8_t**)&pointers[0], graph_scale_0, graph_d_scale , "object scale");
	}

// PixMapImage.cc -- ROI rect in animations

XRAD_END
