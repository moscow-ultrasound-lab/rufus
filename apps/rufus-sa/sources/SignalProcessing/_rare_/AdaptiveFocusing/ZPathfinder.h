#ifndef _z_pathfinder_h
#define _z_pathfinder_h

#include "PathfinderBase.h"

XRAD_BEGIN

class	ZPathfinder : public Pathfinder_Base{
protected:
virtual	void	Batch();
virtual	void	InitWork();
virtual	void	EndWork();

public:
	ComplexFunction2D_F32 Interp_Buffer;
	
	void	Process_Sample(size_t);
	void	Process_ZPicture();
virtual	void	Process_Group();
	void	Interpolate();
	//void	ReInterpolate();

	ZPathfinder();
	virtual ~ZPathfinder();
	};

XRAD_END


#endif //_z_pathfinder_h
