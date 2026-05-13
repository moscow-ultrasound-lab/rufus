#ifndef	__direct_phase_estimation_h
#define	__direct_phase_estimation_h

#include "SyntheticFocusingOptions.h"
#include "LUT_8bit_interpolator.h"
#include "SyntheticApertureFocuser.h"
#include "RawSFDataSourceTransEcho.h"
//#include "ApertureFocusingOptionsOld.h"

XRAD_BEGIN

ComplexFunctionF64	InterpolateComplexFunctionF64(size_t new_length, ComplexFunctionF64 c_function);
RealFunctionF64		InterpolateRealFunctionF64(size_t new_length, RealFunctionF64 function);
void				RearrangeLines(ComplexFunction2D_F64 &lines, size_t number_of_rays, size_t n_lines);
RealFunctionF64		PhaseCompute(ComplexFunction2D_F64 lines, size_t number_of_rays, size_t n_lines, size_t ray_no);
RealFunctionF64		PhaseCut(GraphSet &GS, size_t number_of_rays, physical_angle scanning_sector, physical_speed sound_speed, physical_frequency f0, physical_length aperture_width, RealFunctionF64 phase, int n_elements);

XRAD_END

#endif