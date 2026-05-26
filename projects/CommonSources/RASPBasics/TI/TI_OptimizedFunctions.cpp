#include "pre.h"
#include "TI_OptimizedFunctions.h"

namespace	TI_OptimizedFunctions
{

void	iir2_prepare_coefficients(double first_coefficient, uint16_t& f1, uint16_t& f2)
	{
	const int	offset = 16;
	const int	base = (1<<offset);
	const int	piedestal_correction = 1<<4;

	int	ff1 = base*first_coefficient;
	int	ff2 = base - ff1;
	
	ff1 -= piedestal_correction;
	ff2 -= piedestal_correction;
	f1 = ff1;
	f2 = ff2;
	
	swap(f1,f2);
	
	// компенсация округления, которое вводится в _dotprsu2()
	// (введено эмпирически, чтобы не появлялся "пьедестал"
	// и не пропадал один разряд ни на положительных, ни на
	// отрицательных данных
	}




#define	iir2_one_point_4(result, pre_result)\
		result.s[0] = _dotprsu2(_pack2(result.s[0], pre_result.s[0]), coefficients);\
		result.s[1] = _dotprsu2(_pack2(result.s[1], pre_result.s[1]), coefficients);\
		result.s[2] = _dotprsu2(_pack2(result.s[2], pre_result.s[2]), coefficients);\
		result.s[3] = _dotprsu2(_pack2(result.s[3], pre_result.s[3]), coefficients)
	
//--------------------------------------------------------------------------------
//
//	4 columns parallel version
//
//	быстродействие для массива 128 отсчетов
//	5149 / 4 = 1287 cycles
//
//--------------------------------------------------------------------------------


void	iir2_4_columns(int16_t * restrict arr, int size, int st, uint16_t f1, uint16_t f2)
	{
	int	coefficients = _pack2(f1, f2);
	
	
	int16_t	* restrict ae = arr + st*size;
	int16_t	* restrict a0 = arr + st;
	
	buffer64	pre_result, result;
	pre_result.d = _amemd8(arr);

	for(; a0 < ae;)
		{
		result.d = _amemd8(a0);
		iir2_one_point_4(result, pre_result);
		
		_amemd8(a0) = result.d;
		pre_result.d=result.d;
		
		a0+=st;
		}

	for(; a0 > arr;)
		{
		a0 -= st;
		
		result.d = _amemd8(a0);		
		iir2_one_point_4(result, pre_result);
		
		_amemd8(a0) = result.d;
		pre_result.d=result.d;
		}
	}


//--------------------------------------------------------------------------------
//
//	8 columns parallel version
//
//	быстродействие для массива 128 отсчетов
//	5147 / 8 = 643 cycles
//
//--------------------------------------------------------------------------------

void	iir2_8_columns(int16_t * restrict arr, int size, int st, uint16_t f1, uint16_t f2)
	{
	int	coefficients = _pack2(f1, f2);
	
	
	const int16_t	* restrict ae = arr + st*size;
	int16_t	* restrict a0 = arr + st;
	int16_t	* restrict a1 = arr + st + 4;
	
	buffer64	pre_result0, result0;
	buffer64	pre_result1, result1;
	
	pre_result0.d = _amemd8(arr);
	pre_result1.d = _amemd8(arr+4);

	for(; a0 < ae;)
		{
		result0.d = _amemd8(a0);
		result1.d = _amemd8(a1);
		
		iir2_one_point_4(result0, pre_result0);
		iir2_one_point_4(result1, pre_result1);

		
		_amemd8(a0) = result0.d;
		_amemd8(a1) = result1.d;
		
		pre_result0.d = result0.d;
		pre_result1.d = result1.d;
		
		a0+=st;
		a1+=st;
		}

	for(; a0 > arr;)
		{
		a0-=st;
		a1-=st;
		
		result0.d = _amemd8(a0);
		result1.d = _amemd8(a1);
		
		iir2_one_point_4(result0, pre_result0);
		iir2_one_point_4(result1, pre_result1);
		
		_amemd8(a0) = result0.d;
		_amemd8(a1) = result1.d;

		pre_result0.d=result0.d;
		pre_result1.d=result1.d;
		}
	}
//--------------------------------------------------------------------------------
//
//	16 columns parallel version
//	быстродействие для массива 128 отсчетов
//	5156 / 16 = 322 cycles
//
//--------------------------------------------------------------------------------


void	iir2_16_columns(int16_t * restrict arr, int size, int st, uint16_t f1, uint16_t f2)
	{
	int	coefficients = _pack2(f1, f2);
	
	
	const int16_t	* restrict ae = arr + st*size;
	int16_t	* restrict a0 = arr + st;
	int16_t	* restrict a1 = arr + st + 4;
	int16_t	* restrict a2 = arr + st + 8;
	int16_t	* restrict a3 = arr + st + 12;
	
	buffer64	pre_result0, result0;
	buffer64	pre_result1, result1;
	buffer64	pre_result2, result2;
	buffer64	pre_result3, result3;
	
	pre_result0.d = _amemd8(arr);
	pre_result1.d = _amemd8(arr+4);
	pre_result2.d = _amemd8(arr+8);
	pre_result3.d = _amemd8(arr+12);

	for(; a0 < ae;)
		{
		result0.d = _amemd8(a0);
		result1.d = _amemd8(a1);
		result2.d = _amemd8(a2);
		result3.d = _amemd8(a3);
		
		iir2_one_point_4(result0, pre_result0);
		iir2_one_point_4(result1, pre_result1);
		iir2_one_point_4(result2, pre_result2);
		iir2_one_point_4(result3, pre_result3);

		
		_amemd8(a0) = result0.d;
		_amemd8(a1) = result1.d;
		_amemd8(a2) = result2.d;
		_amemd8(a3) = result3.d;
		
		pre_result0.d = result0.d;
		pre_result1.d = result1.d;
		pre_result2.d = result2.d;
		pre_result3.d = result3.d;
		
		a0+=st;
		a1+=st;
		a2+=st;
		a3+=st;
		}

	for(; a0 > arr;)
		{
		a0-=st;
		a1-=st;
		a2-=st;
		a3-=st;
		
		result0.d = _amemd8(a0);
		result1.d = _amemd8(a1);
		result2.d = _amemd8(a2);
		result3.d = _amemd8(a3);
		
		iir2_one_point_4(result0, pre_result0);
		iir2_one_point_4(result1, pre_result1);
		iir2_one_point_4(result2, pre_result2);
		iir2_one_point_4(result3, pre_result3);
		
		_amemd8(a0) = result0.d;
		_amemd8(a1) = result1.d;
		_amemd8(a2) = result2.d;
		_amemd8(a3) = result3.d;

		pre_result0.d=result0.d;
		pre_result1.d=result1.d;
		pre_result2.d=result2.d;
		pre_result3.d=result3.d;
		}
	}



} //namespace	TI_OptimizedFunctions


