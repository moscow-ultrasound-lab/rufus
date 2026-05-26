#ifndef __blur_algorithms_h
#define __blur_algorithms_h

#include	<stdint.h>

XRAD_BEGIN

/*
void	BIXBlur1D(fixed_point_array &f, double r);

template<class T>
void	BIXBlur1D(DataArray<T> &f, double r);


template<class AR>
void	NoneBlur1D(AR &, double );

template<class AR>
void	RectBlur1D(AR &f, double r);

template<class AR>
void	GaussianBlur1D(AR &f, double r);
*/

//---------------------------------------------------------
template<class AR2D>
void	UniversalBlur2D(AR2D &im, double radius_v, double radius_h);

XRAD_END

#include "BlurAlgorithms.cc"

#endif // __blur_algorithms_h

