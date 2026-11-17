#include "pre.h"
#include "WallFiltersObsolete.h"

//------------------------------------------------------------------
//
//	created:	2021/03/10	15:21
//	filename: 	WallFiltersObsolete.cpp
//	file path:	Q:\Projects\CommonSources\DopplerBasics\CFM\WallFilters
//	author:		kns
//	
//	purpose:	
//
//------------------------------------------------------------------

namespace xrad
{


// фильтрация фурье-спектра
void WallFilterFourier::Apply(ComplexFunctionF32& burst) const
{
	throw invalid_argument("This filter is obsolete");
	fft_buffer.UseDataFragment(burst, 0, fft_size, 1);

	FFT(fft_buffer, ftForward);
	fft_buffer *= filter;
	FFT(fft_buffer, ftReverse);

	for(size_t i = fft_size; i < burst.size(); ++i) burst[i] = 0;
}

WallFilterFourier::WallFilterFourier(size_t in_burst_size)
{
	fft_size = ceil_fft_length(in_burst_size);
	if(fft_size>in_burst_size) fft_size /= 2;
	filter.realloc(fft_size, 1);
}
//
//-------------------------------------------------------------------


//
//-------------------------------------------------------------------
void WallFilterKL::UpdateFilter(ComplexFunctionMD_F32& frame_by_shots, size_t first_beam, size_t last_beam)
{
#if 1
	throw runtime_error("WallFilterKL, filter disabled");
#else
	size_t	n_shots = frame_by_shots.sizes(1);
	ComplexMatrixF32 R(n_shots, n_shots, complexF32(0));
	ComputeCorrelationMatrix(R, frame_by_shots, first_beam, last_beam);
	ComplexMatrixF32 eigenvectors(n_shots, n_shots, complexF32(0));
	RealVectorF32 eigenvalues(n_shots, 0);
	eigenvectors_hermit(R, eigenvectors, eigenvalues);
	//TODO отключено в связи с отказом от комплексной матрицы
	//	SetMatrix(real(eigenvectors));
#endif
}



void WallFilterGaussian::UpdateFilter() const
{
	double	M(0), m(0);
	for(size_t i = 0; i < fft_buffer.size(); ++i)
	{
		double	value = cabs(fft_buffer[i]);
		M += i*value;
		m += value;
	}
	double weight_center(double((fft_buffer.size())/2));
	if(m > 0)
	{
		weight_center = M/m;
	}
	for(size_t i = 0; i < fft_buffer.size(); ++i)
	{
		double	x = fabs(double(i) - weight_center);
		filter[i] = 1. - exp(-0.5*pow(x/bandwidth, power));
	}
}

void WallFilterGaussian::Apply(ComplexFunctionF32& burst) const
{
	fft_buffer.UseDataFragment(burst, 0, fft_size, 1);

	FFT(fft_buffer, ftForward);
	fft_buffer.roll_half(1);

	UpdateFilter();
	fft_buffer *= filter;

	fft_buffer.roll_half(0);
	FFT(fft_buffer, ftReverse);
	for(size_t i = fft_size; i < burst.size(); ++i) burst[i] = 0;
}

WallFilterGaussian::WallFilterGaussian(size_t burst_size, double in_STD, double in_power) : bandwidth(in_STD), power(in_power), WallFilterFourier(burst_size){}

//-------------------------------------------------------------------

void WallFilterButterworth::Init(float in_cutoff_frequency, float in_order, size_t in_fft_size)
{
	cutoff_frequency = in_cutoff_frequency;
	order = in_order;
	fft_size = in_fft_size;
	filter.realloc(fft_size);
	cfilter.realloc(fft_size);
	std::fill(cfilter.begin(), cfilter.end(), complexF64(1));
	for(size_t i = 0; i < fft_size; ++i)
	{
		double	omega = (i - fft_size / 2.) / cutoff_frequency;
		filter[i] = 1. - sqrt(1. / (1. + pow(omega, 2.*order)));
		double beta = tan(cutoff_frequency*pi() / 2);
		complexF64 s = 1./polar(1, pi()/2)*omega;
		cfilter[i] = beta*beta / (s*s + sqrt_2()*beta*s + beta*beta);
	}
}

void WallFilterButterworth::Apply(ComplexFunctionF32& burst) const
{
	ComplexFunctionF32 new_burst(burst);
	ComplexFunctionF32	fft_buffer;
	fft_buffer.UseDataFragment(new_burst, 0, fft_size, 1);
	FFT(fft_buffer, ftForward);
	fft_buffer.roll_half(true);
	fft_buffer *= cfilter;
	fft_buffer.roll_half(false);
	FFT(fft_buffer, ftReverse);
	for(size_t i = fft_size; i < burst.size(); ++i)
	{
		burst[i] = 0;
		new_burst[i] = 0;
	}
	burst = new_burst;
}

void WallFilterButterworth_recursive::Init(float in_cutoff_frequency)
{
	cutoff_frequency = in_cutoff_frequency;
	b.realloc(3);
	a.realloc(3);

	double beta = tan(cutoff_frequency*pi()/2);
	double norm = 1 / (1 + sqrt_2() * beta + beta * beta);
	a[2] = (1 - sqrt_2() * beta + beta*beta) * norm;
	a[1] = (2 - 2 * beta * beta) * norm;
	b[2] = beta * beta * norm;
	b[1] = -2 * b[2];
	b[0] = b[2];
}

void WallFilterButterworth_recursive::Apply(ComplexFunctionF32& burst) const
{
	ComplexFunctionF32 x(3, complexF32(0)), y(3, complexF32(0));
	for(size_t i = 0; i < burst.size(); i++)
	{
		x[2] = x[1];
		x[1] = x[0];
		x[0] = burst[i];

		y[2] = y[1];
		y[1] = y[0];
		y[0] = b[0]*x[0] + b[1]*x[1] + b[2]*x[2] - a[1]*y[1] - a[2]*y[2];

		burst[i] = y[0];
	}
};

}//namespace xrad
