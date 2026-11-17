#ifndef	__focuser_h
#define	__focuser_h

#include "SyntheticFocusingOptions.h"
#include "LUT_8bit_interpolator.h"

XRAD_BEGIN

//#error continue from here



class	SyntheticApertureFocuser : public SyntheticFocusingOptions
{
	PARENT(SyntheticFocusingOptions);
//	void	AllocateRawSFData();
//	void	DisplayRawData();


public:
	virtual	void	Batch();
	virtual	void	InitWork();
	virtual	void	EndWork();
	TimeTable	correction_delays;

protected:
//	void	AberrateRawData();

	void	FocusWholeAperture(size_t frame_no, physical_angle start_angle, physical_angle end_angle);
	void	FocusSubapertures(physical_angle start_angle, physical_angle end_angle);
	void	ComputeCorrections(physical_angle angle, TimeTable	&correction_delays, RealFunction2D_F64	&amplitude_corrections);
	
	void	FocusFramesWithFocusingDefect();
	size_t	FirstSubapertureElement(size_t sub) const;
	size_t	LastSubapertureElement(size_t sub) const;

//	void	FocusWholeAperture(ComplexFunction2D_F32 &frame_buffer, physical_angle start_angle, physical_angle end_angle);
	void	AnalyzeFocusingDefect();

	physical_length ComputeLens(physical_length element_x, physical_length focus_r, double target_sine, double target_cosine) const;


protected:

	void	DCOffsetCorrection(ComplexFunctionF32 &signal);

	void	GatherDelayedSignals(ComplexFunctionF32 &ray_buffer, size_t first_rx_el, size_t last_rx_el);
	void	ComputeDelayedSignals(physical_angle alpha, const TimeTable &correction_delays, const RealFunction2D_F32 &amplitude_corrections);

protected:



	RealFunction2D_F32 apodization;

	DataArray<physical_length> elements_offsets;

	TimeTable TX_Delays;
	TimeTable RX_Delays;

public:
// 	ComplexFunctionF32	delayed_waveform;

//	index_vector	current_waveform_index;

	DataArrayMD<ComplexFunction2D_F32>	delayed_signals;

	void	ComputeApodization();

	void	ComputePathDifferences(TimeTable &delays, physical_angle angle, const aperture_focusing &focus);
//	void	ComputePathDifferencesDefect(TimeMatrix &delays);
	void	ComputeElementsOffsets();

	SyntheticApertureFocuser();
	virtual ~SyntheticApertureFocuser();
};

XRAD_END

#endif