

XRAD_BEGIN

//--------------------------------------------------
//
// массив int16 = скаляр того же типа
//
inline void	AS_1D_OpPtr(int16_t* restrict it, int16_t x, int size, int st, const assign<int16_t,int16_t>& op)
	{
	if(st != 1)
		{
		AS_1D_OpPtr<int16_t, int16_t, assign<int16_t,int16_t> >(it, x, size, st, op);
		return;
		}
	
	int16_t	*ie = it+size;	
	buffer64	b1;
	
	b1.i[0] = b1.i[1] = _pack2(x,x);
	
	for(; it<ie; it+=4)
		{
		_amemd8(it) = b1.d;
		}
	}



//--------------------------------------------------
//
// массив int16 += скаляр того же типа
//
inline void	AS_1D_OpPtr(int16_t* restrict it, int16_t x, int size, int st, const plus_assign<int16_t,int16_t>& op)
	{
	if(st != 1)
		{
		AS_1D_OpPtr<int16_t, int16_t, plus_assign<int16_t,int16_t> >(it, x, size, st, op);
		return;
		}
	int		xx = _pack2(x,x);
	int16_t	*ie = it+size;
	buffer64	b1, b2;

	for(; it<ie; it+=4)
		{
		b2.d = _amemd8(it);

		b1.i[0] = _sadd2(b2.i[0], xx);
		b1.i[1] = _sadd2(b2.i[1], xx);

		_amemd8(it) = b1.d;
		}
	}

//--------------------------------------------------
//
// массив int16 -= скаляр того же типа
//
inline void	AS_1D_OpPtr(int16_t* restrict it, int16_t x, int size, int st, const minus_assign<int16_t,int16_t>& op)
	{
	if(st != 1)
		{
		AS_1D_OpPtr<int16_t, int16_t, minus_assign<int16_t,int16_t> >(it, x, size, st, op);
		return;
		}
	int		xx = _pack2(x,x);
	int16_t	*ie = it+size;
	buffer64	b1, b2;

	for(; it<ie; it+=4)
		{
		b2.d = _amemd8(it);

		b1.i[0] = _sub2(b2.i[0], xx);
		b1.i[1] = _sub2(b2.i[1], xx);

		_amemd8(it) = b1.d;
		}
	}


//--------------------------------------------------
//
// массив int16 = массив того же типа (CopyData)
//
inline void	AA_1D_OpPtr(int16_t* restrict it1, const int16_t* restrict it2, int size, int st1, int st2, const assign<int16_t,int16_t>& op)
	{
	if(st1 != 1 || st2 != 1)
		{
		AA_1D_OpPtr<int16_t, int16_t, assign<int16_t,int16_t> >(it1, it2, size, st1, st2, op);
		return;
		}
	int16_t	*ie = it1+size;
	for(; it1<ie; it1+=4,it2+=4)
		{
		_amemd8(it1) = _amemd8_const(it2);
		}
	}



//--------------------------------------------------
//
// массив int16 += массив того же типа
//
inline void	AA_1D_OpPtr(int16_t* restrict it1, const int16_t* restrict it2, int size, int st1, int st2, const plus_assign<int16_t,int16_t>& op)
	{
	int16_t	*ie = it1+size;
	buffer64	b1, b2, b3;

	if(st1 != 1 || st2 != 1)
		{
		AA_1D_OpPtr<int16_t, int16_t, plus_assign<int16_t,int16_t> >(it1, it2, size, st1, st2, op);
		return;
		}
	for(; it1<ie; it1+=4,it2+=4)
		{
		b2.d = _amemd8(it1);
		b3.d = _amemd8_const(it2);

		b1.i[0] = _sadd2(b2.i[0], b3.i[0]);
		b1.i[1] = _sadd2(b2.i[1], b3.i[1]);

		_amemd8(it1) = b1.d;
		}
	}

