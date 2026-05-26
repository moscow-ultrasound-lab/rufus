#ifndef __ti_optimized_functions
#define __ti_optimized_functions

#include <stdint.h>
#include <assert.h>

#ifndef __TI_COMPILER_VERSION__
#error "This file can be used only for TI compiler"
#endif

#ifndef __REDEFINE_FUNCTION_NAMES

#define	TI_OptimizedFunctions __n00
#define	iir2_prepare_coefficients __f00
#define iir2_4_columns	__f01
#define iir2_8_columns	__f02
#define iir2_16_columns	__f03

#define iir2_row_non_aligned __f04
#define	iir2_row_1	__f05
#define	iir2_row_2	__f06
#define	iir2_row_4	__f07
#define	iir2_row_8	__f08
#define	iir2_row_16	__f09

#endif //__REDEFINE_FUNCTION_NAMES


namespace	TI_OptimizedFunctions
{

//	data access auxiliaries
union	buffer128
	{
	int32_t	i[4];
	int16_t	s[8];
	uint8_t	c[16];
	int64_t	l[2];
	double d[2];
	};


union buffer64
	{
	int32_t	i[2];
	int16_t	s[4];
	uint8_t	c[8];
	int64_t l;
	double d;
	};

union	buffer32
	{
	int32_t	i;
	uint16_t	s[2];
	uint8_t		c[4];
	};


//----------------------------------------------------------------------
//
//	фильтрация по столбцам массива (параллельные версии по 4, 8, 16 столбцов)
//
void	iir2_prepare_coefficients(double first_coefficient, uint16_t& f1, uint16_t& f2);

void	iir2_4_columns(int16_t * restrict arr, int size, int st, uint16_t f1, uint16_t f2);
void	iir2_8_columns(int16_t * restrict arr, int size, int st, uint16_t f1, uint16_t f2);
void	iir2_16_columns(int16_t * restrict arr, int size, int st, uint16_t f1, uint16_t f2);





//----------------------------------------------------------------------
//
//	фильтрация строки массива (в том числе параллельные версие по 2, 4, 8)
//	производительность посчитана при обработке массива в 512 отсчетов

void	iir2_row_non_aligned(int16_t * restrict arr, int s, uint16_t f1, uint16_t f2);
	//
	
void	iir2_row_1(int16_t * restrict arr, int s, uint16_t f1, uint16_t f2);
	//	5300 cycles
void	iir2_row_4(int16_t * restrict arr, int s, uint16_t f1, uint16_t f2);
	//	8371/4 = 2092
void	iir2_row_8(int16_t * restrict arr, int s, uint16_t f1, uint16_t f2);
	//	10092/8 = 1262
void	iir2_row_16(int16_t * restrict arr, int s, uint16_t f1, uint16_t f2);
	//	производительность резко падает (46026/16 = 2877)

}//namespace	TI_OptimizedFunctions



using namespace TI_OptimizedFunctions;



#endif  //__ti_optimized_functions
