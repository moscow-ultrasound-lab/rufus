#ifndef _calc_true_false_rates
#define _calc_true_false_rates

XRAD_BEGIN
/*
void	DisplayDopplerDataTrueFalseDetectionRates(
	const RealFunctionMD_F32& cfm_phase_dispersion,
	const RealFunctionMD_F32 &correlation_frames,
	const RealFunctionMD_F32& correlation_re_im_frames,
	const RealFunctionMD_F32 &std_frames,
	const RealFunctionMD_F32 &amplitude_frames,
	const S500_CFMFrameSet &frames);
*/
void	DisplayDopplerDataTrueFalseDetectionRates(
	const RealFunctionMD_F32& cfm_phase_dispersion,
	const RealFunctionMD_F32& correlation_frames,
	const RealFunctionMD_F32& correlation_re_im_frames,
	const RealFunctionMD_F32& std_frames,
	const RealFunctionMD_F32& amplitude_frames,
	const S500_CFMFrameSet& frames);


XRAD_END

#endif //_calc_true_false_rates