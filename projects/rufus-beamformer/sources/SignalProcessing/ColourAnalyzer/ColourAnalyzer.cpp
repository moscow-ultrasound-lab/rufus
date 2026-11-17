#include "pre.h"
#include <XRADBasic/FFT1D.h>

//#include <PCM_Wave.h>

//#include <XRADBasic/Sources/Utils/StatisticUtils.h>
#include <Attic/LABColorImage.h>
#include <XRADBasic/Sources/Utils/LeastSquares.h>

#include "SignalProcessing/GaussSignalTools.h"
#include "ColourAnalyzer.h"
#include <Utils/SignalFilters.h>
//#include <XRADBasic/Sources/SampleTypes/HLSColorSample.h>
#include "ColourEncode.h"
#include "ColourAnalyzerUtils.h"
#include <XRADBasic/Sources/Utils/Crayons.h>
//#include <ImageUtils.h>

//Код сильно устарел, но некоторые заложенные в нем идеи могут быть интересны в будущем. Поэтому не уничтожаю, но и не трачу сил на полную адаптацию

XRAD_BEGIN

#if 1
//using LABColorImage = DataArray2D<DataArray<LABColorSample>>;

bool no_colorization = false;


//void	ColourAnalyzer :: ProcessInitDialog(){}

ColourAnalyzer :: ColourAnalyzer():SignalProcessor()
	{
	strcpy(SIMIO::Process_Name,"ColourAnalyzer");
	color_encode = &EncodeHLS;
//	color_encode = &EncodeLAB;
//	color_encode = &EncodeXYZ;

	//SetDepthUnits(CENTIMETRES);
	default_frequency = 3.73;
	default_bandwidth = 0.87;

	}

ColourAnalyzer :: ~ColourAnalyzer()
	{
	}


void	ColourAnalyzer :: InitWork() {
	//SetAngleUnits(DEGREES);
	//SetDepthUnits(CENTIMETRES);
	Display("Signal before");
	
	
	n_ranges = depth_range().cm()*10;
	n_spatial_layers = 1;
	//n_ranges = GetSigned("Number of ranges", (r_max-r_min)*10, 2, n_samples);
	n_spatial_layers = GetSigned("Layers number", 1, 1, 15);

	process_buffer.realloc(n_frames, n_spatial_layers, n_rays, n_samples);

	ExportScanConverterOptions(moment_frequency);
	ExportScanConverterOptions(moment_bandwidth);
	ExportScanConverterOptions(range_brightness);
	ExportScanConverterOptions(sample_brightness);
	ExportScanConverterOptions(maxima_count);

	moment_frequency.realloc(n_rays, n_ranges);
//	moment_frequency.SetImageTitle("02_moment_frequency.pct");
	moment_bandwidth.realloc(n_rays, n_ranges);
//	moment_bandwidth.SetImageTitle("03_moment_bandwidth_pct");
	range_brightness.realloc(n_rays, n_ranges);
//	range_brightness.SetImageTitle("Range brightness");
	sample_brightness.realloc(n_rays, n_samples);
//	sample_brightness.SetImageTitle("01_sample_brightness.pct");
	maxima_count.realloc(n_rays, n_ranges);
//	maxima_count.SetImageTitle("04_maxima_count.pct");
	

	PrepareData();
	}

void	ColourAnalyzer :: EndWork()
	{
	}



extern	bool	ATL_data;
//	анализ изменениЯ спектральных свойств сигнала

void	ColourAnalyzer :: PrepareData()
	{
	size_t nsl_2 = n_spatial_layers/2;

	GUIProgressBar	progress;
	progress.start("Spatial layers filtering", n_frames*n_spatial_layers);

	for(size_t sub = 0; sub < n_frames; sub++)
		{
//		SetCurrentFrame(sub);
		for(size_t layer = 0; layer < n_spatial_layers; layer++)
			{
			double	s2 = nsl_2 ? 2*double(layer - nsl_2)/nsl_2 : 0;
			double	broad = 3./n_spatial_layers;


			for(size_t ray = 0; ray < n_rays; ray++)
				{
//				SetCurrentRay(ray);
				ComplexFunctionF32	CurrentRay;
				focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
				process_buffer.at(sub,layer).row(ray).CopyData(CurrentRay);
				//process_buffer.at(sub,layer)[ray].FilterGaussCarrier(1, 0.5);// DC offset correction
				}
			if(n_spatial_layers > 1)  NormalizeSpectrum(process_buffer.at(sub,layer), spatial, s2, broad);

			double	maxval = cabs(MaxValue(process_buffer.at(sub,layer)));
			process_buffer.at(sub,layer) /= maxval;
			
			++progress;
			}
		}
	if(n_spatial_layers > 1)
		{
		process_buffer.AnalyzeLayers();
		process_buffer.DisplayLayers();
		}
	}





