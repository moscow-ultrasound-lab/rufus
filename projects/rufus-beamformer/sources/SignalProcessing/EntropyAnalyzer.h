#ifndef __entropy_analyzer_h
#define __entropy_analyzer_h

#include "SignalProcessing/SignalProcessor.h"

XRAD_BEGIN





class	EntropyAnalyzer : public SignalProcessor
	{
		
	private:
				
				
		virtual	void	Batch();
		
		//DataArray<AcousticFrameDisplayer>	detector;
		MathFunctionMD<RealFunction2D_F32>	detector;
		int	histogram_size;
		
		virtual	void	InitWork();
		void	Analyze1DHistograms();
		void	Analyze2DHistograms();

//		double	ComputeImageEntropy(RealFunction2D_F32 &img, double min_value, double max_value);

		float	max_value, min_value;

	public:	
		EntropyAnalyzer();	
//		virtual ~EntropyAnalyzer();	
		
	};


XRAD_END

	
#endif //__entropy_analyzer_h
