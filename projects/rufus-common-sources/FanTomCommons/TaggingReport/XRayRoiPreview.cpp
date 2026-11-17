#include "pre.h"
#include "XRayRoiPreview.h"
/*!
	\file
	\date 2019/11/08 18:02
	\author kulberg

	\brief 
*/

XRAD_BEGIN

using namespace Dicom;



void	FillCircle(ColorImageUI8 &preview, const point2_I32 &centre, double r, ColorSampleUI8 c)
{
	range2_I32	loc(round_n(centre.y() - r), round_n(centre.x() - r), round_n(centre.y() + r), round_n(centre.x() + r));
	FillEllipse(preview, loc, c);
}



void	DrawDottedEllipse(ColorImageUI8 &preview, const range2_F32 &loc, const ColorSampleUI8 &color, double relative_dot_radius, double relative_gap)
//void	DrawDottedEllipse(ColorImageUI8 &preview, const XRayRoiTaggingReport &roi, double relative_dot_radius, double	relative_gap)
{
//	auto &loc(roi.loc);
	double	dot_pixel_radius = 1 + (relative_dot_radius*preview.vsize())/3000;
	double	dx = 2*relative_gap*dot_pixel_radius;


	physical_angle	fi = radians(0);
	physical_angle	previous_fi = radians(0);

//	auto	color = roi.color();

	const double	h = double(preview.vsize())/10;
	const double	reference_distance = h;

	double	a = loc.width()/2;
	double	b = loc.height()/2;

	if(!loc.width() || !loc.height()) return;

	while(!(fi.radians() > 0 && previous_fi.radians() < 0))
	{
		previous_fi = fi;
		double	c = cosine(fi);
		double	s = sine(fi);


		double	rx =a*s;
		double	ry = b*c;
		double	r = hypot(rx, ry);
		physical_angle	real_fi = radians(atan2(s*a, c*b));
		
		double	cr = cosine(real_fi);
		double	sr = sine(real_fi);

		double	cos_tangent = cr*sr*(a-b)/sqrt((a*a*cr*cr + b*b*sr*sr)*(a*a*sr*sr + b*b*cr*cr));//скалярное произведение радиус-вектора и его производной по fi 
																								//(нехорошее упрощение, должно быть по real_fi, но небольшую коррекцию обеспечивает)
		double	sin_tangent = sqrt(1. - square(cos_tangent));


		double	y = loc.center().y() + ry;
		double	x = loc.center().x() + rx;
		FillCircle(preview, point2_I32(y, x), dot_pixel_radius, color);

 		physical_angle	d_real_fi = radians(atan(dx/(r*sin_tangent)));
//		physical_angle	d_real_fi = radians(atan(dx/r));
		real_fi += d_real_fi;

		fi = radians(atan2(sine(real_fi)*b, cosine(real_fi)*a));
	}
}

void	DrawEllipse_(ColorImageUI8 &preview, XRayRoiTaggingReport roi)
{
	auto &loc(roi.loc);

	auto	equation = [loc](int i, int j)
	{
		double	x = j-loc.center().x();
		double	y = i-loc.center().y();
		double	factor = sqrt(loc.width()*loc.height());
		return factor*(sqrt(square(2*x/loc.width()) + square(2*y/loc.height())) - 1);
	};

	double	pixel_width = 6;
	auto	color = roi.color();
	for(int i = 0; i < preview.vsize(); ++i)
	{
		for(int j = 0; j < preview.hsize(); ++j)
		{
			auto	e = equation(i, j);

			if(fabs(e) < pixel_width)
			{
				preview.at(i, j) = color;
			}
		}
	}
}




ColorImageUI8	GeneratePreview(const RealFunction2D_F32 &img, double wc, double ww, bool negative)
{
	try
	{
		ColorImageUI8 result;
// 		Dicom::image &img = dynamic_cast<Dicom::image &>(instance);
// 		auto	initial = img.get_image();

// 		float	wc = img.get_double(e_window_center);
// 		float	ww = img.get_double(e_window_width);
		float	black = wc - ww/2;
		float	white = wc + ww/2;

		auto	to_pixel = [negative, black, white, ww](ColorSampleUI8 &p, const float &x)
		{
			float	c = range((x-black)*255./ww, 0, 255);
			p = negative ? ColorSampleUI8(255-c) : ColorSampleUI8(c);
		};
		result.MakeCopy(img, to_pixel);
		return result;
	}
	catch(...)
	{
		return ColorImageUI8(100, 100, ColorPixel(RandomUniformF64(0, 255), RandomUniformF64(0, 255), RandomUniformF64(0, 255)));
	}
}


XRAD_END