void	ColourAnalyzer :: MeasureSpectrum()
	{
//	in_range()
	const	size_t min_ft_size = ceil_fft_length(max(n_samples/n_ranges, size_t(16)));
	const	double	ft_step = double(n_samples)/n_ranges; //NB: double, not int
	const	size_t	acquisition_count = n_spatial_layers*n_frames;
	double	colour_amplification = 2;

	ft_size = ceil_fft_length(GetSigned("FFT length", min_ft_size*2, min_ft_size, n_samples));

	RealFunction2D_F32	average_spectra_for_ranges(n_ranges, ft_size);
	RealFunctionF32	carriers_for_ranges(n_ranges);

	RealFunctionF32 ft_accumulator(ft_size), ft_accumulator_filtered(ft_size);
	ComplexFunction2D_F32	ft_buffer(acquisition_count, ft_size);
		

	GUIProgressBar	progress;
	progress.start("Analyzing spectrum", n_ranges);

	maxima_count.fill(0);
	for(size_t i = 0; i < n_ranges; i++)
		{
		for(size_t j = 0; j < n_rays; j++)
			{
			for(size_t layer = 0; layer < n_spatial_layers; layer++)
				{
				for(size_t sub = 0; sub < n_frames; sub ++)
					{
					size_t r = j;
					//size_t s0 = range(double(i)*ft_step - ft_size/2, 0, n_samples-ft_size);
					size_t s0 = double(i)*ft_step - ft_size/2;
					size_t acq = n_frames*layer + sub;

					for(size_t k = 0; k < ft_size; k++)
						{
						size_t l = range(s0+k, 0, n_samples-1);
						ft_buffer.at(acq,k) = process_buffer.at(sub,layer).at(r,l);						
						}					
					}
				}
			
			maxima_count.at(j,i) = 0;
//			ComputeLocalSpectrum(ft_buffer);
			
			ComputeLocalSpectrum(ft_buffer);
			ComputeAverageSpectrum(ft_buffer, ft_accumulator, ft_accumulator_filtered);

//			maxima_count.at(j,i) = CountMaxima(ft_buffer);
			
//			DisplayIntermediateResults(ft_accumulator,ft_accumulator_filtered, 32);
			
			if(1)
			{
				RealVectorF32 moments(2);
				WeightMoments(ft_accumulator_filtered, moments, true);
				moment_frequency.at(j,i) = moments[0];
				moment_bandwidth.at(j,i) = sqrt(moments[1]);
			}
			else
			{
				FindMaximaPosition(ft_buffer, moment_frequency.at(j,i), moment_bandwidth.at(j,i));
			}
			
			range_brightness.at(j,i) = AverageValue(ft_accumulator_filtered);
			//average_spectra_for_ranges[i].CopyData(*real(ft_accumulator_filtered));
			
		//	maxima_count.at(j,i) /= range_brightness.at(j,i);
			
			ft_accumulator_filtered /= MaxValue(ft_accumulator_filtered);
			average_spectra_for_ranges.row(i) += ft_accumulator_filtered;
			}
		RealVectorF32 moments(1);
		WeightMoments(average_spectra_for_ranges.row(i), moments, false);
		carriers_for_ranges[i] = moments[0];
		average_spectra_for_ranges.row(i), &carriers_for_ranges[i];
		++progress;
		}


	for(size_t i = 0; i < n_samples; i++)
		{
		for(size_t j = 0; j < n_rays; j++)
			{
			for(size_t layer = 0; layer < n_spatial_layers; layer++)
				{
				for(size_t sub = 0; sub < n_frames; sub ++)
					{
					sample_brightness.at(j,i) += cabs(process_buffer.at(sub,layer).at(j,i));
					}
				}
			}
		}


//	EndProgress();

	//CorrectAttenuation();
	PrepareResultingImages();
	BuildResonancePicture();

	BuildColorParametricPicture(colour_amplification);	
	FitDataToRanges(colour_amplification);
	
	NormalizeImage(moment_frequency, 0, 255);
	moment_frequency.InitScanConverter();
	moment_frequency.BuildConvertedImage();
	DisplayMathFunction2D(moment_frequency.GetConvertedImage(), "moment frequency");
	NormalizeImage(moment_bandwidth, 0, 255);
	moment_bandwidth.InitScanConverter();
	moment_bandwidth.BuildConvertedImage();
	DisplayMathFunction2D(moment_bandwidth.GetConvertedImage(), "moment bandwidth");
	

//	SetFrequencyUnits(M_HERZ);

	carriers_for_ranges *= sample_rate.MHz()/ft_size;

	if(1)
		{
		phaseCorrection.realloc(n_samples);
		double	w0 = 0, w1 = 0;
		size_t nr2 = n_ranges/2;

		for(size_t i = 0; i < nr2; i++) w0 += carriers_for_ranges[i], w1 += carriers_for_ranges[i+nr2];
		w0 /= nr2;
		w1 /= nr2;

		for(size_t i = 0; i < n_samples; i++)
			{
			double	v = (w1-w0)*double(i)/n_samples;

			v *= (two_pi()*i/sample_rate.MHz());

			phaseCorrection[i] = polar(1,v);
			}
		//phaseCorrection.Display(0,1, "Phase correction");
		}

	DisplayMathFunction(carriers_for_ranges, 0, 1, "Carrier frequencies");
	RealFunction2D_F32 spectr2(average_spectra_for_ranges);
//	spectr2.ScaleAspect(average_spectra_for_ranges, 4);
	DisplayMathFunction2D(spectr2,"Spectra grayscale");
	LogCompressRangeDB(spectr2, 60, 0);
	DisplayMathFunction2D(spectr2, "Spectra grayscale log.");
	}


