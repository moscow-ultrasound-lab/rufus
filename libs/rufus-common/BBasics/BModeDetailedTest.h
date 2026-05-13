#ifndef __b_mode_detailed_test
#define __b_mode_detailed_test

//#include <DopplerBasics/CFDataPhaseAnalyzer.h>
#include <Q:\projects\CommonSources\DopplerBasics\CFDataPhaseAnalyzer.h>
//#include <RFDataImport/S500_CFMFrameSet.h>
#include <Q:\projects\CommonSources\RFDataImport/S500_CFMFrameSet.h>

XRAD_BEGIN

void	BModeDetailedTest(S500_CFMFrameSet& frames);
void	EstimateRiceMM24(const RealFunctionF64& sample, double& nu, double& sigma);
RealFunctionF64	TheoreticalRiceFunction(double A, double sigma, double x0, double dx, size_t n);
//void	BModeDetailedTestSyntheticAperture(ComplexFunction2D_F32 frame);

XRAD_END

#endif //__b_mode_detailed_test
