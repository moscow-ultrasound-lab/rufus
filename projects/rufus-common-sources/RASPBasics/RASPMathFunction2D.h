#ifndef __rasp_math_function_2d_h
#define __rasp_math_function_2d_h

#include <MathFunction2D.h>
#include <RASPFixedPoint.h>
//----------------------------------------------------------------------
//
//	тип целочисленного двумерного массива.
//	умножение на целое -- со сдвигом.
//	при умножение на double множитель преобразуется к целому, при этом сдвиг
//	специально оптимизируется во избежание потерь точности
//
//----------------------------------------------------------------------



XRAD_BEGIN

template<class T, class ST>
class	RASPArray2D : public MathFunction2D <MathFunction<T,ST> >
	{
	public:
		typedef	MathFunction<T,ST> function_1D;
		PARENT(MathFunction2D <function_1D>);
		typedef	RASPArray2D<T,ST>	self;
		typedef	T	value_type;
		typedef	ST	scalar_type;

	public:
		RASPArray2D(){}
		RASPArray2D(int vs, int hs) : MathFunction2D<fixed_point_array>(vs, hs) {}

		inline void	CopyData(const self& original);
	};

template<class T, class ST>
class	FixedPointArray2D : public RASPArray2D<T,ST>
	{
		typedef	 RASPArray2D<T,ST>__p;
	public:
		PARENT(__p);
		typedef	MathFunction<T,ST> fixed_point_array;
		typedef	FixedPointArray2D<T,ST>	self;
		typedef	T	value_type;
		typedef ST	scalar_type;

	public:

		FixedPointArray2D(){}
		FixedPointArray2D(int vs, int hs) : RASPArray2D<fixed_point_array>(vs, hs) {}

		// быстрое умножение массива целых чисел на нецелое
		// требует специальной тщательной проработки,
		// потому оно выписывается особо

		FixedPointArray2D &operator *= (double x);
		FixedPointArray2D &operator *= (T x);

	};





typedef	RASPArray2D<float, double> float_data;
typedef	RASPArray2D<double, long double> double_data;

//-----------------------------------------------------------------------------
//
//	sic! неточность, приведшая к катастрофическим последствиям:
//	вместо fixed_point_array2D<short> написал
//	MathFunction2D<MathFunction<short> >
//	вызывался не тот алгоритм умножения на double, вся обработка
//	рассыпалась
//
typedef	FixedPointArray2D<int,long> integer_data;
typedef	FixedPointArray2D<unsigned short, int> unsigned_short_data;

#ifndef __TI_COMPILER_VERSION__
	// сделана оптимизированная версия под тмс320, которая
	// может обрабатывать 8-битные входные данные
	// в 16-битном контейнере

	typedef	FixedPointArray2D<short,int> signed_short_data;
	typedef	FixedPointArray2D<uchar,int> uchar_data;
#endif //__TI_COMPILER_VERSION__



XRAD_END


#ifdef __TI_COMPILER_VERSION__
#include "TI_RaspMathFunction2D.h"
#endif

#include "RASPMathFunction2D.cc"

#endif //__rasp_math_function_2d_h