void	ColourAnalyzer :: PrepareResultingImages()
	{
//	SetFrequencyUnits(M_HERZ);
	double	f_factor = sample_rate.MHz()/ft_size;

	moment_frequency *= f_factor;
	moment_bandwidth *= f_factor;
	LogCompress(sample_brightness);
	
//	moment_frequency.SmoothRays(0.05);
//	moment_bandwidth.SmoothRays(0.05);
//	sample_brightness.SmoothRays(0.05);

	CutHistogramEdges(sample_brightness, range1_F64(0.001, 0.999));
	NormalizeImage(sample_brightness, 0,255);
	sample_brightness.InitScanConverter();
	sample_brightness.BuildConvertedImage();
	DisplayMathFunction2D(sample_brightness.GetConvertedImage(), "sample brightness");

//	maxima_count.logCompress();
//	maxima_count.SmoothRays(0.5);
	CutHistogramEdges(maxima_count, range1_F64(0.001, 0.999));
	NormalizeImage(maxima_count, 0,255);
	maxima_count.InitScanConverter();
	maxima_count.BuildConvertedImage();
	DisplayMathFunction2D(maxima_count.GetConvertedImage(), "maxima count");

	LogCompress(range_brightness);
//	range_brightness.SmoothRays(0.05);
	CutHistogramEdges(range_brightness, range1_F64(0.001, 0.999));
	NormalizeImage(range_brightness, 0,255);

	range_brightness.InitScanConverter();
	range_brightness.BuildConvertedImage();
	DisplayMathFunction2D(range_brightness.GetConvertedImage(), "range brightness");
	}


void	ColourAnalyzer :: CorrectAttenuation()
	{
	DynamicFrequencyDomainFilter	dynamic_filter;
	//SetDepthUnits(CENTIMETRES);
	size_t n_ranges_dynamical_filter = int(depth_range().cm()); // по одному диапазону фильтра на см
	dynamic_filter.InitDynamicFilter(n_ranges_dynamical_filter, 0.5);
	
	const physical_length	convex_radius = cm(GetFloating("Convex radius", min(6., r_min().cm()), 0, r_max().cm()));
		// криво, сознаю, но что делать
		// была решетка радиусом 4 см (измерениЯ на соколе летом),
		// сейчас 6 см. в dry.dat файле информациЯ есть, сюда она не попадает
	bool	dynamic_filter_loaded = dynamic_filter.LoadUniversalFilter("universal_filter.note", r_min() - convex_radius, depth_range());
	if(!dynamic_filter_loaded) dynamic_filter_loaded = dynamic_filter.LoadFilter(r_min() - convex_radius, depth_range());
	if(dynamic_filter_loaded)
		{
		RealFunctionF32	f0(dynamic_filter.n_ranges), bandwidth(dynamic_filter.n_ranges);
		for(size_t i = 0; i < dynamic_filter.n_ranges; ++i)
			{
			f0[i] = dynamic_filter.filters[0].filter_f0.MHz();
			bandwidth[i] = dynamic_filter.filters[0].filter_bandwidth.MHz();
			}
		
		for(size_t range = 0; range < n_ranges; range++)
			{
			size_t r0 = (range*(dynamic_filter.n_ranges-1))/n_ranges;
			double	dr = double(range*(dynamic_filter.n_ranges-1))/n_ranges - r0;

			double	f_correction = (f0[r0]*(1.-dr) + f0[r0+1]*dr)/f0[0];
			double	bw_correction = (bandwidth[r0]*(1.-dr) + bandwidth[r0+1]*dr)/bandwidth[0];
			moment_frequency.col(range) /= f_correction;
			moment_bandwidth.col(range) /= sqrt(bw_correction);
			// sqrt -- эвристика
			}
		}
	}

