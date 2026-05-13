#ifndef __b_mode_detailed_test
#define __b_mode_detailed_test


//#include <Q:\projects\CommonSources\DopplerBasics\CFDataPhaseAnalyzer.h>

#include <Q:\projects\CommonSources\RFDataImport/S500_CFMFrameSet.h>

XRAD_BEGIN

//void	BModeDetailedTest(S500_CFMFrameSet& frames);
//void	EstimateRiceMM24(const RealFunctionF64& sample, double& nu, double& sigma);
//RealFunctionF64	TheoreticalRiceFunction(double A, double sigma, double x0, double dx, size_t n);
void BModeRiceTestSyntheticAperture(ComplexFunctionMD_F32 original_frames, ScanConverterOptions sco);
void GetSlidingWindowParams(double& a_threthold, size_t& sliding_window_length_in_beams, size_t& sliding_window_length_in_samples, size_t& step_in_beams, size_t& step_in_samples, ComplexFunction2D_F32 slicet);
void CalcRiceParams(ComplexFunctionMD_F32 b_frames, size_t n_frames, RealFunctionMD_F32& a_frames, RealFunctionMD_F32& sigma_frames, RealFunctionMD_F32& a_sigma_frames, double& mean_nu, size_t v_size, size_t h_size, size_t last_beam_no, size_t last_sample_no, size_t step_in_beams, size_t step_in_samples, size_t length_in_beams, size_t length_in_samples);
void ApplyThreshold(size_t n_frames, RealFunctionMD_F32 a_frames, RealFunctionMD_F32& a_sigma_frames, double a_threthold, double mean_nu, size_t v_size, size_t h_size);
void CalcSampleForWindowIter(ComplexFunction2D_F32& region, RealFunctionF64& sample, ComplexFunction2D_F32 slicet, size_t first_beam_no, size_t first_sample_no);
void EstimateRiceMM24(const RealFunctionF64& sample, double& nu, double& sigma);
RealFunctionF64	TheoreticalRiceFunction(double A, double sigma, double x0, double dx, size_t n);
void			DisplaceSpectrum(size_t samples, ComplexFunctionMD_F32& images);


XRAD_END

#endif //__b_mode_detailed_test
