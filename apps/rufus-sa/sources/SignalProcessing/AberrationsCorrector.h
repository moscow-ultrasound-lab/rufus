#ifndef __aberrations_corrector_h
#define __aberrations_corrector_h

#include "SyntheticApertureFocuser.h"

XRAD_BEGIN

enum	shift_detection_algorithm
	{
	cross_correlation,
	deconvolution,
	phase_detection
	};


class	AberrationsCorrector: public SyntheticApertureFocuser
	{
	PARENT(SyntheticApertureFocuser);
//	void	FocusRay(int frame_no, physical_angle direction);
//	void	Correct_Interval(ComplexFunction2D_F32 *);
	RealFunctionF32 &Compute_RX_Phases(ComplexFunction2D_F32 *);
	RealFunctionF32 &Compute_TX_Phases(ComplexFunction2D_F32 *);
	

	void	CreateComment();

	size_t	from_sample, to_sample;

	complexF32 F_Correlation(const complexF32&, const complexF32&);
	void	AnalyzeDelays(TimeTable &delays, shift_detection_algorithm algorithm);
	DataArrayMD<TimeTable>	AberrationsMap;
	RealFunctionF32	slopes;

	size_t	Intervals_To_Compute_Correction;
	int	Correlation_Power;
	float	Correlations_Count, Bad_Correlations_Count;
	RealFunction2D_F32	amplitude_corrections;

public:
	void	InitWork();
	void	Batch();
	void	ComputeAberration();

	RealFunctionMD_F32	displayer;
	DataArrayMD<RealFunction2D_F32>::slice_type::invariable displayer_slice;

	virtual	void	Display(const char *);
//	void	DisplayAberrations(const char *);
	AberrationsCorrector();
	virtual ~AberrationsCorrector();
	};


XRAD_END
#endif //__aberrations_corrector_h