void	ColourAnalyzer :: FitDataToRanges(double colour_amplification)
	{
	double	min_visible_f = default_frequency - 1./colour_amplification;
	double	max_visible_f = default_frequency + 1./colour_amplification;
	double	min_visible_bw = default_bandwidth - 1./(1.5*colour_amplification);
	double	max_visible_bw = default_bandwidth + 1./(1.5*colour_amplification);

	CutHistogramEdges(moment_frequency, range1_F64(0.001, 0.999));
	CutHistogramEdges(moment_bandwidth, range1_F64(0.001, 0.999));
	RangeData(moment_frequency, min_visible_f, max_visible_f);
	RangeData(moment_bandwidth, min_visible_bw, max_visible_bw);
	


	FILE	*param_analyze = fopen("parametric_analyze.note", "w");
	if(!param_analyze)
		{
		Error("File 'parametric_analyze.note' could not be opened");
		if(YesOrNo("Try again?", true))
			param_analyze = fopen("parametric_analyze.note", "w");
		}
	if(param_analyze)
		{
		// double	dw_factor = 2*sqrt(2*log2); // чтобы считать ширину по половинному уровню
		
		fprintf(param_analyze, "Parameter\tFrequency\tBandwidth\n");

		fprintf(param_analyze, "Default value\t%.2f\t%.2f\n",
			default_frequency,
			default_bandwidth);

		fprintf(param_analyze, "Av. value\t%.2f\t%.2f\n",
			AverageValue(moment_frequency),
			AverageValue(moment_bandwidth));
/*
		fprintf(param_analyze, "Std. deviation\t%.2f\t%.2f\n",
			moment_frequency.StandardDeviation(),
			moment_bandwidth.StandardDeviation());
*/
		fprintf(param_analyze, "Min. value\t%.2f\t%.2f\n",
			MinValue(moment_frequency),
			MinValue(moment_bandwidth));

		fprintf(param_analyze, "Min.visible\t%.2f\t%.2f\n",
			min_visible_f,
			min_visible_bw);

		fprintf(param_analyze, "Max. value\t%.2f\t%.2f\n",
			MaxValue(moment_frequency),
			MaxValue(moment_bandwidth));

		fprintf(param_analyze, "Max.visible\t%.2f\t%.2f\n",
			max_visible_f,
			max_visible_bw);
		fclose(param_analyze);
		}
	}


size_t color_encode_function_no = 0;

ColorEncodeFunction*	encoding_functions[5] = {
	&EncodeHLS,
	&EncodeLAB,
	&EncodeF0,
	&EncodeDW,
	&EncodeXYZ
	};

