

XRAD_BEGIN


template<class T, class ST>
inline void	RASPArray2D<T,ST> ::CopyData(const RASPArray2D<T,ST> &original)
	{
	memcpy(&at(0,0), &original.at(0,0), vsize()*hsize()*sizeof(value_type));
	}


//--------------------------------------------------------------------------------------
//	ускоренное форма записи действия r1 = op(r2,r3)
//	MathAlgorithms.h


//template<class T, class ST>
//void	AddArrays2D(RASPArray2D<T,ST> &r1, const RASPArray2D<T,ST> &r2, const RASPArray2D<T,ST> &r3)
//	{
//	AAA_2D_Op(r1, r2, r3, plus<T,ST>());
//	}
//
//template<class T, class ST>
//void	SubtractArrays2D(RASPArray2D<T,ST> &r1, const RASPArray2D<T,ST> &r2, const RASPArray2D<T,ST> &r3)
//	{
//	AAA_2D_Op(r1, r2, r3, minus<T,ST>());
//	}



//-----------------------------------------------------------------------------
//
//	operator *= (double) и (fixed_point)
//
//-----------------------------------------------------------------------------

template<class T, class ST>
FixedPointArray2D<T,ST>	&FixedPointArray2D<T,ST> :: operator *= (double v)
	{
	int	correction_shift = RASPFunctors::shift_multiplier<T>::calculate_correction_shift(v);

	T	factor(T(float(1<<correction_shift)*v));
	return AS_2D_OpEq(*this, factor, RASPFunctors::shift_multiplier<T>(correction_shift));
//	return Array2DScalarOperationEq__(*this, factor, RASPFunctors::shift_multiplier<T,ST>(correction_shift));
	}

template<class T, class ST>
FixedPointArray2D<T,ST>	&FixedPointArray2D<T,ST> :: operator *= (T v)
	{
	return	Array2DScalarOperationEq__(*this, v, RASPFunctors::shift_multiplier<T,ST>());
	}





XRAD_END
