#ifndef __pathfinder_base_h
#define __pathfinder_base_h

#include "SignalProcessing/SignalProcessor.h"

XRAD_BEGIN


class	Pathfinder_Base : public SignalProcessor
	{
	
static	size_t	Pathfinder_Count;
protected:

	size_t	n_intervals;
	size_t	interval_increment;

// Џараметры эмулированной решетки:
//	float	Mu;
//	float	deltaY;
//	float	side_Lobe, U_Bound;
	
	ComplexFunctionF32 Fresnel_Buffer;
	complexF32 Find_Fresnel1(float, ptrdiff_t, float);
	void	Find_Fresnel(float);
	bool	Do_Fresnel_Correction;
	
public:	
static	size_t	FFT_Length;
static	size_t	From_Sample, To_Sample, delta_Sample;
	size_t	Path_Len;

	float	dZ, dX, dAngle;
	float	R0;// Frequency;
	float	R_Equivalent;//, wave_k;
	float	a;

static	float	Receptor_X;
static	float	FX_Point;
	float	Ro_Index;
	float	Ro;
	
	float	Zeta;
	float	alpha;
	
	float	Scale_Factor;
	float	Schift;
	
	float	Find_Scale_Factor();	
	float	Find_Schift();	

virtual	void	InitWork();
virtual	void	ProcessInitDialog();
	
	Pathfinder_Base();
	virtual ~Pathfinder_Base();
	};

XRAD_END

#endif //__pathfinder_base_h