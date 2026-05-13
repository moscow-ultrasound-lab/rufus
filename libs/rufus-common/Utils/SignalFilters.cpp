#include "pre.h"
#include <XRADSystem/Sources/TextFile/text_file.h>
#include <XRADBasic/ContainersAlgebra.h>
#include "SignalFilters.h"
#include <XRADBasic/FFT1D.h>
//#include <GraphSet.h>

XRAD_BEGIN

namespace
{
size_t offset_suppress_zone = 128;//samples
}

void	StaticFrequencyDomainFilter::CalculateFDFilter(RealFunctionF32 &filter, const ComplexFunction2D_F32 &)
{
	size_t filter_size = filter.size();
	size_t offset_zone = filter_size/offset_suppress_zone;

	physical_frequency	df = sample_rate/filter_size;

	// полосовой фильтр, ачх в виде окна блэкмена-наттолла.
	// по умолчанию используем это окно длЯ получениЯ низкого
	// (ок. 100 дб) уровнЯ боковых лепестков в пространственной
	// области
	blackman_nuttall_window window_f;

	// частоты около нулевой подавлЯютсЯ особо, вводЯтсЯ
	// половинки обычного окна наттолла на диапазоны
	// [0,offset_zone) и [filter_size-offset_zone, filter_size)
	nuttall_window low_pass_offset_suppress;

	size_t	filter_start_sample = (filter_f0 - filter_bandwidth)/df;
	size_t	filter_last_sample = (filter_f0 + filter_bandwidth)/df;
	size_t	filter_width = filter_last_sample - filter_start_sample;

	size_t	i0 = max(size_t(0), filter_start_sample);
	size_t	i1 = min(filter_size, size_t(filter_last_sample));

	allowed_downsample_ratio = double(filter_size)/double(filter_last_sample);
	filter.fill(0);
	for(size_t i = i0; i < i1; i++)
	{
		filter[i] = window_f(i-filter_start_sample, filter_width);

		if(i < offset_zone) filter[i] *= low_pass_offset_suppress(i, offset_zone*2);
		if(i > filter_size - offset_zone) filter[i] *= low_pass_offset_suppress(filter_size-i, offset_zone*2);
	}

}


void	StaticFrequencyDomainFilter::InitFilter(physical_frequency f0, physical_frequency bandwidth/*, double flatness*/)
{
	filter_f0 = f0;
	filter_bandwidth = bandwidth;
//	filter_flatness = flatness;
}


void	StaticFrequencyDomainFilter::SetUpFilter(ComplexFunction2D_F32 &frame_buffer, physical_frequency in_sample_rate, physical_length /*z0*/, physical_length /*z_range*/)
{
	sample_rate = in_sample_rate;
	size_t	n_rays = frame_buffer.vsize();
	size_t	n_samples = frame_buffer.hsize();
	size_t	fft_size = ceil_fft_length(n_samples);

	ComplexFunctionF32	ft_buffer(fft_size, complexF32(0));
	RealFunctionF32	sample_magnitude(fft_size, 0);
	RealFunctionF32	filter(fft_size, 0);

	physical_frequency	df = sample_rate/fft_size;


	ft_buffer.CopyData(frame_buffer.row(n_rays/2));
	FFT(ft_buffer, ftForward);
//	CopyData(sample_magnitude, ft_buffer, cabs_functor<float, complexF32>());
	CopyData(sample_magnitude, ft_buffer, [](float &x, const complexF32 &y){return x=cabs(y);});

	size_t	offset_zone = fft_size/offset_suppress_zone;
	for(size_t sample = 0; sample < offset_zone; sample++)
		sample_magnitude[sample] *= sin(pi()*0.5*double(sample)/offset_zone);// исключаем из анализа константу

	sample_magnitude /= MaxValue(sample_magnitude);

	GraphSet	filter_select("Spectrum selection", "MHz", "Magnitude");
	filter_select.AddGraphUniform(sample_magnitude, 0, df.MHz(), "Original spectrum");
	filter_select.AddGraphUniform(filter, 0, df.MHz(), "Filter");


	do
	{
		filter_f0 = MHz(GetFloating("Carrier frequency", filter_f0.MHz(), 0, HUGE_VAL));
		filter_bandwidth = MHz(GetFloating("Bandwidth", filter_bandwidth.MHz(), 0.1, HUGE_VAL));
//		filter_flatness = GetFloating("Filter flatness", filter_flatness, 0, 1);

		filter_select.SetWindowTitle(ssprintf("f0 = %g MHz, band = %g MHz, flatness = %g", filter_f0.MHz(), filter_bandwidth.MHz()/*, filter_flatness*/));

		CalculateFDFilter(filter, frame_buffer);
		filter_select.ChangeGraphUniform(1, filter, 0, df.MHz(), "Filter");
		filter_select.Display(false);
	} while(!YesOrNo("Is filter suitable?", true));
}



