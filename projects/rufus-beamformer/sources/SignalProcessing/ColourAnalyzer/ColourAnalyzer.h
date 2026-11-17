#ifndef _colour_analyzer_h
#define _colour_analyzer_h

#include "ColourEncode.h"
#include "SignalProcessing/SignalProcessor.h"
#include "LayersContainer.h"

XRAD_BEGIN

class	ColourAnalyzer : public SignalProcessor
	{
	ComplexFunctionF32	phaseCorrection;

//	ComplexFunctionF32 ComputeShiftFilter(double shiftValue);
	size_t	filterSize;
	void	PrepareData();
	
	size_t	n_ranges;
	size_t	ft_size;
	size_t	n_spatial_layers;
	double	default_frequency;
	double	default_bandwidth;

	LayersContainer	process_buffer;
	GrayScanConverter	moment_frequency;
	GrayScanConverter	moment_bandwidth;
	GrayScanConverter	range_brightness;
	GrayScanConverter	sample_brightness;
	GrayScanConverter	maxima_count;
	
	ColorEncodeFunction	*color_encode;
	
	
	void	MeasureSpectrum();
	void	FitDataToRanges(double colour_amplification);
	void	CorrectAttenuation();
	
	void	PrepareResultingImages();
	void	BuildColourPicture(const RealFunction2D_F32 &r, const RealFunction2D_F32 &g, const RealFunction2D_F32 &b);
	void	BuildGrayPicture(const RealFunction2D_F32 &m);
	void	BuildColorParametricPicture(double &ab_amplification);
	void	DisplayStatistics(const LayersContainer &total_layers);
	void	BuildResonancePicture();

	

public:

	void	InitWork();
//	void	ProcessInitDialog();
	void	Batch();
	void	EndWork();

	ColourAnalyzer();
	virtual ~ColourAnalyzer();
	};

XRAD_END


#endif //_colour_analyzer_h
