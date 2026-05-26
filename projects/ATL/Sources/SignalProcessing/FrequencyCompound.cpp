#include "Pre.h"
#include "FrequencyCompound.h"
//#include "NonUniformMotionAnalysis.h"
#include <XRADSystem/Sources/TextFile/text_file.h>

XRAD_BEGIN


struct plus_cabs_assign //functor for operator+=
	{
	void operator()(float &x, const complexF32 &y) const { x+=cabs(y); }
	};



//-----------------------------------------------------------------------------------------------------
//
//	resampling utils
//

void	UpsampleTDToTD(ComplexFunctionF32 &upsampled, const ComplexFunctionF32 &original)
	{
	static	UniversalInterpolator<FilterKernelComplex>	complex_sinc_LARGE;
	do_once
		{
		complex_sinc_LARGE.InitFilters(32, SincComplexFilterGenerator(48, 0.5));
		};

	size_t up_size = upsampled.size();
	size_t original_size = original.size();
	double	upsample_ratio = double(up_size)/double(original_size);
	for(size_t i = 0; i < up_size; ++i)
		{
//		upsampled[i] = original.in(double(i)/upsample_ratio, &interpolators::complex_sinc);
		upsampled[i] = original.in(double(i)/upsample_ratio, &complex_sinc_LARGE);
		}
	}

void	DownsampleTDToTD(ComplexFunctionF32 &downsampled, const ComplexFunctionF32 &original)
	{
	size_t downsampled_size = downsampled.size();
	size_t original_size = original.size();
	
	if(original_size%downsampled_size)
		{
		double	downsample_ratio = double(original_size)/double(downsampled_size);
		for(size_t i = 0; i < downsampled_size; ++i)
			{
			downsampled[i] = original.in(double(i)*downsample_ratio, &interpolators::complex_sinc);
			}
		}
	else
		{
		size_t downsample_ratio = original_size/downsampled_size;
		for(size_t i = 0; i < downsampled_size; ++i)
			{
			downsampled[i] = original[i*downsample_ratio];
			}
		}
	}


void	UpsampleFilterSpectrumToTD(ComplexFunctionF32 &upsampled_time_domain, const RealFunctionF32 &original_spectrum)
	{
	size_t	up_size = upsampled_time_domain.size();
	size_t	original_size = original_spectrum.size();
	if(ceil_fft_length(original_size) != original_size)
		{
		FatalError("Invalid resample data size");
		}
	
//	size_t upsample_ratio = up_size/original_size;

	size_t internal_upsample_ratio = 16;
	size_t internal_up_size = original_size*internal_upsample_ratio;
	
	ComplexFunctionF32	internal_upsampled_time_domain(internal_up_size, complexF32(0));
	
	for(size_t i = 0; i < original_size; ++i)
		{
		if(i) internal_upsampled_time_domain[i] = internal_upsampled_time_domain[internal_up_size - i] = original_spectrum[i];
		else internal_upsampled_time_domain[i] = original_spectrum[i];
		}
	FFTf(internal_upsampled_time_domain, fftRevRollAfter);
	
	// resample to final size
	for(size_t i = 0; i < up_size; ++i)
		{
//		upsampled_time_domain[i] = internal_upsampled_time_domain.in(double(i*internal_up_size)/up_size , &interpolators::sinc);
		upsampled_time_domain[i] = internal_upsampled_time_domain.in(double(i*internal_up_size)/up_size , &interpolators::sinc);
		// если нечетный, то x+0,5
		}
	}



void	ShortenTDFilter(RealFunctionF32 &tdf, const ComplexFunctionF32 &data)
	{
	size_t data_size = data.size();
	size_t filter_size = tdf.size();
	size_t ds2 = data_size/2;
	size_t fs2 = filter_size/2;
//	size_t fsd = filter_size%2;
	
	for(size_t i = 0; i < filter_size; ++i)
		{
		tdf[i] = real(data[i-fs2 + ds2]);
		}
	}

//
//-----------------------------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------------------------
//
//



