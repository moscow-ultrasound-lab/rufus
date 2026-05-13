#ifndef __correlator_h
#define __correlator_h

#include "SignalProcessing/SignalProcessor.h"

XRAD_BEGIN

class	Correlator : public SignalProcessor
	{
	float	Mu;
	float	side_Lobe, U_Bound;
	
virtual	void	Batch();
virtual	void	InitWork();
virtual	void	EndWork();


public:	
	Correlator();
	virtual ~Correlator();
	};

XRAD_END

#endif //__nl_pathfinder
