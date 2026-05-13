#ifndef __Displayer2D_h
#define __Displayer2D_h

#include "Options.h"
#include <MathFunction2D.h>

#error obsolete file

XRAD_BEGIN

class	AcousticFrameDisplayer : public RealFunction2D_F32, public ScanConverterOptions
	{
protected:

// 	string unitsX;
// 	string unitsY;
// 	string imageTitle;
// 	string dataUnits;
	
//	double	x0, deltaX, y0, deltaY;

public:
	AcousticFrameDisplayer(int x = 0, int y = 0);
	virtual ~AcousticFrameDisplayer();
//	void	LogCompress(double dynRangeShadows = 0, double dynRangeLights = 0);

//virtual	void	Display(const char *Title);

//	void	SetArguments(double yMin = 0, double yMax = 1, double xMin = 0, double xMax = 1);
//	void	SetUnits(string dataU, string yU, string xU);

friend	class	SectorData;
	};


XRAD_END

#endif //__Displayer2D_h
