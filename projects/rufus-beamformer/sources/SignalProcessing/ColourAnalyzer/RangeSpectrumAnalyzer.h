#ifndef _range_spectrum_analyzer_h
#define _range_spectrum_analyzer_h

#include "ColourEncode.h"
#include "SignalProcessing/SignalProcessor.h"

XRAD_BEGIN

class	RangeSpectrumAnalyzer : public SignalProcessor
	{
	protected:
		size_t n_ranges;
		//size_t range_length;
		float	range_step; //NB: float, not int
		const	size_t default_range_length;
	
		ScanConverterOptions	sco;
		ColorImageF32	parametric_picture;
		RealFunction2D_F32	sample_brightness;
		RealFunction2D_F32	range_brightness;

		//RealFunction2D_F32 white_spectra;
		void	CalculateWhite(size_t range_length, RealFunction2D_F32 &white_spectra);
		void	NormalizeSpectrum(size_t range_length, bool dynamic_normalization = true);
		void	NormalizeBrightness();

		void	InitWork();

		RangeSpectrumAnalyzer(size_t drl) : default_range_length(drl){}
		virtual ~RangeSpectrumAnalyzer(){}
	};

XRAD_END


#endif //_range_spectrum_analyzer_h
