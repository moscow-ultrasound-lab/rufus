#ifndef __noise_twister_h
#define __noise_twister_h

#include "SignalProcessing/_rare_/AdaptiveFocusing/ZPathfinder.h"

XRAD_BEGIN

class	NoiseTwister : public ZPathfinder{
protected:
	void	Batch();
public: 
	void	TwistNoise();
	
	NoiseTwister(){};
	virtual ~NoiseTwister(){};
	};

XRAD_END

#endif //__noise_twister_h