void	StaticFrequencyDomainFilter::ApplyFilter(ComplexFunction2D_F32 &frame_buffer)
{
	size_t n_rays = frame_buffer.vsize();
	size_t n_samples = frame_buffer.hsize();

	size_t fft_size = ceil_fft_length(n_samples);
	ComplexFunctionF32	fft_buffer(fft_size, complexF32(0));
	fd_filter.realloc(fft_size);

	CalculateFDFilter(fd_filter, frame_buffer);
	GUIProgressBar	progress;
	progress.start("Filtering (static)", n_rays);
	for(size_t i = 0; i < n_rays; i++)
	{
		fft_buffer.CopyData(frame_buffer.row(i));
		FFT(fft_buffer, ftForward);
		fft_buffer *= fd_filter;
		FFT(fft_buffer, ftReverse);
		frame_buffer.row(i).CopyData(fft_buffer);
		++progress;
	}
}




void	DynamicFrequencyDomainFilter::SaveFilter()
{
	string filename;
	GetFileNameWrite(filename, "Filter file name", "dynamic_filter.note");

	text_file_writer	filter_file;
	filter_file.open_create(filename, text_encoding::utf8);

	filter_file.printf_("Dynamic filter\nn_ranges = %d;\n", n_ranges);

	for(size_t range = 0; range < n_ranges; range++)
	{
		filter_file.printf_("range = %d, f0 = %.1f, bandwidth = %.1f\n", range,
						   filters[range].filter_f0.MHz(),
						   filters[range].filter_bandwidth.MHz());
	}
}

bool	DynamicFrequencyDomainFilter::LoadFilter(physical_length z0, physical_length z_range)
{

	string filename;
	GetFileNameRead(filename, "Filter file name");

	shared_cfile	filter_file;
	filter_file.open(filename, "rb");

	int	n;
	fscanf(filter_file.c_file(), "Dynamic filter\nn_ranges = %d;\n", &n);
	if(size_t(n) != n_ranges)
	{
		Error(ssprintf("Invalid dynamic filter ranges amount!(%d != %d)", n, n_ranges));
		return false;
	}

	RealFunctionF32	f0(n_ranges, 0), bandwidth(n_ranges, 0);
	for(size_t range = 0; range < n_ranges; range++)
	{
		if(fscanf(filter_file.c_file(), "range = %d, f0 = %g, bandwidth = %g\n", &n,
		   &f0[range],
		   &bandwidth[range]) != 3) Error("Invalid params in file");

		filters[range].filter_f0 = MHz(f0[range]);
		filters[range].filter_bandwidth = MHz(bandwidth[range]);
	}

	{
		GraphSet	dynamic_filter_params("Dynamic filter params", "Depth", "");

		dynamic_filter_params.AddGraphUniform(f0, z0.cm(), (z_range/(n_ranges-1)).cm(), "Carrier frequency");
		dynamic_filter_params.AddGraphUniform(bandwidth, z0.cm(), (z_range/(n_ranges-1)).cm(), "Band width");

		GraphScale	gs;
		dynamic_filter_params.GetScale(gs);
		gs.y1() = 0;
		dynamic_filter_params.SetScale(gs);
		dynamic_filter_params.Display(false);
	}

	if(YesOrNo("Is the filter correct?", true)) return true;
	else return false;
}


