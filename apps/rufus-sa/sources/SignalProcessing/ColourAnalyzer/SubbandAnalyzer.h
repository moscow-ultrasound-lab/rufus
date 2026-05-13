#ifndef __subband_analyzer_h
#define __subband_analyzer_h

#include "ColourEncode.h"
#include "SignalProcessing/SignalProcessor.h"

XRAD_BEGIN

class	SubbandAnalyzer : public SignalProcessor
	{
		int	n_bands;
		int	n_subbands;
		
		int	fft_length;
		DataArray2D<DataArray<RealFunctionF32> > filters;
		
		double	filters_bandwidth;
		RealFunctionF32	filters_carriers;
		
		GraphSet	filter_gs;
		double	graph_scale_0;
		double	graph_d_scale;
		
		void	PrepareRayForProcessing();

	protected:
	
		void	InitWork();
		void	InitFilters();
		void	Batch();
		void	AnimateResult(DataArray<RealFunction2D_F32> &);
	
	public:
		SubbandAnalyzer() : filter_gs("Filter GS", "", ""){}
		virtual ~SubbandAnalyzer(){}
	};

XRAD_END


#endif //__subband_analyzer_h