//--------------------------------------------------------------------------------
//
//	auxiliary functions
//
//--------------------------------------------------------------------------------

namespace
{
inline	void	iir_part4_fwd(buffer128& restrict result, int pre_result, const buffer64& restrict buf, int coefficients)
	{
	result.i[0] = _dotprsu2(_pack2(buf.s[0], pre_result),coefficients);// умножение, суммма, сдвиг (только на 16)
	result.i[1] = _dotprsu2(_pack2(buf.s[1], result.i[0]),coefficients);
	result.i[2] = _dotprsu2(_pack2(buf.s[2], result.i[1]),coefficients);
	result.i[3] = _dotprsu2(_pack2(buf.s[3], result.i[2]),coefficients);
	}

inline	void	iir_part4_back(buffer128& restrict result, int pre_result, const buffer64& restrict buf, int coefficients)
	{
		result.i[0] = _dotprsu2(_pack2(buf.s[3], pre_result),coefficients);
		result.i[1] = _dotprsu2(_pack2(buf.s[2], result.i[0]),coefficients);
		result.i[2] = _dotprsu2(_pack2(buf.s[1], result.i[1]),coefficients);
		result.i[3] = _dotprsu2(_pack2(buf.s[0], result.i[2]),coefficients);
	}
}//namespace