void	ColourAnalyzer :: BuildColorParametricPicture(double &ab_amplification)
	{
	do
		{
		RealFunction2D_F32	L_norm, a_norm, b_norm;
		size_t n_samples_L = sample_brightness.hsize();
		if(YesOrNo("Use auto f0 and dw", false))
			{
			ComputeHistogramMaxValues(moment_frequency, moment_bandwidth, range_brightness, default_frequency, default_bandwidth);
			}
		else
			{
			default_frequency = 3.73;
			default_bandwidth = 0.87;
			}
		
		
		L_norm.MakeCopy(sample_brightness);
		a_norm.MakeCopy(moment_frequency);
		b_norm.MakeCopy(moment_bandwidth);
		
		NormalizeImage(L_norm, 0,255);
		
		a_norm -= default_frequency;
		b_norm -= default_bandwidth;
		
		color_encode_function_no = GetButtonDecision("Color encoding function", //5,
		{
			"HLS model",
			"LAB model",
			"F0 encode",
			"DW encode",
			"XYZ encode"
		});
		
		ab_amplification = GetFloating("Colourization factor", ab_amplification, 0, HUGE_VAL);
		color_encode = encoding_functions[color_encode_function_no];
		
		a_norm *= ab_amplification;
		b_norm *= 1.5*ab_amplification;
		
		
		
		BuildFDWHistogram(a_norm, b_norm, color_encode);
		
		ColorScanConverter	SC_f0(n_rays, n_samples_L);// SC_dw(NULL, n_rays, n_samples_L);
		ExportScanConverterOptions(SC_f0);
//		ExportScanConverterOptions(SC_dw);

		
		for(size_t i = 0; i < n_rays; i++)
			{
			for(size_t j = 0; j < n_samples_L; j++)
				{
				double	x = double(j*moment_frequency.hsize())/n_samples_L;
				double	B = L_norm.at(i,j);
				double	f = range(b_norm.row(i).in(x), -1, 1);
				double	w = range(a_norm.row(i).in(x), -1, 1);
			
				SC_f0.at(i,j) = color_encode(B, f, w);

				}
			}
//		SC_f0.SetImageTitle("04_color_parametrization.pct");
		NormalizeImage(SC_f0, 0,255);
		SC_f0.InitScanConverter();
		SC_f0.BuildConvertedImage();
		DisplayMathFunction2D(SC_f0.GetConvertedImage(),"color parametrization");
		}while(YesOrNo("Change colourization setting", false));
	}



//	цвет/антиспекл
void	ColourAnalyzer :: Batch()
	{
	size_t i,j, p, q, sub;

	filterSize = ceil_fft_length(n_samples);
	
	ATL_data = (Decide2("Fourier direction", "Standard", "Inverse (ATL interpolator)", 0) == 1);
	
	MeasureSpectrum();
	
	for(size_t ray = 0; ray < n_rays; ++ray)
		{
//		SetCurrentRay(ray);
		for(sub = 0; sub < n_frames; sub++)
			{
//			SetCurrentFrame(sub);
			ComplexFunctionF32	CurrentRay;
			focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
			for(i = 0; i < n_samples; i++) CurrentRay[i] %= phaseCorrection[i];
			}
		}
	
	if(no_colorization) return;
	
	if(!YesOrNo("Perform colourization?", false)) return;
	size_t nL_ax = GetSigned("N layers axial", 3, 1, n_samples/2);
	size_t nL_lat = 3;//GetSigned("N layers lateral", 3, 1, n_rays/2);
	size_t ax2 = nL_ax/2;
	size_t lat2 = nL_lat/2;

//	double	broadFactor = GetFloating("Compress factor", 4./nL_ax, 0.1, 2);
	double	broadFactor = GetFloating("Compress factor", 4./(nL_ax + 1), 0.1, 2);

	ComplexFunction2D_F32	result(n_rays, n_samples);

	RealFunction2D_F32	red(n_rays,n_samples), green(n_rays,n_samples), blue(n_rays,n_samples);

	LayersContainer	total_layers;
	total_layers.realloc(nL_ax, nL_lat, n_rays, n_samples);

	double	filteringFactor = 1;
	if(nL_ax > 1) filteringFactor = 4./(nL_ax-1);
	filteringFactor = GetFloating("Filtering factor", filteringFactor, 0, 4);

	GUIProgressBar	progress;
	progress.start("Filtering layers", nL_ax*nL_lat*n_frames);
	for(p = 0; p < nL_ax; p++)
		{
		double	s1 = p - ax2;

		//if(s1 < 0) s1 *= 2;
		// сдвиг в высокочастотную область принудительно увеличивалсЯ. из-за этого
		// данные почти совсем терЯлись

		for(sub = 0; sub < n_frames; sub++)
			{
//			SetCurrentFrame(sub);
			ComplexFunction2D_F32	Layer(n_rays, filterSize);
			for(size_t ray = 0; ray < n_rays; ++ray)
				{
				ComplexFunctionF32	CurrentRay;
				focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
//				SetCurrentRay(ray);
				Layer.row(ray).CopyData(CurrentRay);
				}

			NormalizeSpectrum(Layer, temporal, s1*filteringFactor, broadFactor);

			for(q = 0; q < nL_lat; q++)
				{
				ComplexFunction2D_F32	Layer2(Layer);
				double	s2 = q - lat2;

				if(nL_lat > 1)
//					NormalizeSpectrum(Layer2, spatial, s2, broadFactor);
					NormalizeSpectrum(Layer2, spatial, s2*1.4, max(1., broadFactor));
//					NormalizeSpectrum(Layer2, spatial, s2, max(1., broadFactor));
//					NormalizeSpectrum(Layer2, spatial, s2/2, broadFactor);
//					NormalizeSpectrum(Layer2, spatial, s2*filteringFactor, broadFactor);

				for(i = 0; i < n_rays; i++)
					{
					for(j = 0; j < n_samples; j++)
						{
						if(n_frames) total_layers.at(p,q).at(i,j) += cabs(Layer2.at(i,j));
						else total_layers.at(p,q).at(i,j) += Layer2.at(i,j);
						// не приравниваю, а прибавлЯю,
						// чтобы не потерЯть данные по субапертурам (хоть это и редко бывает нужно)
						if(!p && !q) result.at(i,j) = cabs(Layer2.at(i,j));
						else result.at(i,j) = min(cabs(result.at(i,j)), cabs(Layer2.at(i,j)));
						}
					}

				++progress;
				}
			}
		}
	progress.end();
//	EndProgress();


	total_layers.AnalyzeLayers();
	total_layers.DisplayLayers();

	progress.start("Computing colour image", n_rays);
	size_t nL_ax_per_color = nL_ax/3;
	for(i = 0; i < n_rays; i++)
		{
		for(j = 0; j < n_samples; j++)
			{
			for(size_t k = 0; k < nL_lat; k++)
				{
				//red.at(i,j) += cabs(total_layers.statistics.at(i,j)[k]);
				//green.at(i,j) += cabs(total_layers.statistics.at(i,j)[k + nL_ax]);
				//blue.at(i,j) += cabs(total_layers.statistics.at(i,j)[k + 2*nL_ax]);

				for(size_t l = 0; l < nL_ax_per_color; l++)
					{
					red.at(i,j) += cabs(total_layers.at(l, k).at(i,j));
					green.at(i,j) += cabs(total_layers.at(l + nL_ax_per_color, k).at(i,j));
					blue.at(i,j) += cabs(total_layers.at(l + 2*nL_ax_per_color, k).at(i,j));
					}
				}
			}
		++progress;
		}
//	EndProgress();

	BuildColourPicture(red, green, blue);

	total_layers.OrderStatistics();
	DisplayStatistics(total_layers);
//	End_Signal_IO();
	}


