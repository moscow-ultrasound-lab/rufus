#include "pre.h"
#include "SignalProcessing/GaussSignalTools.h"

/*
template<class numType> void MathFunction<numType> :: WeightMoments(float *weight_centre, float *weight_deviation) const{
	double	sd = 0, r = 0, m = 0;
	fast_const_iterator	it = begin();

	for(int i = 0; i < size(); ++it, ++i)
		{
		r += fabs(*it)*i;
		m += fabs(*it);
		}
	if(m) *weight_centre = (r /= m);
	else *weight_centre = size()/2;

	if(weight_deviation)
		{
		it = begin();
		for(int i = 0; i < size(); ++it, i++) sd += square(i-r) * fabs(*it);
		if(m) *weight_deviation = sqrt(sd/m);
		else *weight_deviation = size()/2;
		}
	}
*/

/*
double weight_centre(const ComplexFunctionF32 &f)
	{
	int	s = f.size();
	int	i;
	double	r = 0, m = 0;
	
	for(i = 0; i < s; i++)
		{
		r += fabs(f[i])*double(i);
		m += fabs(f[i]);
		}
	
	return r/m;
	}

double	weight_deviation(const ComplexFunctionF32 &f)
	{
	int	s = f.size();
	int	i;
	double	sd = 0, r = 0, m = 0;
	
	for(i = 0; i < s; i++)
		{
		r += fabs(f[i])*double(i);
		m += fabs(f[i]);
		}	
	r /= m;
	
	for(i = 0; i < s; i++) sd += square(i-r) * fabs(f[i]);
	
	return sqrt(sd/m);
	}
*/

XRAD_BEGIN

template<class T>
struct	limit_value
	{
	const	double	bound;
	const absolute_value_functor<T, T> abs_f;
	
	limit_value() : bound(20./pi()){}
//	template<class T>
	T operator()(const T &value) const
		{
		return	T(bound*atan(abs_f(value)/bound));
// 		if(value > 20) return 20*value/cabs(value);
// 		else return value;
		}
	};

bool	ATL_data = false;

void	NormalizeSpectrum(ComplexFunction2D_F32 &procBuffer, complexSignalCoord coord,
		double shift,
		double broadFactor)
	{
	size_t	i,j;
	
	ft_flags	forward_fourier = fftFwd;
	ft_flags	reverse_fourier = fftRev;
	
	if(ATL_data)
		{
		// здесь можно перевернуть порЯдок прЯмого/обратного преобразованиЯ ”урье
		// в угоду стандарту, обусловленному фазой интерполЯторов атл.
		// сейчас ничего не стоило бы перевернуть фазу в наших интерполЯторах и т. д.
		// но много уже файлов насчитано. чтобы все не переделывать, принимаем их стандарт
		forward_fourier = fftRev;
		reverse_fourier = fftFwd;
		}

	if(coord == spatial)
		{
		procBuffer.transpose();
		FFTf(procBuffer, forward_fourier | fftRollAfter, fftNone);//2016_09_16
		}
	else
		{
		FFTf(procBuffer, forward_fourier, fftNone);//2016_09_16
		}

	size_t	vs = procBuffer.vsize();
	size_t	hs = procBuffer.hsize();
	
	
	
	RealFunctionF32	avSpectrum(hs, 0);
	RealFunctionF32	gFilter(hs, 0);
	ComplexFunctionF32	initFilter(hs);

	initFilter.fill(complexF32(1));
	ApplyWindowFunction(initFilter, cos2_window());
	for(i = 0; i < vs; i++) procBuffer.row(i) *= initFilter;
	
	
	for(i = 0; i < vs; i++)
		{
		for(j = 0; j < hs; j++)
			{
			avSpectrum[j] += cabs(procBuffer.at(i,j));
			}
		}
	double max = MaxValue(avSpectrum);
	avSpectrum /= max;

	RealVectorF32	m(2);
	float	omega0_s;// _s -- in samples
	float	bandWidth_s;
	
	WeightMoments(avSpectrum, m, true);
	omega0_s = m[0];
	bandWidth_s = sqrt(m[1]);
	
	
if(0)	if(coord == temporal)
		{
		//#pragma message ("Here forced params")
		// никакой автоматики, стандартные показатели данных атл
		omega0_s = (2.8/6.)*hs;
		bandWidth_s = (0.7/6.)*hs;
		}
	
//	double	new_omega0_s = omega0_s + shift*bandWidth_s;
//	double	broad_tune = float(hs/2-shift*bandWidth_s)/float(hs/2);
		// попытка сузить полосу по мере увеличениЯ сдвига частот
//	double	broad_tune = 1./(1.+fabs(shift)/4);
	double	broad_tune = 1.;
	double	new_omega0_s = omega0_s + shift*bandWidth_s;
	double	new_bandwidth_s = bandWidth_s * broadFactor * broad_tune;
	
	for(i = 0; i < hs; i++) gFilter[i] = gauss(i-new_omega0_s, new_bandwidth_s);
	ApplyWindowFunction(gFilter, cos2_window());

	if(CapsLock())
		{
		ShowString("Shift params", ssprintf("Shift = %g, Broad tune = %g", shift, broad_tune));
		printf("\nomega0_s = %g, std. deviation_s = %g", omega0_s, bandWidth_s);
		fflush(stdout);
		DisplayMathFunction(avSpectrum, 0, 1, "Average spectrum");
		DisplayMathFunction(gFilter, 0, 1, "Gauss approximation");
		}
	

	for(i = 0; i < hs; i++)
		{
		double d = fabs(avSpectrum[i]);
		if(d) gFilter[i] /= d;
		else gFilter[i] = 0;
		}

	ApplyFunction(gFilter, limit_value<float>());

	avSpectrum *= gFilter;
	for(i = 0; i < vs; i++) procBuffer.row(i) *= gFilter;

	if(CapsLock())
		{
		DisplayMathFunction(gFilter, 0, 1, "Gauss correction");
		DisplayMathFunction(avSpectrum, 0, 1, "Corrected spectrum");
		}

	if(coord == spatial){// см. комментарий в начале процедуры
		FFTf(procBuffer, reverse_fourier | fftRollBefore, fftNone);//2016_09_16
		procBuffer.transpose();
		}
	else FFTf(procBuffer, reverse_fourier, fftNone);//2016_09_16
	}
	
XRAD_END