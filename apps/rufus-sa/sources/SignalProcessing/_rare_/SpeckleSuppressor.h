#ifndef __speckle_suppressor_h
#define __speckle_suppressor_h

#include "SignalProcessing/_rare_/AdaptiveFocusing/Pathfinder.h"

XRAD_BEGIN

class	Speckle_Suppressor : public Pathfinder
	{
	PARENT(Pathfinder);
virtual	void	Batch();
virtual	void	init_Work();
virtual	void	end_Work();


public:
static	int	Speckle_Suppressor_Count;	
static	int	n_Cases;
static	int	Use_Array_Factor;
	int	Personal_No;
	Speckle_Suppressor	*NextCase;

	Speckle_Suppressor();
	virtual ~Speckle_Suppressor();

virtual	void	Process_Interval();
	void	Interpolate();
	void	ReInterpolate();

	GrayScanConverter *Detected_Signal();
	};

XRAD_END

#endif __speckle_suppressor_h