bool	DynamicFrequencyDomainFilter::LoadUniversalFilter(const string &filename, physical_length z0, physical_length z_range)
{
/*
предполагаемаЯ концепциЯ: "универсальный фильтр" всегда начинаетсЯ с нулЯ
и идет интервалами по 1 см.

в файле фильтра другой заголовок (на всЯкий случай, чтобы они все-таки не открывались как попало
ограничениЯ по числу интервалов, только чтобы хватило длЯ максимальной дальности
*/

	size_t first_range = size_t(z0.cm());
	size_t last_range = size_t((z0 + z_range).cm());
	if(last_range - first_range != n_ranges)
	{
		Error("Invalid filter initialization");
		return false;
	}


	shared_cfile	filter_file;
	try
	{
		filter_file.open(filename.c_str(), "rb");
	}
	catch(file_container_error &)
//	while(!filter_file)
	{
	// чтобы не перезапускать программу, а быстро скопировать недостающий файл и ехать дальше
		Error(ssprintf("Universal filter '%s' not found", filename.c_str()));
		if(YesOrNo("Copy/create file and try again?", true))
		{
			filter_file.open(filename.c_str(), "rb");
		}
		else return false;
	}
	int	n = 0;


	fscanf(filter_file.c_file(), "Universal dynamic filter\nn_ranges = %d;\n", &n);
	if(size_t(n) < last_range)
	{
		Error(ssprintf("Invalid dynamic filter ranges amount!(%d < %d)", n, last_range));
		return false;
	}

	RealFunctionF32	f0(n_ranges, 0), bandwidth(n_ranges, 0);

	for(size_t range = 0; range < first_range; range++)
	{
		if(fscanf(filter_file.c_file(), "range = %d, f0 = %g, bandwidth = %g\n", &n,
		   &f0[0],
		   &bandwidth[0]) != 3)
		{
			Error("Invalid params in file");
			return false;
		}
	}

	for(size_t range = 0; range < n_ranges; range++)
	{
		if(fscanf(filter_file.c_file(), "range = %d, f0 = %g, bandwidth = %g\n", &n,
		   &f0[range],
		   &bandwidth[range]) != 3)
		{
			Error("Invalid params in file");
			return false;
		}
		filters[range].filter_f0 = MHz(f0[range]),
			filters[range].filter_bandwidth = MHz(bandwidth[range]);
	}


	if(1)
	{
		GraphSet	dynamic_filter_params("Dynamic filter params", "Depth", "");

		dynamic_filter_params.AddGraphUniform(f0, z0.cm(), (z_range/(n_ranges-1)).cm(), "Carrier frequency");
		dynamic_filter_params.AddGraphUniform(bandwidth, z0.cm(), (z_range/(n_ranges-1)).cm(), "Band width");

		GraphScale	gs;
		dynamic_filter_params.GetScale(gs);
		gs.y1() = 0;
		dynamic_filter_params.SetScale(gs);
	}

	return true;
}


bool	DynamicFrequencyDomainFilter::LoadFilter(const string &filename)
{
	shared_cfile	filter_file;
	filter_file.open(filename.c_str(), "rb");

	int	n;
	fscanf(filter_file.c_file(), "Dynamic filter\nn_ranges = %d;\n", &n);
	if(size_t(n) != n_ranges)
	{
		Error(ssprintf("Invalid dynamic filter ranges amount!(%d != %d)", n, n_ranges));
		return false;
	}

	for(size_t range = 0; range < n_ranges; range++)
	{
		float	f0, bw;
		if(fscanf(filter_file.c_file(), "range = %d, f0 = %g, bandwidth = %g\n", &n,
		   &f0, &bw) != 3)
		{
			Error("Invalid params in file");
			return false;
		}
		filters[range].filter_f0 = Hz(f0);
		filters[range].filter_bandwidth = Hz(bw);
	}

	return true;
}



