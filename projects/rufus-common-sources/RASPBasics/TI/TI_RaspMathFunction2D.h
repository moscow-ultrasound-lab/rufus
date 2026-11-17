#ifndef __ti_math_function_2d_h
#define __ti_math_function_2d_h

#ifndef __TI_COMPILER_VERSION__
#error "This file can be used only with TI compiler"
#endif //__TI_COMPILER_VERSION__


#include <stdint.h>
#include <RASPFixedPoint.h>
#include "RaspTableFunction.h"
#include <ImportExportSample.h>
#include "TI_OptimizedFunctions.h"



XRAD_BEGIN

class	signed_short_data : public MathFunction2D <MathFunction<int16_t> >
	{
	public:
		PARENT(MathFunction2D <MathFunction<int16_t> >);

		typedef	MathFunction<int16_t> fixed_point_array;
		typedef	signed_short_data	self;
		typedef	int16_t	value_type;

	public:

		signed_short_data(){}
		signed_short_data(int vs, int hs) : MathFunction2D<fixed_point_array>(vs, hs) {}

		// быстрое умножение массива целых чисел на нецелое
		// требует специальной тщательной проработки,
		// потому оно выписывается особо

		inline signed_short_data &operator *= (double x);

	};

inline int16_t	MaxValue(const signed_short_data & restrict data, int* v=NULL, int* h=NULL);


class	uchar_data : public MathFunction2D <MathFunction<uint8_t> >
	{
	public:
		PARENT(MathFunction2D <MathFunction<uint8_t> >);

		typedef	MathFunction<uint8_t> fixed_point_array;
		typedef	uchar_data	self;
		typedef	uint8_t	value_type;

	public:

		uchar_data(){}
		uchar_data(int vs, int hs) : MathFunction2D<fixed_point_array>(vs, hs) {}
	};


XRAD_END

#include <TI_RaspMathFunction2D.cc>

#endif //__ti_math_function_2d_h

