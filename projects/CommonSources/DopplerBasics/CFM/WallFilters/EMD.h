#ifndef __emd
#define __emd
#include "WallFilters.h"

XRAD_BEGIN

struct Interpolation 
{
	virtual void Apply( RealFunctionF32 &burst, RealFunctionF32 &a, RealFunctionF32 &b, RealFunctionF32 &an, RealFunctionF32 &bn)  const = 0;
	virtual ~Interpolation(){}
};

struct LinearInterpolation : Interpolation
{
	void Apply(RealFunctionF32 &burst, RealFunctionF32 &a, RealFunctionF32 &b, RealFunctionF32 &an, RealFunctionF32 &bn) const;
};
struct CSplineInterpolation : Interpolation 
{
	void Apply(RealFunctionF32 &burst, RealFunctionF32 &a, RealFunctionF32 &b, RealFunctionF32 &an, RealFunctionF32 &bn) const;
};

struct WallFilterEMD : WallFilter 
{
	enum e_interpolation_type{e_linear, e_spline};
	e_interpolation_type flag;
	size_t number_of_iterations;

	WallFilterEMD(e_interpolation_type in_flag, size_t in_number_of_iterations) : flag(in_flag), number_of_iterations(in_number_of_iterations) {}
	void Apply(ComplexFunctionF32 &burst) const;
	virtual	wstring	filter_name() const override { return ssprintf(L"WallFilterEMD"); };

	virtual	bool is_size_actual(size_t in_vector_size) const override { throw invalid_argument("WallFilterEMD is obsolete"); }
};

void LocalExtrema( RealFunctionF32 &burst, RealFunctionF32 &a, RealFunctionF32 &b, RealFunctionF32 &an, RealFunctionF32 &bn); 


XRAD_END

#endif //__emd
