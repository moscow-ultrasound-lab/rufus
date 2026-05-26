#ifndef __analyzer
#define __analyzer

#include "S500_CFMRawDataDisplay.h"
//#include "WallFilters.h"
//#include "KarhunenLoeve.h"
#include "CFDataPhaseAnalyzer.h"

XRAD_BEGIN

//void	CalculateCorrelation(const ComplexFunctionF32 &burst, complexF64 &accumulator, double &divisor);
void	MenuAnalysis(S500_CFMFrameSet &frames);
// void	FilterBankOneFrame(ComplexFunctionMD_F32 &frame, size_t n_beams_in_sweep, size_t	n_filtered_components, double bank_threshold);

//void	TissueMotionCompensationOneFrame(ComplexFunctionMD_F32::slice_type /* CFDataPhaseAnalyzer::shots_array_t */ &slice, size_t n_beams_in_sweep, size_t n_shots, size_t accumulator_blur, bool acceleration_flag);

XRAD_END

#endif //__analyzer