template<class FILTER_T>
void TimeDomainFilter<FILTER_T> :: ApplyFilter(ComplexFunction2D_F32 &frame_buffer)
	{
	size_t upsample_ratio = 6;
	size_t n_rays = frame_buffer.vsize();
	size_t n_samples = frame_buffer.hsize();
	td_filter.realloc(td_filter_order);

	if(1)	// вычисление временного фильтра
		{
		size_t ft_size = ceil_fft_length(n_samples);
		fd_filter.realloc(ft_size);
		parent::CalculateFDFilter(fd_filter, frame_buffer);
		
		
		size_t filter_up_size = ft_size*upsample_ratio;

		ComplexFunctionF32	up_td_filter(filter_up_size);

		UpsampleFilterSpectrumToTD(up_td_filter, fd_filter);
		ShortenTDFilter(td_filter, up_td_filter);
		
		td_filter -= AverageValue(td_filter);
		ApplyWindowFunction(td_filter, hamming_window());
		
		// ничтожнаЯ асимметричность фильтров, длЯ порЯдка устранЯем и ее
		size_t s = td_filter.size();
		for(size_t i = 0; i < s/2; ++i)
			{
			td_filter[i] = (td_filter[i] + td_filter[s-i-1])/2;
			td_filter[s-i-1] = td_filter[i];
			}

		filter_gain = MaxValue(td_filter);
		td_filter /= filter_gain;
		
		if(CapsLock())
			{
			DisplayMathFunction(fd_filter, 0,1,"FD original");
			DisplayMathFunction(up_td_filter, -int(filter_up_size)/2,1,"TD filter");
			DisplayMathFunction(td_filter, 0,1,"td_filter");
			}
		}

	size_t data_upsampled_size = upsample_ratio*n_samples;
	ComplexFunctionF32	convolver(data_upsampled_size);
	
	FilterKernelReal	tdf;
	tdf.MakeCopy(td_filter);
	for(size_t ray = 0; ray < n_rays; ray++)
		{
		UpsampleTDToTD(convolver, frame_buffer.row(ray));
		convolver.Filter(tdf);
		DownsampleTDToTD(frame_buffer.row(ray), convolver);
		frame_buffer.row(ray) *= filter_gain;
		}
	}


























void	FilterSet::DisplayFilters()
	{
	GraphSet filter_displayer("Filters", "x", "y");
	GraphSet td_filter_displayer("TD Filters", "x", "y");
	for(size_t i = 0; i < size(); ++i)
		{
		filter_displayer.AddGraphUniform(at(i).fd_filter, 0, 1, "Force FD filter");
		td_filter_displayer.AddGraphUniform(at(i).td_filter, 0, 1, "Force TD filter");
		filter_gain[i] = at(i).filter_gain;
		}
	filter_gain /= MaxValue(filter_gain);
	DisplayMathFunction(filter_gain, 0,1,"Filter gain");
	DisplayMathFunction(filter_attenuation, 0,1,"Filters attenuation");
	}

void	FilterSet::WriteFilters(const char *filter_file_name, size_t n_samples)
	{
	char	fn[256];
	for(size_t i = 0; i < size(); ++i)
		{
		sprintf(fn, "%s_%d.txt", filter_file_name, int(i+1));
		text_file_writer filter_file;
		filter_file.open_create(fn, text_encoding::utf8);
		filter_file.printf_("filter No %d\n", i+1);
		filter_file.printf_("static gain = %g dB\n", 10*log10(filter_gain[i]));
		filter_file.printf_("dynamic gain = %g dB per sample\n", 10*log10(filter_attenuation[i])/n_samples);
		filter_file.printf_("Filter order = %d\n", at(i).td_filter.size());

		filter_file.printf_("\nFilter coefficients:\n\n");
		
		for(size_t j = 0; j < at(i).td_filter.size(); ++j)
			{
			filter_file.printf_("%d\t%g\n", j, at(i).td_filter[j]);
			}
		}
	}


//float	attenuation_table[4] = {1.0, 0.75, 0.1, 0.01};	
float	attenuation_table[4] = {1.0, 0.75, 0.5, 0.25};	
//float	attenuation_table[4] = {1.0, 0.875, 0.75, 0.625};	
	
