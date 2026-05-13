#ifndef _white_analyzer_h
#define _white_analyzer_h

#include "ColourEncode.h"
#include "SignalProcessing/SignalProcessor.h"
#include "RangeSpectrumAnalyzer.h"


XRAD_BEGIN

class	WhiteAnalyzer : public RangeSpectrumAnalyzer
	{
	PARENT(RangeSpectrumAnalyzer);
//	void	PrepareData();
	
//	DataArray<ComplexFunction2D> process_buffer;
//	size_t n_ranges;	
//	size_t etalon_length;
//	float	etalon_step; //NB: float, not int
//	RealFunction2D_F32 white_spectra;
//	void	CalculateWhite();
//	GrayScanConverter	sample_brightness;
//	GrayScanConverter	range_brightness;

	size_t range_length;
	RealFunction2D_F32 white_spectra;
	
	void	AnalyzeLocalSpectra();
	float	ExtractWhiteComponent(const RealFunctionF32 &spectrum,
			const RealFunctionF32 &white,
			RealFunctionF32 &white_component,
			RealFunctionF32 &colour_component) const;

public:

	void	InitWork();
//	void	ProcessInitDialog();
	void	Batch();
	void	EndWork();

	WhiteAnalyzer();
	virtual ~WhiteAnalyzer();
	};

XRAD_END


#endif //_white_analyzer_h