void	DynamicFrequencyDomainFilter::ApplyFilter(ComplexFunction2D_F32 &frame_buffer)
{
	size_t	n_rays = frame_buffer.vsize();
	size_t	n_samples = frame_buffer.hsize();

	hann_window window_f;

	double	range_step = n_samples/(n_ranges+1);
	double	range_samples = 2*range_step;

	ComplexFunction2D_F32	segment_buffer(n_rays, range_samples, complexF32(0));
	ComplexFunction2D_F32	segment_reference;
	ComplexFunction2D_F32	result(n_rays, n_samples, complexF32(0));

	GUIProgressBar	progress;
	progress.start("Filtering (Dynamic)", n_ranges);
	for(size_t range = 0; range < n_ranges; range++)
	{
		size_t	range_start = range*range_step;
//TODO urgent: do something here 
//		void	UseDataFragment(DataArray2D &new_data, size_t v0, size_t h0, size_t v1, size_t h1);

		segment_reference.UseDataFragment(frame_buffer, 0, range_start, n_rays, range_start+range_samples);
		segment_buffer.CopyData(segment_reference);
		for(size_t sample = 0; sample < range_samples; sample++)
		{
			double	x = double(sample*2 - range_samples)/range_samples;

			if(!(range == 0 && x < 0) && !(range == n_ranges-1 && x > 0))
			{
				segment_buffer.col(sample) *= window_f(sample, range_samples);
			}
		}

		filters[range].sample_rate = sample_rate;
		filters[range].ApplyFilter(segment_buffer);

//TODO urgent: do something here 
		segment_reference.UseDataFragment(result, 0, range_start, n_rays, range_start+range_samples);

		//DisplayMathFunction2D(segment_reference, "result segment before add");
		segment_reference += segment_buffer;
		//DisplayMathFunction2D(segment_reference, "result segment after add");
//		segment_buffer.PutDataSegment(result, 0, range_start, 1);
		++progress;
	}
	frame_buffer.CopyData(result);
}








namespace
{

void	CutNoise(RealFunctionF32 &cut, const ComplexFunctionF32 &original, double treshold)
{
	if(cut.size() != original.size()) FatalError("Invalid noise cut buffer");

	cut.CopyData(original, [](float &x, const complexF32 &y){return x=cabs(y);});
//	cut.CopyData(original, Functors::absolute_value());
	cut.FilterMedian(15);
	double	max_value = MaxValue(cut);


	for(size_t i = 0; i < cut.size(); i++)
	{
		if(cut[i] < max_value*treshold) cut[i] = 0;
	}
}

double	TresholdForRange(size_t range_no, size_t n_ranges)
{
//static const	double	min_noise = 0.25;
//static const	double	max_noise = 0.95;
	static const	double	min_noise = 0.05;
	static const	double	max_noise = 0.75;

	static const	double	s0 = 1./min_noise - 1.;
	static const	double	alpha = log((1./max_noise - 1.)/s0);

	double	q = double(range_no)/double(n_ranges-1);
	return 1./(1. + s0*exp(q*alpha));
}
}


