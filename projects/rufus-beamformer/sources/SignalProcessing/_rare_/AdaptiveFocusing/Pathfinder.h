#ifndef __pathfinder_h
#define __pathfinder_h

#include "PathfinderBase.h"

XRAD_BEGIN

class	Pathfinder : public Pathfinder_Base{
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

	Pathfinder();
	virtual ~Pathfinder();
	};

XRAD_END

#endif// __pathfinder_h
