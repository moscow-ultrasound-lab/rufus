#ifndef	__focuser_analyzer_h
#define	__focuser_analyzer_h

#include "SyntheticFocusingOptions.h"
#include "LUT_8bit_interpolator.h"
#include "SyntheticApertureFocuser.h"
#include "RawSFDataSourceTransEcho.h"
#include "DirectPhaseEstimation.h"
#include "PolynomialNarrowing.h"
#include "SyntheticApertureFocuserAina.h"
//#include "ApertureFocusingOptionsOld.h"



XRAD_BEGIN

void				CaclAandSTD(ComplexFunction2D_F32 image, double half_sector);
double			DisplayLines(GraphSet &GS, ComplexFunction2D_F64 lines, size_t number_of_rays, size_t n_lines, size_t iteration_no, physical_angle scanning_sector);
void				SubstractLinearShift(RealFunctionF64 &phase_interpolated);
RealFunctionF32 AddAberration(GraphSet& GS, size_t length, physical_frequency f0);
RealFunctionF64 CalcLine(ComplexFunction2D_F64 lines, size_t number_of_rays, size_t n_lines);

XRAD_END

#endif