void	ColourAnalyzer :: DisplayStatistics(const LayersContainer &total_layers)
	{
	enum{
		min_value = 0,
		max_value,
		statistic_n_4,
		median,
		average,
		average_square,
		sampling_width,
		sampling_width_relative,
		sampling_dispersion,
		sampling_dispersion_relative,

		n_options
		};
	size_t answer = min_value;

	while(answer != n_options)
		{
		answer = GetButtonDecision("Gray image options", //n_options+1,
		{
			"Min value",
			"Max value",
			"Statistic N/4",
			"Median",
			"Average",
			"Av. square",
			"Width",
			"Width relative",
			"Dispersion",
			"Disp. relative",
			"Exit"
		}
			);
		if(answer < n_options)
			{
			GUIProgressBar	progress;
			progress.start("Computing gray image", n_rays);
			RealFunction2D_F32	img(n_rays, n_samples);
			for(size_t i = 0; i < n_rays; i++)
				{
				for(size_t j = 0; j < n_samples; j++)
					{
					size_t s = total_layers.statistics[i][j].size();
					double	value;
					switch(answer)
						{
						case min_value:
							value = cabs(total_layers.statistics[i][j][0]);
							break;
						case max_value:
							value = cabs(total_layers.statistics[i][j][s-1]);
							break;
						default:
						case median:
							value = cabs(total_layers.statistics[i][j][s/2]);
							break;
						case statistic_n_4:
							value = cabs(total_layers.statistics[i][j][s/4]);
							break;
						case average:
							value = average_non_coherent(total_layers.statistics[i][j]);
							break;
						case average_square:
							value = average_square_non_coherent(total_layers.statistics[i][j]);
							break;
						case sampling_width:
							value = cabs(total_layers.statistics[i][j][s*3/4]) - cabs(total_layers.statistics[i][j][s/4]);
							break;
						case sampling_width_relative:
							value = (cabs(total_layers.statistics[i][j][s-1]) - cabs(total_layers.statistics[i][j][0]))/cabs(total_layers.statistics[i][j][s/2]);
							break;
						case sampling_dispersion:
							value = deviation_non_coherent(total_layers.statistics[i][j]);
							break;
						case sampling_dispersion_relative:
							value = relative_deviation_non_coherent(total_layers.statistics[i][j]);
							break;
						};
					img.at(i,j) = value;

					}
				++progress;
				}

//			EndProgress();
			if(0)if(answer == sampling_width_relative || answer == sampling_dispersion_relative) if(YesOrNo("Perform additional filtering?", false))
				{
				FIRFilter2DReal	sf;
//				sf.InitFilter(square9);
				sf.IFilterGaussian(3);
//				img.FilterMedian(sf);
				img.Filter(sf);
				}
			BuildGrayPicture(img);
			}
		}

	}