//--------------------------------------------------
//
// массив int16 -= массив того же типа
//
inline void	AA_1D_OpPtr(int16_t* restrict it1, const int16_t* restrict it2, int size, int st1, int st2, const minus_assign<int16_t,int16_t>& op)
	{
	int16_t	*ie = it1+size;
	buffer64	b1, b2, b3;

	if(st1 != 1 || st2 != 1)
		{
		AA_1D_OpPtr<int16_t, int16_t, minus_assign<int16_t,int16_t> >(it1, it2, size, st1, st2, op);
		return;
		}

	for(; it1<ie; it1+=4,it2+=4)
		{
		b2.d = _amemd8(it1);
		b3.d = _amemd8_const(it2);

		b1.i[0] = _sub2(b2.i[0], b3.i[0]);
		b1.i[1] = _sub2(b2.i[1], b3.i[1]);

		_amemd8(it1) = b1.d;
		}
	}

//--------------------------------------------------
//
// int16 arr0[] = arr1[] + arr2[]
//
inline void	AAA_1D_OpPtr(int16_t* restrict it0, const int16_t* restrict it1, const int16_t* restrict it2, int size, int st0, int st1, int st2, const plus_ternary<int16_t,int16_t,int16_t>& op)
	{
	const int16_t	*ie = it1+size;
	buffer64	b1, b2, b3;

	if(st1 != 1 || st2 != 1 || st0 != 1)
		{
		AAA_1D_OpPtr<int16_t, int16_t, int16_t, plus_ternary<int16_t,int16_t,int16_t> >(it0, it1, it2, size, st0, st1, st2, op);
		return;
		}
	for(; it1<ie; it0+=4,it1+=4,it2+=4)
		{
		b2.d = _amemd8_const(it1);
		b3.d = _amemd8_const(it2);

		b1.i[0] = _sadd2(b2.i[0], b3.i[0]);
		b1.i[1] = _sadd2(b2.i[1], b3.i[1]);

		_amemd8(it0) = b1.d;
		}
	}

//--------------------------------------------------
//
// int16 arr0[] = arr1[] - arr2[]
//
inline void	AAA_1D_OpPtr(int16_t* restrict it0, const int16_t* restrict it1, const int16_t* restrict it2, int size, int st0, int st1, int st2, const minus_ternary<int16_t,int16_t,int16_t>& op)
	{
	const int16_t	*ie = it1+size;
	buffer64	b1, b2, b3;

	if(st1 != 1 || st2 != 1 || st0 != 1)
		{
		AAA_1D_OpPtr<int16_t, int16_t, int16_t, minus_ternary<int16_t,int16_t,int16_t> >(it0, it1, it2, size, st0, st1, st2, op);
		return;
		}
	for(; it1<ie; it0+=4,it1+=4,it2+=4)
		{
		b2.d = _amemd8_const(it1);
		b3.d = _amemd8_const(it2);

		b1.i[0] = _sub2(b2.i[0], b3.i[0]);
		b1.i[1] = _sub2(b2.i[1], b3.i[1]);

		_amemd8(it0) = b1.d;
		}
	}



//--------------------------------------------------
//
// массив int16 *= скаляр того же типа
//
inline void AS_1D_OpPtr(int16_t* restrict it, int16_t x, int size, int st, const RASPFunctors::shift_multiplier<int16_t> &multiplier)
	{
	
	if(st!=1)
		{
		AS_1D_OpPtr<int16_t, int16_t,  RASPFunctors::shift_multiplier<int16_t> >(it, x, size, st, multiplier);
		return;
		}
	
	const int	sh = multiplier.correction_shift;
	
	int		xx = _pack2(x,x);
	int16_t	*ie = it + size;
	buffer64	b1, b2;
	buffer64	p1, p2;
	buffer64	c1, c2;
	
	for(; it<ie; it+=4)
		{
		b2.d = _amemd8(it);

		p1.d = _mpy2(b2.i[0], xx);
		p2.d = _mpy2(b2.i[1], xx);

		c1.i[0] = _sshvr(p1.i[0], sh);
		c1.i[1] = _sshvr(p1.i[1], sh);
		c2.i[0] = _sshvr(p2.i[0], sh);
		c2.i[1] = _sshvr(p2.i[1], sh);
		
//		b1.i[0] = _shr2(_pack2(p1.i[1], p1.i[0]), sh);
//		b1.i[1] = _shr2(_pack2(p2.i[1], p2.i[0]), sh);
		b1.i[0] = _pack2(c1.i[1], c1.i[0]);
		b1.i[1] = _pack2(c2.i[1], c2.i[0]);
		
		_amemd8(it) = b1.d;
		}
	}


