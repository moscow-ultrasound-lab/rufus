#ifndef __interpolation_h
#define __interpolation_h

#include <XRADBasic/SampleTypes.h>

XRAD_BEGIN

class	Interpolator
	{
protected: size_t	nFiltres, nTaps;
	size_t	centreTap;
	size_t	maxWhere;
	
protected: complexF32 **Filtres;
public:	
	void	SetMaxWhere(size_t);
	Interpolator();
	virtual ~Interpolator();
	void	I_Interpolator(size_t, size_t, size_t, double);
	void	I_Convolutor(size_t, size_t, size_t, complexF32 *, float);
	complexF32 Interpolate(complexF32 *, double);
	
	void	Analyze();
	};
	
enum{
	Hamming, Gauss, rectangle, roof4, roof8, deltaF, Custom
	};

XRAD_END


#endif