void	DynamicFrequencyDomainFilter::SetUpFilter(ComplexFunction2D_F32 &frame_buffer, physical_frequency in_sample_rate, physical_length z0, physical_length z_range)
{
	size_t n_rays = frame_buffer.vsize();
	size_t n_samples = frame_buffer.hsize();

	size_t n = GetUnsigned("Dynamic filter ranges number", z_range.cm(), 3, max(size_t(z_range.cm()), n_samples/256));
	InitDynamicFilter(n, GetFloating("Dynamic filter flatness", 0.5, 0, 1));
	bool	show_cut = true;

	double	range_step = n_samples/(n_ranges+1);
	double	range_samples = 2*range_step;
	size_t	fft_size = ceil_fft_length(range_samples);
	ComplexFunctionF32	fft_buffer(fft_size, complexF32(0));
	ComplexFunctionF32	display_signal_buffer(fft_size, complexF32(0));
	RealFunctionF32	cut_noise_buffer(fft_size, 0);
	RealFunctionF32	filter(fft_size, 0);

	physical_frequency	df = in_sample_rate/fft_size;


	if(YesOrNo("Load universal filter?", true))
	{
		if(LoadUniversalFilter("universal_filter.note", z0, z_range)) return;
	}

	if(YesOrNo("Load dynamic filter?", true))
	{
		if(LoadFilter(z0, z_range)) return;
	}
	GraphSet	dynamic_filter_params("Filter params", "Depth", "");
	GraphSet	filter_select("Spectrum selection", "MHz", "");

	filter_select.AddGraphUniform(real(fft_buffer), 0, df.MHz(), "Original spectrum");
	filter_select.AddGraphUniform(filter, 0, df.MHz(), "Filter");
	if(show_cut) filter_select.AddGraphUniform(cut_noise_buffer, 0, df.MHz(), "Spectrum with noise cut");
	filter_select.Display(false);

	RealFunctionF32	f0(n_ranges, 0), bandwidth(n_ranges, 0);

	dynamic_filter_params.AddGraphUniform(f0, z0.cm(), (z_range/(n_ranges-1)).cm(), "Carrier frequency");
	dynamic_filter_params.AddGraphUniform(bandwidth, z0.cm(), (z_range/(n_ranges-1)).cm(), "Band width");
	dynamic_filter_params.Display(false);

	for(size_t range = 0; range < n_ranges; range++)
	{
		filters[range].sample_rate = in_sample_rate;
		size_t range_start = range*range_step;
		size_t cycle_count = 0;
		char	title[256];
		sprintf(title, "Spectrum selection, range # %d of %d", int(range+1), int(n_ranges));

		filter_select.SetWindowTitle(title);
		bool	change_filter_params = true;
		do
		{
			if(!cycle_count)
			{
				display_signal_buffer.fill(complexF32(0));
				for(size_t ray = 0; ray < n_rays; ray++)
				{
					fft_buffer.fill(complexF32(0));
					for(size_t sample = 0; sample < range_samples; sample++)
					{
						fft_buffer[sample] = frame_buffer.at(ray,range_start + sample);
					}
					ApplyWindowFunction(fft_buffer, cos2_window());
					FFT(fft_buffer, ftForward);
				#pragma message Проблема с функтором
					ApplyFunction(fft_buffer, [](complexF32 &b){return cabs(b);});
//					ApplyFunction(fft_buffer, cabs_functor<complexF32, complexF32>());
					display_signal_buffer += fft_buffer;

				}
				for(size_t sample = fft_size/2; sample < fft_size; sample++) display_signal_buffer[sample] = 0;
				size_t offset_zone = fft_size/offset_suppress_zone;
				for(size_t sample = 0; sample < offset_zone; sample++)
					display_signal_buffer[sample] *= sin(pi()*0.5*double(sample)/offset_zone);// исключаем из анализа константу

				display_signal_buffer /= MaxValue(display_signal_buffer);
				CutNoise(cut_noise_buffer, display_signal_buffer, TresholdForRange(range, n_ranges));

//				double	ff0, fbw;
				RealVectorF32 moments(2);

				WeightMoments(cut_noise_buffer, moments, true);
				f0[range] = moments[0]*df.MHz();
				bandwidth[range] = sqrt(moments[1])*6*df.MHz();
				physical_frequency	carrier = df*fft_size;
				if(f0[range] + bandwidth[range]/2 > 0.875*carrier.MHz()/2) bandwidth[range] = (0.875*carrier.MHz()/2 - f0[range])*2;

				filters[range].filter_f0 = MHz(f0[range]);
				filters[range].filter_bandwidth = MHz(bandwidth[range]);

				filters[range].CalculateFDFilter(filter, frame_buffer);
				filter_select.ChangeGraphUniform(0, real(display_signal_buffer), 0, df.MHz(), "Original spectrum");
				filter_select.ChangeGraphUniform(1, filter, 0, df.MHz(), "Filter");
				if(show_cut) filter_select.ChangeGraphUniform(2, cut_noise_buffer, 0, df.MHz(), "Spectrum with noise cut");
			}
			change_filter_params = YesOrNo("Change filter params?", false);
			if(change_filter_params)
			{
				f0[range] = GetFloating("Filter carrier", f0[range], 0, in_sample_rate.MHz()/2);
				bandwidth[range] = GetFloating("Filter bandwidth", bandwidth[range], 0, in_sample_rate.MHz()/2);
			}

			filters[range].filter_f0 = MHz(f0[range]);
			filters[range].filter_bandwidth = MHz(bandwidth[range]);

			filters[range].CalculateFDFilter(filter, frame_buffer);
			filter_select.ChangeGraphUniform(0, real(display_signal_buffer), 0, df.MHz(), "Original spectrum");
			filter_select.ChangeGraphUniform(1, filter, 0, df.MHz(), "Filter");

			dynamic_filter_params.ChangeGraphUniform(0, f0, z0.cm(), z_range.cm()/(n_ranges-1), "Carrier frequency");
			dynamic_filter_params.ChangeGraphUniform(1, bandwidth, z0.cm(), z_range.cm()/(n_ranges-1), "Band width");

			GraphScale	gs;
			dynamic_filter_params.GetScale(gs);
			gs.y1() = 0;
			dynamic_filter_params.SetScale(gs);

			filter_select.Display(false);
			cycle_count++;
			if(change_filter_params) change_filter_params = !YesOrNo("Is the filter correct?", false);
		} while(change_filter_params);
	}
	if(YesOrNo("Save dynamic filter?", true)) SaveFilter();
}

XRAD_END
