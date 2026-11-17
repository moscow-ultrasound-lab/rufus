#ifndef	ATTENUATOR_H
#define	ATTENUATOR_H

#include	"SignalProcessing/SignalProcessor.h"

XRAD_BEGIN


class	Attenuator : SignalProcessor
	{
	size_t	n_intervals;
	size_t	interval_increment;

	int	From_Sample, To_Sample;
	
	GrayScanConverter *Coefficient_Map;
virtual	double	Process_Interval();
	double	Frequency(double);
	double	Cryterion(double, double);

virtual	void	InitWork();
virtual	void	EndWork();
virtual	void	Batch();	

		void	DisplayColour();
		void	SaveCoefMap();
		void	ReadCoefMap();
public:
	Attenuator();
	virtual ~Attenuator();
	};

XRAD_END

#endif