#if 1
//	вот проблемная функция
//	замечание, видимо, устарело. проверить все еще раз внимательно и убрать

//--------------------------------------------------
//
// массив int16 *= массив того же типа
//
inline void	AA_1D_OpPtr(int16_t* restrict it1, const int16_t* restrict it2, int size, int st1, int st2, const RASPFunctors::shift_multiplier<int16_t> &multiplier)
//signed_short_data &AA_2D_OpEq__(signed_short_data& restrict y, const signed_short_data& restrict x, RASPFunctors::shift_multiplier<int16_t> const &multiplier)
	{
	if(st1!=1 || st2!=1)
		{
		AA_1D_OpPtr<int16_t, int16_t, RASPFunctors::shift_multiplier<int16_t> >(it1, it2, size, st1, st2, multiplier);
		return;
		}
	
	const int	sh = multiplier.correction_shift;
	int16_t	*ie = it1+size;
	buffer64	b1, b2, b3;

	buffer64	p1, p2;
	buffer64	c1, c2;
	
	for(; it1<ie; it1+=4, it2+=4)
		{
		b2.d = _amemd8(it1);
		b3.d = _amemd8_const(it2);

		p1.d = _mpy2(b2.i[0], b3.i[0]);
		p2.d = _mpy2(b2.i[1], b3.i[1]);
				
		// 	в результате умножения происходит следующее:
		//	p1.i[0] = b2.s[0] * b3.s[0];
		//	p1.i[1] = b2.s[1] * b3.s[1];
		//	p2.i[0] = b2.s[2] * b3.s[2];
		//	p2.i[1] = b2.s[3] * b3.s[3];

		c1.i[0] = _sshvr(p1.i[0], sh);
		c1.i[1] = _sshvr(p1.i[1], sh);
		c2.i[0] = _sshvr(p2.i[0], sh);
		c2.i[1] = _sshvr(p2.i[1], sh);

		//	правильно работающий порядок
		b1.i[0] = _pack2(c1.i[1], c1.i[0]);
		b1.i[1] = _pack2(c2.i[1], c2.i[0]);

//		до этого сдвиг делался таким вот образом,
//		но происходила потеря разрядов
//		b1.i[0] = _shr2(_pack2(p1.i[1], p1.i[0]), sh);
//		b1.i[1] = _shr2(_pack2(p2.i[1], p2.i[0]), sh);

		//	однако хотелось написать по-другому (ниже),
		//	но оно не работает. хорошо бы понять почему
		//	(нужно, чтобы навести в голове порядок с endian)
		//	b1.i[0] = _shr2(_pack2(p1.i[0], p1.i[1]), sh);
		//	b1.i[1] = _shr2(_pack2(p2.i[0], p2.i[1]), sh);
		
		_amemd8(it1) = b1.d;
		}
	}
#endif//1


//--------------------------------------------------
//
//	применение функции abs к массиву
//	можно ли применять здесь restrict?
//	проблема в том, что чаще всего вызывается функция от it==it2
//
//	табличную функцию сюда, к сожалению, не засунешь,
//	т.к. она зависит от DataArray
//
inline void	AA_1D_OpPtr(int16_t* restrict it1, const int16_t* restrict it2, int size, int st1, int st2, const absolute_value_functor &op)
	{
	if(st1 != 1 || st2 != 1)
		{
		AA_1D_OpPtr<int16_t, int16_t, absolute_value_functor>(it1, it2, size, st1, st2, op);
		return;
		}
	int16_t	*ie = it1 + size;

	buffer64	b, c;
	for(;it1<ie; it1+=4, it2+=4)
		{
		b.d = _amemd8_const(it2);
		c.i[0] = _abs2(b.i[0]);
		c.i[1] = _abs2(b.i[1]);
		_amemd8(it1) = c.d;
		}
	}



