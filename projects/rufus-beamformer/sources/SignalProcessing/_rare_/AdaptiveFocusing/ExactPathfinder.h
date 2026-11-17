#ifndef __exact_pathfinder_h
#define __exact_pathfinder_h


#include "PathfinderBase.h"

XRAD_BEGIN

class	Exact_Pathfinder : public Pathfinder_Base{
protected:
virtual	void	Batch();
virtual	void	InitWork();
virtual	void	EndWork();

public:
	ComplexFunction2D_F32 Interp_Buffer;
	
	void	ProcessSample(size_t);
virtual	void	Process_Group();
	void	Interpolate();
	//void	ReInterpolate();

	Exact_Pathfinder();
	virtual ~Exact_Pathfinder();
	};

XRAD_END

#endif 