void	FilterSet :: InitSignalFilters(size_t nsf, double max_frequency_factor)
	{	
	
	double	bandwidth_factor = 1;
	physical_frequency	edge_frequency = sample_rate;
	size_t n_signal_filters = nsf;
	
	physical_frequency	default_halfwidth = 0.5 * max_frequency_factor * sample_rate * bandwidth_factor/n_signal_filters;
	
	min_frequency = default_halfwidth*2;
	max_frequency = max_frequency_factor*sample_rate - default_halfwidth;
	//симметричный отступ?

	default_bandwidth = 1.5 * (max_frequency - min_frequency) * bandwidth_factor/n_signal_filters;		
		
	realloc(n_signal_filters);

	filter_gain.realloc(n_signal_filters, 1.);
	
	if(n_signal_filters==4) 
	{
		filter_attenuation.realloc(n_signal_filters);
		filter_attenuation.CopyData(attenuation_table);
	}
	else
	{
		filter_attenuation.realloc(n_signal_filters, 1.);
	}
	
	for(size_t i = 0; i < n_signal_filters; ++i)
		{
		at(i).sample_rate = sample_rate;
		at(i).filter_f0 = min_frequency + (max_frequency - min_frequency)*double(i)/(n_signal_filters-1);
		at(i).filter_bandwidth = default_bandwidth;
		
//		at(i).filter_flatness = 0.25;
		}	
	}




void	ForceSpectrumFilter :: CalculateFDFilter(RealFunctionF32 &filter, const ComplexFunction2D_F32 &frame_buffer)
	{
	size_t n_rays = frame_buffer.vsize();
	size_t n_samples = frame_buffer.hsize();

	size_t ft_size = filter.size();
	ComplexFunctionF32	ft_buffer(ft_size);
	
	parent::CalculateFDFilter(filter, frame_buffer);
	RealFunctionF32	filter_divisor(ft_size, 0);

	for(size_t i = 0; i < n_rays; i++)
		{
		ft_buffer.CopyData(frame_buffer.row(i));
		FFT(ft_buffer, ftForward);
		
		AA_1D_OpEq(filter_divisor, ft_buffer, plus_cabs_assign());
		}
	
	filter_divisor.FilterGauss(n_samples/32);
	filter /= filter_divisor;
	}


FrequencyCompound :: FrequencyCompound()
	{
	static size_t N = 2;
	N = GetSigned("Compopund preset no", N, 1, 6);
	switch(N)
		{
		case 1:
			n_signal_filters = 3;
			break;
		
		case 2:
			n_signal_filters = 4;
			break;
		
		case 3:
			n_signal_filters = 5;
			break;

		case 4:
			n_signal_filters = 6;
			break;

		case 5:
			n_signal_filters = 9;
			break;

		case 6:
			n_signal_filters = 12;
			break;
		};
	++N;
	
//	InitSignalFilters(n_signal_filters, 0.65);

//	SetFrequencyUnits(M_HERZ);
	SignalFilters.sample_rate = sample_rate;
//	SignalFilters.InitSignalFilters(n_signal_filters, 0.9);
	
	
	frame_buffer.realloc(n_signal_filters);	
	for(size_t i = 0; i < n_signal_filters; ++i)
		{
		frame_buffer[i].realloc(n_rays, n_samples);
		}

	Display("Original data");
	}

	
void	FrequencyCompound :: InitWork()
	{
	}


void	FrequencyCompound :: FilterFrame(size_t frame, ComplexFunction2D_F32 &buffer)
	{
//	size_t n = 0;

//	SetCurrentFrame(frame);
	ComplexFunction2D_F32 CurrentFrame;
	focused_data.GetSlice(CurrentFrame, {frame, slice_mask(0), slice_mask(1)});
	for(size_t filter_no = 0; filter_no < n_signal_filters; ++filter_no)
		{
		CopyData(frame_buffer[filter_no], CurrentFrame);
		SignalFilters[filter_no].ApplyFilter(frame_buffer[filter_no]);
		frame_buffer[filter_no] *= SignalFilters.filter_gain[filter_no];
		
//		if(filter_no != n)
//			frame_buffer[filter_no].fill(complexF32(0));
		
		size_t n_samples1 = frame_buffer[filter_no].hsize();
		RealFunctionF32	factors(n_samples1);
		for(size_t sample = 0; sample < n_samples1; ++sample)
			{
			double	f1 = double(sample)/n_samples1;
			double	f2 = log(SignalFilters.filter_attenuation[filter_no]);
			factors[sample] = exp(f1*f2);
			frame_buffer[filter_no].col(sample) *= factors[sample];
			}
//		if(filter_no==n) factors.Display(0,1,"Factors");
//		ApplyFunction(frame_buffer[filter_no], cabs2_functor());
		ApplyFunction(frame_buffer[filter_no], cabs_functor<complexF32, complexF32>());
		
//		++progress;
		}

	buffer.CopyData(frame_buffer[0]);

	for(size_t filter_no = 0; filter_no < n_signal_filters; ++filter_no)
		{
		buffer += frame_buffer[filter_no];
		}
	}

