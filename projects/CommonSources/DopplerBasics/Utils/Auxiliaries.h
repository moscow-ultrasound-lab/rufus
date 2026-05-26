#ifndef __Auxiliaries_h
#define __Auxiliaries_h

//------------------------------------------------------------------
//
//	created:	2014/06/09
//	created:	9.6.2014   13:10
//	filename: 	Q:\programs\ElastographyTest\sources\ElastoAuxiliaries.h
//	file path:	Q:\programs\ElastographyTest\sources
//	author:		kns
//	
//	purpose:	
//
//------------------------------------------------------------------

#include <XRADBasic/ContainersAlgebra.h>
#include <XRADBasic/Sources/Utils/LeastSquares.h>
#include <omp.h>

// ради циклов omp, которые запрещают счетчик unsigned
#pragma warning (disable:4018)


XRAD_BEGIN


inline void	NormalizeFrameRange(RealFunction2D_F32 &frame)
{
	double	mv = MaxValue(frame);
	double	factor = 1./mv;
	if(!isnormal(factor)) return;

	frame *= factor;

// 	#pragma omp parallel for schedule (guided)
// 	for(ptrdiff_t i = 0; i < frame.vsize(); ++i)
// 	{
// 		frame.row(i) *= factor;
// 	}
}



// делает то же самое, что сопряженное умножение, но при этом
// модули комплексных чисел не перемножаются.
// этим мы избегаем чрезмерного роста динамического диапазона,
// который происходит при обычном сопряженном умножении
struct low_contrast_conjugate
	{
	template<class T, class ST>
	inline ComplexSample<T,ST> &operator()(ComplexSample<T,ST> &x, const ComplexSample<T,ST> &y) const
		{
		double	a = fabs(x.re) + fabs(x.im);
		return a ? (x%=y)/=a : x=ComplexSample<T,ST>(0);
		}
	};

XRAD_END


#endif //__Auxiliaries_h