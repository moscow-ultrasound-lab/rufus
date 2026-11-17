#include "pre.h"
#include <Attic/LABColorImage.h>

XRAD_BEGIN


LABColorImage::LABColorImage(const ColorImageF32 &original) : DataArray2D<LAB_array>(original.vsize(), original.hsize())
{
	MapPlates();
	Apply_AA_2D_F2(*this, original, Functors::assign());
}

void	LABColorImage::MapPlates()
{
	long q_st = sizeof(at(0, 0))/sizeof(at(0, 0).L);
	L.UseData(&(at(0, 0).L), vsize(), hsize(), hsize()*q_st, q_st);
	a.UseData(&(at(0, 0).a), vsize(), hsize(), hsize()*q_st, q_st);
	b.UseData(&(at(0, 0).b), vsize(), hsize(), hsize()*q_st, q_st);
}

void	LABColorImage::MakeCopy(const ColorImageF32 &original)
{
	realloc(original.vsize(), original.hsize());
	MapPlates();

	Apply_AA_2D_F2(*this, original, Functors::assign());
}

void	LABColorImage::CopyData(const ColorImageF32 &original)
{
	Apply_AA_2D_Different_F2(*this, original, Functors::assign());
}

#ifdef __XRAD_INTERFACE_FUNCTIONS
void	LABColorImage::Display(const char *theTitle, bool bypass) const{
	if(bypass) return;
	size_t	answer = 0;

	char	title[256];
	if(theTitle) strcpy(title, theTitle);
	strcpy(title, "Lab image display");



	while(answer != 4)
	{
		answer = GetButtonDecision(title, //5,
		{"Color image",
			"L plate",
			"a plate",
			"b plate",
			"Exit display"}
		);
		switch(answer)
		{
			case 0:
			{
				ColorImageF64	im(*this);
				Error("Not done");
				//im.DisplayRaster(theTitle);
			}
			break;
			case 1:
				DisplayMathFunction2D(L, "L plate");
				break;
			case 2:
				DisplayMathFunction2D(a, "a plate");
				break;
			case 3:
				DisplayMathFunction2D(b, "b plate");
				break;
			case 4:
				break;
		}
	}
}
#else

void	LABColorImage::Display(const char *, bool) const{}

#endif //__XRAD_INTERFACE_FUNCTIONS

XRAD_END