void	FrequencyCompound :: Batch()
	{
//	ProcessDynamicFilters();
	ProcessStaticFilters();
	}




void	FrequencyCompound :: ProcessStaticFilters()
	{
//	const size_t n_ranges = 3;
	double	upper_frequency = 0.9;
	
//	size_t sw = n_samples/2;
//	size_t s0 = 0;
//	size_t s1 = n_samples/4;
//	size_t s2 = n_samples/2;
//	size_t s3 = 3*n_samples/4;
//	size_t s4 = n_samples;

	
	GUIProgressBar	progress;
	progress.start("Processing compound", n_frames);

	ComplexFunction2D_F32	result_buffer(n_rays, n_samples);
	
	for(size_t frame = 0; frame < n_frames; ++frame)
		{
		SignalFilters.InitSignalFilters(n_signal_filters, upper_frequency);
		FilterFrame(frame, result_buffer);		
		++progress;
		
//		SetCurrentFrame(frame);
		for(size_t ray = 0; ray < n_rays; ++ray)
			{
//			SetCurrentRay(ray);
			ComplexFunctionF32	CurrentRay;
			focused_data.GetRow(CurrentRay, {frame, ray, slice_mask(0)});
			CurrentRay.CopyData(result_buffer.row(ray));
			}
		}
	progress.end();

	SignalFilters.DisplayFilters();
	try
		{
		string filter_file_name;
		GetFileNameWrite(filter_file_name, "Filter filename");
		SignalFilters.WriteFilters(filter_file_name.c_str(), n_samples);
		}
	catch(canceled_operation &)
		{
		}
	}
	



	
void	FrequencyCompound :: ProcessDynamicFilters()
	{
	const size_t n_ranges = 3;
	double	bounds_for_ranges[n_ranges] = {0.9, 0.7, 0.5};

//	size_t sw = n_samples/2;
	size_t s0 = 0;
	size_t s1 = n_samples/4;
	size_t s2 = n_samples/2;
	size_t s3 = 3*n_samples/4;
	size_t s4 = n_samples;

	
	GUIProgressBar	progress;
	progress.start("Processing compound", n_frames*n_signal_filters*n_ranges);

	DataArray<ComplexFunction2D_F32>	result_buffer;
	result_buffer.realloc(n_ranges);
	for(size_t range = 0; range < n_ranges; ++range)
		{
		result_buffer[range].realloc(n_rays, n_samples);
		}

	
	for(size_t frame = 0; frame < n_frames; ++frame)
		{
		for(size_t range = 0; range < n_ranges; ++range)
			{
			SignalFilters.InitSignalFilters(n_signal_filters, bounds_for_ranges[range]);
			FilterFrame(frame, result_buffer[range]);
			SignalFilters.DisplayFilters();
			}
		
//		SetCurrentFrame(frame);
		for(size_t ray = 0; ray < n_rays; ++ray)
			{
			ComplexFunctionF32	CurrentRay;
			focused_data.GetRow(CurrentRay, {frame, ray, slice_mask(0)});
//			SetCurrentRay(ray);
			CurrentRay.fill(complexF32(0));
			for(size_t range = 0; range < n_ranges; ++range)
				{
				if(range==0) ApplyWindowFunction(result_buffer[range].row(ray), constant_window(), hann_window(), s0, s2);
				else if(range==1) ApplyWindowFunction(result_buffer[range].row(ray), hann_window(), s1, s3);
				else if(range==2) ApplyWindowFunction(result_buffer[range].row(ray), hann_window(), constant_window(), s2, s4);
				
				
				CurrentRay += result_buffer[range].row(ray);
				}
			}
		}
	progress.end();

	}
	


	



	
XRAD_END
