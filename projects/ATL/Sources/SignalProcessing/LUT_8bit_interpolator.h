#ifndef __focuser_containers_h
#define __focuser_containers_h

#include "ATL_src_reduced/atl_lut_interpolator.h"
//#include "Q:\projects\ATL\Sources\ATL_src_reduced/atl_lut_interpolator.h"

XRAD_BEGIN

class	LUT_8bit_interpolator : public MathFunction<char, int, AlgebraicStructures::FieldTagScalar>
{
	static bool	ATL_LUT;
	static DataArray<int>	LUT;
//		static	int	*LUT;
	static	size_t	filter_order;
	static	size_t	nEntriesPerPnt;
	static	size_t	nEntriesPerFilter;
	static	size_t	nEntriesPerTap;

	static	ptrdiff_t	centre_offset; //filter_order/2

public:
	static	size_t	interpolation_factor;

	static	FILE	*Open_LUT_File();
//		static	void	Get_LUT();
public:

	static	void	LoadLUT_ATL();

	static	void	InitLUT(int fSize = 15);

	static	void	AnalyzeLUT();

	complexF32 interpolate(double x) const;
	LUT_8bit_interpolator(){};
	LUT_8bit_interpolator(int s) : MathFunction<char, int, AlgebraicStructures::FieldTagScalar>(s){ ATL_LUT=false; };
};

//inline Int8 *StdConversion( const char *) { return 0;}

XRAD_END


#endif //__focuser_containers_h
