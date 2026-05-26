#ifndef XRayRoiPreview_h__
#define XRayRoiPreview_h__

/*!
	\file
	\date 2019/11/08 18:02
	\author kulberg

	\brief  
*/

#include <XRADDicom/XRADDicom.h>
#include "XRayRoiTaggingReport.h"

XRAD_BEGIN

void	DrawDottedEllipse(ColorImageUI8 &preview, const range2_F32 &loc, const ColorSampleUI8 &color, double relative_dot_radius, double relative_gap);

//void	DrawDottedEllipse(ColorImageUI8 &preview, const XRayRoiTaggingReport &roi, double relative_radius, double relative_gap);
ColorImageUI8	GeneratePreview(const RealFunction2D_F32 &image, double wc, double ww, bool negative);



template<class ARR_T, class C>
void	FillEllipse(DataArray2D<ARR_T> &preview, const range2_I32 &loc, const C &c)
{

	auto	y0 = range(loc.p1().y(), 0, preview.vsize()-1);
	auto	y1 = range(loc.p2().y(), 0, preview.vsize()-1);
	auto	x0 = range(loc.p1().x(), 0, preview.hsize()-1);
	auto	x1 = range(loc.p2().x(), 0, preview.hsize()-1);

	auto	xc = loc.center().x();
	auto	yc = loc.center().y();
	auto	xr = loc.width()/2;
	auto	yr = loc.height()/2;

	auto	equation = [&xc,&yc,xr,yr](int i, int j)
	{
		double	x = j-xc;
		double	y = i-yc;

		return (square(x/xr) + square(y/yr)) < 1;
	};

	for(int i = y0; i < y1; ++i)
	{
		for(int j = x0; j < x1; ++j)
		{
			if(equation(i, j))
			{
				preview.at(i, j) = typename ARR_T::value_type(c);
			}
		}
	}
}

XRAD_END

#endif // XRayRoiPreview_h__
