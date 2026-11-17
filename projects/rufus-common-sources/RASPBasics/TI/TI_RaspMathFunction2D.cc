
/*
// функции из posix для ендианов
uint32_t htonl(uint32_t hostlong); — конвертирует 32-битную беззнаковую величину из локального порядка байтов в сетевой;
uint16_t htons(uint16_t hostshort); — конвертирует 16-битную беззнаковую величину из локального порядка байтов в сетевой;
uint32_t ntohl(uint32_t netlong); — конвертирует 32-битную беззнаковую величину из сетевого порядка байтов в локальный;
uint16_t ntohs(uint16_t netshort); — конвертирует 16-битную беззнаковую величину из сетевого порядка байтов в локальный.
*/

XRAD_BEGIN





inline signed_short_data &signed_short_data::operator *= (double v)
	{
	int	sh = RASPFunctors::shift_multiplier<value_type>::calculate_correction_shift(v);

	value_type	x(value_type(float(1<<sh)*v));
	return AS_2D_OpEq(*this, x, RASPFunctors::shift_multiplier<value_type>(sh));
	}






inline void ApplyFunction2D(signed_short_data &a, const RaspTableFunction<short> &f)
	{
	const short	*base = &f[0];

	int16_t	*it = &a.at(0,0);
	int16_t	*ie = it+a.vsize()*a.hsize();
	buffer64	b, c;
	for(;it<ie; it+=4)
		{
		b.d = _amemd8(it);

		c.s[0] = base[b.s[0]];
		c.s[1] = base[b.s[1]];
		c.s[2] = base[b.s[2]];
		c.s[3] = base[b.s[3]];

		_amemd8(it) = c.d;
		}
	}

inline int16_t	MaxValue(const signed_short_data & restrict data, int* v, int* h)
	{
	bool	step_one = (min(data.row_step(), data.column_step())==1);
	bool	continuous = (data.row_step() == 1) ?
				(data.column_step()==data.hsize()):
				(data.row_step()==data.vsize());
	
	if(v || h || !continuous || !step_one)
		{
		return MaxValue<MathFunction<int16_t> >(data, v, h);
		}	
	
	const int16_t	*it = &data.at(0,0);
	const int16_t	*ie = it + data.vsize()*data.hsize();
	buffer32	rr;
	rr.s[0] = rr.s[1] = *it;
	
	buffer64	b;
	
	for(;it<ie; it+=4)
		{
		b.d = _amemd8_const(it);
		
		b.i[0] = _max2(b.i[0],rr.i);
		rr.i = _max2(b.i[0],b.i[1]);
		}
	return max(rr.s[0],rr.s[1]);
	}


XRAD_END
