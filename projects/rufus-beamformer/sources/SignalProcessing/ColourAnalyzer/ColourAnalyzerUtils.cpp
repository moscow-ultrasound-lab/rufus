#include "pre.h"


//#include <PCM_Wave.h>

//Код сильно устарел, но некоторые заложенные в нем идеи могут быть интересны в будущем. Поэтому не уничтожаю, но и не трачу сил на полную адаптацию

#if 1

#include <XRADBasic/Sources/Utils/StatisticUtils.h>
#include <Attic/LABColorImage.h>
#include <XRADBasic/Sources/Utils/LeastSquares.h>

#include "SignalProcessing/GaussSignalTools.h"
#include "ColourAnalyzer.h"
#include <Utils/SignalFilters.h>
#include <XRADBasic/Sources/SampleTypes/HLSColorSample.h>
#include "ColourEncode.h"
#include "ColourAnalyzerUtils.h"



XRAD_BEGIN


//---------------------------------------------------------------------------
//
//	функционалы
//
//---------------------------------------------------------------------------



double	average_non_coherent(const ComplexFunctionF32 &f)
	{
	return ElementSumTransformed(f, cabs_functor<float, complexF32>())/f.size();
	}

double	average_square_non_coherent(const ComplexFunctionF32 &f)
	{
	return sqrt(ElementSumTransformed(f, cabs2_functor<float, complexF32>())/f.size());
	}

double	deviation_non_coherent(const ComplexFunctionF32 &f)
	{
	RealFunctionF32	f1(f.size());
	CopyData(f1, f, Functors::assign_f1(cabs_functor<float, complexF32>()));
	return StandardDeviation(f1);
	}

double	relative_deviation_non_coherent(const ComplexFunctionF32 &f)
	{
	RealFunctionF32	f1(f.size());
	CopyData(f1, f, Functors::assign_f1(cabs_functor<float, complexF32>()));
	double	a = AverageValue(f1);
	if(a) return StandardDeviation(f1)/a;
	else return 0;
	}


//---------------------------------------------------------------------------




extern	bool	ATL_data;

void	CutTreshold(RealFunctionF32 &f, double treshold)
	{
	ApplyFunction(f,fabs_functor<float,float>());
//	f.FilterMedian(5);
	double	m = MaxValue(f);
	for(size_t i = 0; i < f.size(); i++)
		{
		if(f[i] < m*treshold) f[i] = 0;
		}
	}


void	RangeData(RealFunction2D_F32 &m, double mn, double mx)
	{
	size_t vs = m.vsize();
	size_t hs = m.hsize();
	
	for(size_t i = 0; i < vs; i++)
		{
		for(size_t j = 0; j < hs; j++)
			{
			if(m.at(i, j) < mn) m.at(i, j) = mn;
			if(m.at(i, j) > mx) m.at(i, j) = mx;
			}
		}
	}


double	CountMaxima1(const ComplexFunctionF32 &f)
	{
	double	maxima = 0;
	double	minima = 0;
	double	abs_max = cabs(MaxValue(f));
	if(!abs_max) return 0;
	
	size_t s0 = 1;//f.size()/4;
	size_t s1 = f.size();
	
	for(size_t i = s0; i < s1; i++)
		{
		maxima += cabs(f[i]-f[i-1]);
		minima += cabs(f[i]);
		}
	return maxima/minima;
	}




double	CountMaxima1D(const RealFunctionF32 &f)
	{
	size_t s = f.size(), n = 0;
	RealFunctionF32	mx(s);
	double	m = MaxValue(f);
	if(!m) return 0;
	double	result;
	for(size_t i =  1; i < s-1; i++)
		{
		if(f[i]>f[i-1] && f[i]>f[i+1])
			{
			mx[n++]=f[i];
			}
		}
	//mx.resize(n);
	//result = n;
	result = 0;
	double	c = 0;
	for(size_t i = 0; i < n; i++)
	{
//		if(mx[i]*20 < m) --result;
		if(mx[i]*20 >= m)
			{
			result += mx[i]/m;
			c++;
			}
		}
	return result;//c;
	}

double	CountMaxima(const ComplexFunction2D_F32 &f)
	{
	double	maxima = 0;
	double	abs_max = cabs(MaxValue(f));
	if(!abs_max) return 0;
	
	size_t vs = f.vsize();
//	size_t hs = f.hsize();
//	size_t s0 = 1;//f.size()/4;
//	size_t s1 =hs - 1;
	
	
	for(size_t j = 0; j < vs; j++)
		{
		maxima += CountMaxima1D(real(f.row(j)));
		}
	return maxima;
	}