void	ColourAnalyzer :: BuildGrayPicture(const RealFunction2D_F32 &m)
	{
	GrayScanConverter	SC(n_rays, n_samples);
	ExportScanConverterOptions(SC);

	for(size_t i = 0; i < n_rays; i++)
		{
		for(size_t j = 0; j < n_samples; j++)
			{
			SC.at(i,j) = m.at(i,j);
			}
		}

	double	dynRange = 0;//GetFloating("Dynamic range for colour image (0=auto)", 55, 0, HUGE_VAL);

	if(YesOrNo("Log. compress?", true))LogCompressRangeDB(SC, dynRange, 0);

//	SC.SmoothRays(0.025);
//	SC.SetImageTitle("Gray image");
	NormalizeImage(SC, 0,255);
	SC.InitScanConverter();
	SC.BuildConvertedImage();
	DisplayMathFunction2D(SC.GetConvertedImage(), "Gray image");;
	}

void	ColourAnalyzer :: BuildColourPicture(const RealFunction2D_F32 &r, const RealFunction2D_F32 &g, const RealFunction2D_F32 &b)
	{
	ColorScanConverter	SC(n_rays, n_samples);
	ExportScanConverterOptions(SC);

	for(size_t i = 0; i < n_rays; i++)
		{
		for(size_t j = 0; j < n_samples; j++)
			{
			SC.at(i,j) = ColorSampleF64(r.at(i,j), g.at(i,j), b.at(i,j));
			}
		}

	LogCompressRangeHistogram(SC, 0.05, 0.001);


	double	factor = 128;
	double	ab_factor = 5;

	NormalizeImage(SC, 0,255);


	LABColorImage	lab(SC);
	while(YesOrNo("Do Lab correction?", true))
		{
		RealFunctionF32	a1(n_samples), b1(n_samples);
		lab.MakeCopy(SC);

		lab.Display("LAB version)");

		factor = GetFloating("Filtering factor", factor, 1, n_rays);
		ab_factor = GetFloating("ab factor", ab_factor, 1, 400);

		for(size_t i = 0; i < n_samples; i++)
			{
			lab.a.row(i).FilterMedian(double(2*n_rays)/factor);
			lab.b.row(i).FilterMedian(double(2*n_rays)/factor);
			}
		for(size_t i = 0; i < n_rays; i++)
			{
			lab.a.col(i).FilterMedian(double(n_samples)/factor);
			lab.b.col(i).FilterMedian(double(n_samples)/factor);
			}

		for(size_t i = 0; i < n_samples; i++)
			{
			lab.a.row(i).FilterGauss(double(2*n_rays)/factor);
			lab.b.row(i).FilterGauss(double(2*n_rays)/factor);
			}
		for(size_t i = 0; i < n_rays; i++)
			{
			lab.a.col(i).FilterGauss(double(n_samples)/factor);
			lab.b.col(i).FilterGauss(double(n_samples)/factor);
			}
		for(size_t i = 0; i < n_rays; i++)
			{
			a1 += lab.a.col(i);
			b1 += lab.b.col(i);
			}
		a1 /= double(n_rays);
		b1 /= double(n_rays);

		a1.resize(n_samples*8/10);
		b1.resize(n_samples*8/10);
		DisplayMathFunction(a1, 0,1,"a");
		DisplayMathFunction(b1, 0,1,"b");

		//double	a0, a1, a2, b0, b1, b2;
		RealVectorF64	ac(3), bc(3);
		
//		DetectSquare(a, &a0, &a1, &a2);
//		DetectSquare(b, &b0, &b1, &b2);
		
		DetectLSPolynomUniformGrid(a1, ac);
		DetectLSPolynomUniformGrid(b1, bc);
		
		for(size_t i = 0; i < n_samples; i++)
			{
			lab.a.row(i) -= (ac[0] + ac[1]*i + ac[2]*i*1);
			lab.b.row(i) -= (bc[0] + bc[1]*i + bc[2]*i*i);
			}

		lab.a *= ab_factor;
		lab.b *= ab_factor;

		lab.Display("LAB corrected)");
		}
	lab.CopyData(SC);
	for(size_t i = 0; i < n_samples; i++)
		{
		for(size_t j = 0; j < n_rays; j++)
			{
			SC.at(i,j).red() = range(SC.at(i,j).red(), 0, 255);
			SC.at(i,j).green() = range(SC.at(i,j).green(), 0, 255);
			SC.at(i,j).blue() = range(SC.at(i,j).blue(), 0, 255);
			}
		}
//	SC.NormalizeImage(0,255);


//	SC.SmoothRays(0.025);
//	SC.SetImageTitle("Colour image");
	NormalizeImage(SC, 0,255);

	SC.InitScanConverter();
 	SC.BuildConvertedImage();
 	DisplayMathFunction2D(SC.GetConvertedImage(), "Colour image");
 	DisplayMathFunction2D(SC, "Colour image, details");
	}

