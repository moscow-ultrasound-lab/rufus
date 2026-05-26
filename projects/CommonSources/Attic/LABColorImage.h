#ifndef __lab_color_image_h
#define __lab_color_image_h

#include <XRADBasic/Sources/SampleTypes/LABColorSample.h>

XRAD_BEGIN

typedef DataArray<LABColorSample> LAB_array;
typedef DataArray2D<LAB_array> LAB_array2D;

class	LABColorImage : public LAB_array2D
{
	PARENT(LAB_array2D);
	void	MapPlates();

public:
	RealFunction2D_F64	L, a, b;

	LABColorImage(){};
	LABColorImage(int v, int h) : parent(v, h){ MapPlates(); };

	LABColorImage(const ColorImageF32 &im);
	void	CopyData(const ColorImageF32 &im);
	void	MakeCopy(const ColorImageF32 &im);
	void	realloc(size_t v, size_t h)
	{
		parent::realloc(v, h);
		MapPlates();
	};

	void	Display(const char *theTitle = NULL, bool bypass = false) const;
};

XRAD_END


#endif // __lab_color_image_h
