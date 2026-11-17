#ifndef __nl_pathfinder
#define __nl_pathfinder

#include "SignalProcessing/SignalProcessor.h"

XRAD_BEGIN

class	NL_Pathfinder : public SignalProcessor
	{
static	int	Pathfinder_Count;
	// Џараметры эмулированной решетки:
	float	Mu;
	float	side_Lobe, U_Bound;
	
	float	Amplification(float);

	float	first_SL, second_SL;
	float	first_SL_Level, second_SL_Level;
	void	Intelligent_Suppress(float, float);
	void	Single_Intelligent_Suppress(short, float, float);
	void	Detect_1st_SL(short, float*, float*);

virtual	void	Batch();
virtual	void	InitWork();
virtual	void	EndWork();

	bool	Cross_Terms;
	bool	second_SLs;
	bool	Intelligent_SLs;
	bool	FPS_On;

public:
	ComplexFunction2D_F32 Interp_Buffer1, Interp_Buffer2;
static	int	FFT_Length;
static	int	From_Sample, To_Sample, delta_Sample;
static	bool	do_Focusing;
static	bool	Use_Frequency_Filtre;
	int	Path_Len;
	
	float	z0, dZ, dX, dAngle;
static	float	Receptor_X;
static	float	FX_Point;
	float	Ro_Index;
	float	R0;
	float	Ro;
	
	float	Beta;
	float	Zeta;
	float	alpha;

	float	Scale_Factor;
	float	Schift;
	
	float	Find_Scale_Factor();	
	float	Find_Schift();	
	
virtual	void	ProcessBuffer();
virtual	void	Process_Group();
	void	Interpolate(int Subaperture);
	void	ReInterpolate(int Subaperture);
virtual	void	ProcessInitDialog();
	
	NL_Pathfinder();
	virtual ~NL_Pathfinder();

	void	sqrt_Group();
	void	power_Group(float);
	};


XRAD_END

#endif //__nl_pathfinder