double	radius(double x, double y)
	{
	return sqrt(x*x + y*y);
	}

void	PutCircle(ColorScanConverter &sc, size_t v, size_t h, double rv, double rh, double b)
	{
	size_t rv2 = max(rv/2., 1.);
	size_t	rh2 = max(rh/2., 1.);
	
	size_t v0 = range(int(v)-rv2, 0, sc.vsize()-1);	
	size_t v1 = range(int(v)+rv2+1, 0, sc.vsize()-1);	

	size_t h0 = range(int(h)-rh2, 0, sc.hsize()-1);	
	size_t h1 = range(int(h)+rh2+1, 0, sc.hsize()-1);	
	
	
	
	
	for(size_t i = v0; i < v1; i++)
		{
		for(size_t j = h0; j < h1; j++)
			{
			double	f = gauss(double(i)-v, rv/4)*gauss(double(j)-h, rh/4)/(rv*rh);
			sc.at(i,j) += f*b;
			}
		}
	
	}

void	ColourAnalyzer :: BuildResonancePicture()
	{
	
	ColorScanConverter	SC(n_rays, n_samples);
	double	scale = double(n_samples)/n_ranges;

	ExportScanConverterOptions(SC);
const	GrayScanConverter	&mb(moment_bandwidth);
const	GrayScanConverter	&mf(moment_frequency);
const	GrayScanConverter	&rb(range_brightness);
	
static	double	rv0 = 4;
static	double	rh0 = 4;
	do
		{
		SC.fill(crayons::black());
		rv0 = GetFloating("resonance ray", rv0, 0, 1000);
		rh0 = GetFloating("resonance sample", rh0, 0, 1000);
		for(size_t i = 1; i < n_rays-1; i++)
			{
			for(size_t j = 1; j < n_ranges-1; j++)
				{
				double	m00 = mb.at(i,j);
				if(	//m00 < mb[i-1][j] && m00 < mb[i+1][j] &&
					m00 < mb.at(i, j-1) && m00 < mb.at(i,j+1) &&
					m00 < mb.at(i-1, j-1) && m00 < mb.at(i+1, j+1) &&
					m00 < mb.at(i-1,j+1) && m00 < mb.at(i+1, j-1)
					){

					double	rv = rv0;
					double	rh = rh0/mf.at(i,j);

					PutCircle(SC, i, j*scale, rv, rh*scale, rb.at(i,j));
					
					}				
				}
			}
		NormalizeImage(SC, 0,255);
		if(0)for(size_t i = 0; i < n_rays; i++)
			{
			for(size_t j = 0; j < n_samples; j++)
				{
				SC.at(i,j).red()  = sample_brightness.at(i,j);
				}
			}
		SC.InitScanConverter();
		SC.BuildConvertedImage();
		DisplayMathFunction2D(SC.GetConvertedImage(), "Resonance markers");		
		DisplayMathFunction2D(rb, "RB");
		}while(YesOrNo("Repeat?", true));
	}
#endif
XRAD_END