void	ComputeLocalSpectrum(ComplexFunction2D_F32 &ft_buffer)
	{
	for(size_t i = 0; i < ft_buffer.vsize(); i++)
		{
		ApplyWindowFunction(ft_buffer.row(i), hamming_window());
		if(ATL_data)FFT(ft_buffer.row(i), ftReverse);
		else FFT(ft_buffer.row(i), ftForward);
		ApplyWindowFunction(ft_buffer.row(i), cos2_window());
		}
	
	if(CapsLock())
		{
		DisplayMathFunction2D(ft_buffer, "FT_buffer", ScanFrameRectangle(cm(ft_buffer.hsize())/5, cm(ft_buffer.vsize())/5));
		}
	}

void	ComputeAverageSpectrum(const ComplexFunction2D_F32 &ft_buffer, RealFunctionF32 &ft_accumulator, RealFunctionF32 &ft_accumulator_filtered)
	{
	ft_accumulator.fill(0);
	for(size_t i = 0; i < ft_buffer.vsize(); i++)
		{
		for(size_t j = 0; j < ft_buffer.hsize(); j++)
			{
			ft_accumulator[j] += cabs(ft_buffer.at(i, j));
			}
		}
	ft_accumulator_filtered.CopyData(ft_accumulator);
	CutTreshold(ft_accumulator_filtered, 0.05);	
	}

void	FindMaximaPosition(const ComplexFunction2D_F32 &b, float &m1, float &m2)
	{
	size_t r = b.vsize();
	size_t s = b.hsize();
	
	m1 = m2 = 0;

	double	v1(0), v2(0), v3(0);
	double	u1(0), u2(0), u3(0);

	for(size_t i = 0; i < r; i++)
		{
		size_t pos_1 = 0;
		for(size_t j = 0; j < s; j++)
			{
			if(b.at(i, j) > b.at(i,pos_1)) pos_1 = j;
			}

		size_t pos_2 = 0;
		for(size_t j = 0; j < s; j++)
			{
			if(b.at(i, j) > b.at(i,pos_2) && j != pos_1) pos_2 = j;
			}

		size_t pos_3 = 0;
		for(size_t j = 0; j < s; j++)
			{
			if(b.at(i, j) > b.at(i,pos_3) && j != pos_1 && j != pos_2) pos_3 = j;
			}
		
		v1 += cabs(b.at(i,pos_1));
		v2 += cabs(b.at(i,pos_2));
		v3 += cabs(b.at(i,pos_3));
		
		u1 += pos_1;
		u2 += pos_2;
		u3 += pos_3;
		
//		m1 += (pos_1 + pos_2)/2;
		m2 += abs(int(pos_1) - int(pos_2))*cabs(b.at(i,pos_2))/cabs(b.at(i,pos_1));
		m2 += abs(int(pos_1) - int(pos_3))*cabs(b.at(i,pos_3))/cabs(b.at(i,pos_1));
		
		}
	
	double	d2 = v2/v1;
	double	d3 = v3/v1;
	
	m1 = (u1 + u2*2 + u3*d3)/(r*(1.+ d2+ d3));
	m2 /= r;
	}


void	DisplayIntermediateResults(const RealFunctionF32 &ft_accumulator, const RealFunctionF32 &ft_accumulator_filtered, size_t n_rays_average)
	{

	size_t ft_size = ft_accumulator.size();
	static	double	max_power = 0;
	static	size_t count = 0;
	static	RealFunctionF32	accumulator1(ft_size), accumulator2(ft_size);
	static	GraphSet	spectra_dynamic("spectra_dynamic", "frequency", "power");

	if(!count)
		{
		spectra_dynamic.AddGraphUniform(ft_accumulator, 0, 1, "Spectra");
		spectra_dynamic.AddGraphUniform(ft_accumulator_filtered, 0, 1, "Spectra cut");
		}


	
	if(ft_size != accumulator1.size()) FatalError("DisplayIntermediateResults: invalid arguments");
	if(count%n_rays_average)
		{
		accumulator1 += ft_accumulator;
		accumulator2 += ft_accumulator_filtered;
		}
	else if(count)
		{
		for(size_t i = 0; i < ft_size; i++)
			{
			if(accumulator1[i]>1) accumulator1[i] = 20*log10(accumulator1[i]);
			else accumulator1[i] = 0;
			if(accumulator2[i]>1) accumulator2[i] = 20*log10(accumulator2[i]);
			else accumulator2[i] = 0;
			}
		GraphScale	gs;
		spectra_dynamic.GetScale(gs);
		max_power = max(double(MaxValue(accumulator1)), max_power);
		gs.y2() = max_power;
		spectra_dynamic.SetScale(gs);

		spectra_dynamic.ChangeGraphUniform(0, accumulator1, 0, 1, "Spectra");
		accumulator1.fill(0);
		spectra_dynamic.ChangeGraphUniform(1, accumulator2, 0, 1, "Spectra cut");
		accumulator2.fill(0);
		}		
	

	count++;
	ForceUpdateGUI();
	}






XRAD_END
#endif