//--------------------------------------------------------------------------------
//
//	auxiliary macros
//
//	следующие ниже действия не удалось превратить в inline функции, т.к.
//	резко падает производительность (пропадает pipeline)
//
//--------------------------------------------------------------------------------


namespace	TI_OptimizedFunctions
{

#define read(buf, a0)\
		buf.d = _amemd8(a0)

#define write_forward(result, a0)\
		_amemd8(a0) = _itod(_pack2(result.i[3], result.i[2]), _pack2(result.i[1], result.i[0]))

#define write_backward(result, a0)\
		_amemd8(a0) = _itod(_pack2(result.i[0], result.i[1]), _pack2(result.i[2], result.i[3]))

#define perform_forward(buf, result)\
		iir_part4_fwd(result, result.i[3], buf, coefficients)

#define perform_backward(buf, result)\
		iir_part4_back(result,result.i[3], buf, coefficients)



//--------------------------------------------------------------------------------
//
//	base version (1 row)
//
//--------------------------------------------------------------------------------

void	iir2_row_1(int16_t * restrict arr, int s, uint16_t f1, uint16_t f2)
	{
	int32_t	coefficients;
	coefficients = _pack2(f1, f2);
		
	int16_t	*a0 = arr;
	
	
	// 8 byte aligned version
	int16_t	*const ae = a0 + s;
	
	
	buffer128	result;
	buffer64	buf;
	result.i[3] = _amem2_const(a0); // это значит, что за границы массива продолжаем крайним значением. то же и ниже
	
	#pragma MUST_ITERATE(4,256)
	for(; a0<ae;)
		{
		read(buf, a0);
		perform_forward(buf, result);				
		write_forward(result, a0);
		a0 += 4;
		}

	// крайнее значение по-прежнему хранится в result.i[3], этим пользуемся
	#pragma MUST_ITERATE(4,256)
	for(; a0>arr;)// неравенство строгое, т.к. счетчик убавляется внутри цикла
		{
		a0 -= 4;
		read(buf, a0);		
		perform_backward(buf, result);
		write_backward(result, a0);
		}	
	}


//--------------------------------------------------------------------------------
//
//	4 rows in parallel version
//
//--------------------------------------------------------------------------------
		

void	iir2_row_4(int16_t * restrict arr, int s, uint16_t f1, uint16_t f2)
	{
	int32_t	coefficients;
	coefficients = _pack2(f1, f2);
		
	int16_t	*a0 = arr;
	int16_t	*a1 = a0 + s;
	int16_t	*a2 = a1 + s;
	int16_t	*a3 = a2 + s;
	
	buffer128	result0, result1, result2, result3;
	
	buffer64	buf0, buf1, buf2, buf3;
	
	

	result0.i[3] = _amem2_const(a0); // это значит, что за границы массива продолжаем крайним значением. то же и ниже
	result1.i[3] = _amem2_const(a1); 
	result2.i[3] = _amem2_const(a2); 
	result3.i[3] = _amem2_const(a3); 
	
	#pragma MUST_ITERATE(4,256)

	for(int i = s; i >0; i-=4)
		{
		read(buf0, a0);
		read(buf1, a1);
		read(buf2, a2);
		read(buf3, a3);
		
		perform_forward(buf0, result0);
		perform_forward(buf1, result1);
		perform_forward(buf2, result2);
		perform_forward(buf3, result3);

		write_forward(result0, a0);
		write_forward(result1, a1);
		write_forward(result2, a2);
		write_forward(result3, a3);
		
		a0 += 4;
		a1 += 4;
		a2 += 4;
		a3 += 4;
		}

	// крайнее значение по-прежнему хранится в result.i[3], этим пользуемся

	#pragma MUST_ITERATE(4,256)
	for(; a0>arr;)// неравенство строгое, т.к. счетчик убавляется внутри цикла
		{
		a0 -= 4;
		a1 -= 4;
		a2 -= 4;
		a3 -= 4;

		read(buf0, a0);
		read(buf1, a1);
		read(buf2, a2);
		read(buf3, a3);
		
		perform_backward(buf0, result0);
		perform_backward(buf1, result1);
		perform_backward(buf2, result2);
		perform_backward(buf3, result3);

		write_backward(result0, a0);
		write_backward(result1, a1);
		write_backward(result2, a2);
		write_backward(result3, a3);
		}	
	}

//--------------------------------------------------------------------------------
//
//	8 rows in parallel version
//
//--------------------------------------------------------------------------------
		

void	iir2_row_8(int16_t * restrict arr, int s, uint16_t f1, uint16_t f2)
	{
	int32_t	coefficients;
	coefficients = _pack2(f1, f2);
		
	int16_t	*a0 = arr;
	int16_t	*a1 = a0 + s;
	int16_t	*a2 = a1 + s;
	int16_t	*a3 = a2 + s;
	int16_t	*a4 = a3 + s;
	int16_t	*a5 = a4 + s;
	int16_t	*a6 = a5 + s;
	int16_t	*a7 = a6 + s;
	
	buffer128	result0, result1, result2, result3;
	buffer128	result4, result5, result6, result7;
	
	buffer64	buf0, buf1, buf2, buf3;
	buffer64	buf4, buf5, buf6, buf7;
	

	result0.i[3] = _amem2_const(a0); // это значит, что за границы массива продолжаем крайним значением. то же и ниже
	result1.i[3] = _amem2_const(a1); 
	result2.i[3] = _amem2_const(a2); 
	result3.i[3] = _amem2_const(a3); 

	result4.i[3] = _amem2_const(a4); 
	result5.i[3] = _amem2_const(a5); 
	result6.i[3] = _amem2_const(a6); 
	result7.i[3] = _amem2_const(a7); 
	
	#pragma MUST_ITERATE(4,256)
//	for(; a0<ae;)
	for(int i = s; i >0; i-=4)
		{
		read(buf0, a0);
		read(buf1, a1);
		read(buf2, a2);
		read(buf3, a3);
		read(buf4, a4);
		read(buf5, a5);
		read(buf6, a6);
		read(buf7, a7);

		perform_forward(buf0, result0);
		perform_forward(buf1, result1);
		perform_forward(buf2, result2);
		perform_forward(buf3, result3);
		perform_forward(buf4, result4);
		perform_forward(buf5, result5);
		perform_forward(buf6, result6);
		perform_forward(buf7, result7);

		write_forward(result0, a0);
		write_forward(result1, a1);
		write_forward(result2, a2);
		write_forward(result3, a3);
		write_forward(result4, a4);
		write_forward(result5, a5);
		write_forward(result6, a6);
		write_forward(result7, a7);
		
		a0 += 4;
		a1 += 4;
		a2 += 4;
		a3 += 4;
		a4 += 4;
		a5 += 4;
		a6 += 4;
		a7 += 4;
		}

	// крайнее значение по-прежнему хранится в result.i[3], этим пользуемся
	
	#pragma MUST_ITERATE(4,256)
	for(; a0>arr;)// неравенство строгое, т.к. счетчик убавляется внутри цикла
		{
		a0 -= 4;
		a1 -= 4;
		a2 -= 4;
		a3 -= 4;
		a4 -= 4;
		a5 -= 4;
		a6 -= 4;
		a7 -= 4;

		read(buf0, a0);
		read(buf1, a1);
		read(buf2, a2);
		read(buf3, a3);
		read(buf4, a4);
		read(buf5, a5);
		read(buf6, a6);
		read(buf7, a7);
		
		perform_backward(buf0, result0);
		perform_backward(buf1, result1);
		perform_backward(buf2, result2);
		perform_backward(buf3, result3);
		perform_backward(buf4, result4);
		perform_backward(buf5, result5);
		perform_backward(buf6, result6);
		perform_backward(buf7, result7);

		write_backward(result0, a0);
		write_backward(result1, a1);
		write_backward(result2, a2);
		write_backward(result3, a3);
		write_backward(result4, a4);
		write_backward(result5, a5);
		write_backward(result6, a6);
		write_backward(result7, a7);
		}	
	}

//--------------------------------------------------------------------------------
//
//	16 rows in parallel version (slower than 8!)
//
//--------------------------------------------------------------------------------
		

void	iir2_row_16(int16_t * restrict arr, int s, uint16_t f1, uint16_t f2)
	{
	int32_t	coefficients;
	coefficients = _pack2(f1, f2);
		
	int16_t	*a0 = arr;
	int16_t	*a1 = a0 + s;
	int16_t	*a2 = a1 + s;
	int16_t	*a3 = a2 + s;
	int16_t	*a4 = a3 + s;
	int16_t	*a5 = a4 + s;
	int16_t	*a6 = a5 + s;
	int16_t	*a7 = a6 + s;
	
	int16_t	*a8 = a7 + s;
	int16_t	*a9 = a8 + s;
	int16_t	*aa = a9 + s;
	int16_t	*ab = aa + s;
	int16_t	*ac = ab + s;
	int16_t	*ad = ac + s;
	int16_t	*ae = ad + s;
	int16_t	*af = ae + s;
	
	buffer128	result0, result1, result2, result3;
	buffer128	result4, result5, result6, result7;
	buffer128	result8, result9, resulta, resultb;
	buffer128	resultc, resultd, resulte, resultf;
	
	buffer64	buf0, buf1, buf2, buf3;
	buffer64	buf4, buf5, buf6, buf7;
	buffer64	buf8, buf9, bufa, bufb;
	buffer64	bufc, bufd, bufe, buff;
	

	result0.i[3] = _amem2_const(a0); // это значит, что за границы массива продолжаем крайним значением. то же и ниже
	result1.i[3] = _amem2_const(a1); 
	result2.i[3] = _amem2_const(a2); 
	result3.i[3] = _amem2_const(a3); 

	result4.i[3] = _amem2_const(a4); 
	result5.i[3] = _amem2_const(a5); 
	result6.i[3] = _amem2_const(a6); 
	result7.i[3] = _amem2_const(a7); 

	result8.i[3] = _amem2_const(a8); 
	result9.i[3] = _amem2_const(a9); 
	resulta.i[3] = _amem2_const(aa); 
	resultb.i[3] = _amem2_const(ab); 

	resultc.i[3] = _amem2_const(ac); 
	resultd.i[3] = _amem2_const(ad); 
	resulte.i[3] = _amem2_const(ae); 
	resultf.i[3] = _amem2_const(af); 
	
	#pragma MUST_ITERATE(4,256)
//	for(; a0<ae;)
	for(int i = s; i >0; i-=4)
		{
		read(buf0, a0);
		read(buf1, a1);
		read(buf2, a2);
		read(buf3, a3);
		read(buf4, a4);
		read(buf5, a5);
		read(buf6, a6);
		read(buf7, a7);
		read(buf8, a8);
		read(buf9, a9);
		read(bufa, aa);
		read(bufb, ab);
		read(bufc, ac);
		read(bufd, ad);
		read(bufe, ae);
		read(buff, af);

		perform_forward(buf0, result0);
		perform_forward(buf1, result1);
		perform_forward(buf2, result2);
		perform_forward(buf3, result3);
		perform_forward(buf4, result4);
		perform_forward(buf5, result5);
		perform_forward(buf6, result6);
		perform_forward(buf7, result7);
		perform_forward(buf8, result8);
		perform_forward(buf9, result9);
		perform_forward(bufa, resulta);
		perform_forward(bufb, resultb);
		perform_forward(bufc, resultc);
		perform_forward(bufd, resultd);
		perform_forward(bufe, resulte);
		perform_forward(buff, resultf);

		write_forward(result0, a0);
		write_forward(result1, a1);
		write_forward(result2, a2);
		write_forward(result3, a3);
		write_forward(result4, a4);
		write_forward(result5, a5);
		write_forward(result6, a6);
		write_forward(result7, a7);
		write_forward(result8, a8);
		write_forward(result9, a9);
		write_forward(resulta, aa);
		write_forward(resultb, ab);
		write_forward(resultc, ac);
		write_forward(resultd, ad);
		write_forward(resulte, ae);
		write_forward(resultf, af);
		
		a0 += 4;
		a1 += 4;
		a2 += 4;
		a3 += 4;
		a4 += 4;
		a5 += 4;
		a6 += 4;
		a7 += 4;
		a8 += 4;
		a9 += 4;
		aa += 4;
		ab += 4;
		ac += 4;
		ad += 4;
		ae += 4;
		af += 4;
		}

	// крайнее значение по-прежнему хранится в result.i[3], этим пользуемся
	
	#pragma MUST_ITERATE(4,256)
	for(; a0>arr;)// неравенство строгое, т.к. счетчик убавляется внутри цикла
		{
		a0 -= 4;
		a1 -= 4;
		a2 -= 4;
		a3 -= 4;
		a4 -= 4;
		a5 -= 4;
		a6 -= 4;
		a7 -= 4;
		a8 -= 4;
		a9 -= 4;
		aa -= 4;
		ab -= 4;
		ac -= 4;
		ad -= 4;
		ae -= 4;
		af -= 4;

		read(buf0, a0);
		read(buf1, a1);
		read(buf2, a2);
		read(buf3, a3);
		read(buf4, a4);
		read(buf5, a5);
		read(buf6, a6);
		read(buf7, a7);
		read(buf8, a8);
		read(buf9, a9);
		read(bufa, aa);
		read(bufb, ab);
		read(bufc, ac);
		read(bufd, ad);
		read(bufe, ae);
		read(buff, af);
		
		perform_backward(buf0, result0);
		perform_backward(buf1, result1);
		perform_backward(buf2, result2);
		perform_backward(buf3, result3);
		perform_backward(buf4, result4);
		perform_backward(buf5, result5);
		perform_backward(buf6, result6);
		perform_backward(buf7, result7);
		perform_backward(buf8, result8);
		perform_backward(buf9, result9);
		perform_backward(bufa, resulta);
		perform_backward(bufb, resultb);
		perform_backward(bufc, resultc);
		perform_backward(bufd, resultd);
		perform_backward(bufe, resulte);
		perform_backward(buff, resultf);

		write_backward(result0, a0);
		write_backward(result1, a1);
		write_backward(result2, a2);
		write_backward(result3, a3);
		write_backward(result4, a4);
		write_backward(result5, a5);
		write_backward(result6, a6);
		write_backward(result7, a7);

		write_backward(result8, a8);
		write_backward(result9, a9);
		write_backward(resulta, aa);
		write_backward(resultb, ab);
		write_backward(resultc, ac);
		write_backward(resultd, ad);
		write_backward(resulte, ae);
		write_backward(resultf, af);
		}	
	}

//--------------------------------------------------------------------------------
//
//	non-aligned version
//
//--------------------------------------------------------------------------------


void	iir2_row_non_aligned(int16_t * restrict arr, int s, uint16_t f1, uint16_t f2)
	{
	int	coefficients = _pack2(f1, f2);
	int16_t	*a0 = arr;
	
	
	//	non-aligned version
	const int16_t	*ae = arr+s-1;
	for(; a0 < ae;)
		{
		int buf = _dotprsu2(_mem4(a0), coefficients);
		*(++a0) = buf;
		//*(++a0)= _dotprsu2(_mem4(a0), coefficients.i);	плохо! оптимизатор сбивается, скорость подлетает вчетверо, но работает неправильно
		}
	
	//swap(coefficients.s[0], coefficients.s[1]);
	coefficients = _pack2(f2, f1);
	for(; a0 > arr;)
		{
		*a0 = _dotprsu2(_mem4(--a0),coefficients);
		}
	}
/*
другие варианты невыровненного вычисления:
вперед:
//		ver.1
		double	buf = _mpy2(_mem4(a0), coefficients.i);
		*(++a0) = (_hi(buf) + _lo(buf))>>offset;

//		ver.2
		int buf = _dotp2(_mem4(a0), coefficients.i);// + baseline_correction;
		*(++a0) = buf >> offset;
назад:
//		ver.1
		double buf = _mpy2(_mem4(--a0), coefficients.i);
		*a0 = (_hi(buf) + _lo(buf))>>offset;

//		ver.2
		*a0 = _dotp2(_mem4(--a0),coefficients.i)>>offset;//offset не более 15
 
 */


} //namespace	TI_OptimizedFunctions