//--------------------------------------------------------------------
//
//	импорт входного массива uchar во внутренний массив signed int16_t
//

inline void	AA_1D_OpPtr(int16_t* restrict a1, const uint8_t* restrict a2, int size, int st1, int st2, const RASPFunctors::import_sample_functor<int16_t, uchar> &)
	{
	int	sh = conversion_offset(uint16_t(0), uchar(0));
	int16_t			*destination_it = a1;
	
	int	s8 = size - size%8;
	const uint8_t	*source_it = a2,
					*source_end = a2 + s8;

	//	вместо умножения сдвиг, шаг по 8 входных отсчетов
	//	занимает 24к циклов на 65к массив. то же с шагом
	//	в 4 входных отсчета занимало 32к циклов
	buffer64	b1, c0, c1;
	
	// сначала параллельная обработка по 8 отсчетов (пока возможно)
	for(; source_it < source_end; source_it += 8)
		{
		// измерения показали, что если данные выровнены, то
		// скорость не меняется. поэтому ставлю более безопасную
		// команду. то же и в конце экспорта
		b1.d = _memd8_const(source_it);

		c0.i[0] = _sshl(_unpklu4(b1.i[0]), sh);
		c0.i[1] = _sshl(_unpkhu4(b1.i[0]), sh);
		c1.i[0] = _sshl(_unpklu4(b1.i[1]), sh);
		c1.i[1] = _sshl(_unpkhu4(b1.i[1]), sh);
			// сдвигаем влево по целому слову, пользуясь тем, что
			// четные байты заведомо нулевые
			
			// использовать для этого _shr2 не удается, так как
			// эта команда действительно сдвигает только вправо,
			// отрицательное значение сдвига не воспринимается
				
		// здесь выровненные команды, т.к. внутренний буфер идет всегда с выравниванием
		_amemd8(destination_it) = c0.d;
		destination_it+=4;
		_amemd8(destination_it) = c1.d;
		destination_it+=4;
		}
	
	// остаток массива обрабатывается по одному отсчету
	source_end = a2 + size;
	for(; source_it < source_end; ++source_it, ++destination_it)
		{
		*destination_it = *source_it<<sh;
		}
	}

//--------------------------------------------------------------------
//
//	экспорт внутреннего массива int16_t во внешний массив uint16_t
//


inline void	AA_1D_OpPtr(uint8_t* restrict a1, const int16_t* restrict a2, int size, int st1, int st2, const RASPFunctors::export_sample_functor<uchar,int16_t> &)
	{
	int	sh = conversion_offset(uint16_t(0), uchar(0));

	uint8_t	*destination_it = a1;
	
	int	s8 = size - size%8;
	const int16_t	*source_it = a2,
					*source_end = a2 + s8;
	
	register	buffer64	b1;
	register	buffer64	c0, c1;

	for(; source_it < source_end; destination_it +=8)
		{
		c0.d = _amemd8_const(source_it);
		source_it += 4;
		c1.d = _amemd8_const(source_it);
		source_it += 4;
		
		b1.i[0] = _spacku4(_shr2(c0.i[1], sh), _shr2(c0.i[0], sh));
		b1.i[1] = _spacku4(_shr2(c1.i[1], sh), _shr2(c1.i[0], sh));

		_memd8(destination_it) = b1.d;
		}
	
	source_end = a2 + size;
	for(; source_it < source_end; ++source_it, ++destination_it)
		{
		*destination_it = *source_it>>sh;
		}
	}

XRAD_END
