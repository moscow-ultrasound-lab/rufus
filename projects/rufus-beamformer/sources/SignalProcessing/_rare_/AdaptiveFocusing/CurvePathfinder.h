#ifndef __curve_pathfiner_h
#define __curve_pathfiner_h

#include "PathfinderBase.h"
#include "interpolation_many_obsolete/Interpolation.h"

XRAD_BEGIN

class	Curve_Pathfinder : public Pathfinder_Base
	{
	Interpolator **Intrrr;
	int	nRoofs;

protected:
virtual	void	Batch();
virtual	void	InitWork();
virtual	void	EndWork();

public:
	ComplexFunction2D_F32 Interp_Buffer;

virtual	void	Process_Interval();
virtual	void	Process_Group();
	void	Interpolate();
	void	ReInterpolate();

virtual	void	ProcessInitDialog();
	Curve_Pathfinder();
	virtual ~Curve_Pathfinder();
	};

XRAD_END

#endif __curve_pathfiner_h
