#include "pre.h"
#include "NoiseTwister.h"

XRAD_BEGIN

void	NoiseTwister :: TwistNoise()
	{
//	int	i;
	
	ComplexFunction2D_F32	buffer(n_rays, n_samples);

	size_t	sub = 0;
	for(size_t ray = 0; ray < n_rays; ++ray)
		{
		ComplexFunctionF32	CurrentRay;
		focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
//		SetCurrentRay(ray);
		buffer.row(ray).CopyData(CurrentRay);
		FFT(buffer.row(ray), ftForward);
		}
	for(size_t i = 0; i < n_samples; i++) FFTf(buffer.col(i), fftFwdRollBoth);
	
	DisplayMathFunction2D(buffer, "Spectrum BEFORE");


	for(size_t ray = 0; ray < n_rays; ++ray)
		{
		ComplexFunctionF32	CurrentRay;
		focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
//		SetCurrentRay(ray);
		buffer.row(ray).CopyData(CurrentRay);
		}

	//SetDepthUnits(CENTIMETRES);
	//SetAngleUnits(RADIANS);
	float	angle0 = (end_angle() + start_angle()).radians()/2;

	float	angle_strob = pi()/8;//+-
	
	float	z = (r_max()+r_min()).cm()/2;
	float	factor = 1.3*(TX_Focus[0].cm()-z)/z;
	
	factor *= fabs(factor);
	
	while(YesOrNo("Do rotation?", true))
		{
		for(size_t ray = 0; ray < n_rays; ++ray)
			{
			ComplexFunctionF32	CurrentRay;
			focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
//			SetCurrentRay(ray);
			CurrentRay.CopyData(buffer.row(ray));
			}
		factor = GetFloating("Rotation factor", factor, -HUGE_VAL, HUGE_VAL);
		for(size_t ray = 0; ray < n_rays; ++ray)
			{
			ComplexFunctionF32	CurrentRay;
			focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
//			SetCurrentRay(ray);
			FFT(CurrentRay, ftForward);

			//SetAngleUnits(RADIANS);
			//SetDepthUnits(CENTIMETRES);
//			SetFrequencyUnits(BACK_SECONDS);
			
			float	angle = CurrentRayAngle(ray).radians() - angle0;
			float	dz = factor*(1.-cos(angle))*z;
			float	dw = sample_rate.rad_sec()/n_samples;
			
			float	rot_factor = dw*dz/sound_speed.cm_sec();

			if(CapsLock())
				{
				printf("\nray = %lu, angle = %g, dz = %g, dw = %g, rot_factor = %g", int(ray), angle, dz, dw, rot_factor);
				fflush(stdout);
				}
			
			
			if(fabs(angle) < angle_strob)
				{
				float	strob = (1 + cos(pi()*angle/angle_strob))/2;
//				for(size_t i = 0; i < n_samples; i++) CurrentRay[i] *= polar(strob, float(i)*rot_factor);
				for(size_t i = 0; i < n_samples; i++) CurrentRay[i] *= polar(strob, float(n_samples/2)*rot_factor);
				FFT(CurrentRay, ftReverse);
				}
			else CurrentRay.fill(complexF32(0));
			}
		
		Display("Rotated");
		}
	for(size_t ray = 0; ray < n_rays; ++ray)
		{
//		SetCurrentRay(ray);
		ComplexFunctionF32	CurrentRay;
		focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
		buffer.row(ray).CopyData(CurrentRay);
		FFT(buffer.row(ray), ftForward);
		}
	for(size_t i = 0; i < n_samples; i++) FFTf(buffer.col(i), fftFwdRollBoth);
	DisplayMathFunction2D(buffer,"Twisted spectrum");
	
//----------------------------------
//
//	"here suppress noise"
//
	ComplexFunction2D_F32	mask(n_rays, n_samples);
	mask.fill(complexF32(1));
//	int	j;
	double	alpha1 = 0.05;
	double	beta = 0.7;
	for(size_t i = 0; i < n_rays; i++)
		{
		float	v = float(i)/n_rays;
		for(size_t j = 0; j < n_samples; j++)
			{
			float	h = float(j)/n_samples;
			if(v+h > 1.+alpha1 && v > h+alpha1) mask.at(i,j) = 0;
			if(v+h < 1.-alpha1 && v < h-alpha1) mask.at(i,j) = 0;
			
			if(v+h < beta || v+h > 2-beta) mask.at(i,j) = 0;
			if(v-h > 1-beta || h-v > 1-beta) mask.at(i,j) = 0;
			}
		}
	for(size_t i = 0; i < n_rays; i++) mask.row(i).FilterGauss(alpha1*n_samples/2);
	for(size_t j = 0; j < n_samples; j++) mask.col(j).FilterGauss(alpha1*n_rays/2);
	
	DisplayMathFunction2D(mask,"Mask");
	buffer *= mask;
	DisplayMathFunction2D(buffer, "Twisted&noise suppressed");

//----------------------------------



	for(size_t i = 0; i < n_samples; i++) FFTf(buffer.col(i), fftRevRollBoth);

	if(1)
		{
		for(size_t ray = 0; ray < n_rays; ++ray)
			{
			ComplexFunctionF32	CurrentRay;
			focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
//			SetCurrentRay(ray);
			CurrentRay.CopyData(buffer.row(ray));
			}
		for(size_t ray = 0; ray < n_rays; ++ray)
			{
			ComplexFunctionF32	CurrentRay;
			focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
//			SetCurrentRay(ray);
			//SetAngleUnits(RADIANS);
			//SetDepthUnits(CENTIMETRES);
//			SetFrequencyUnits(BACK_SECONDS);
			
			float	angle = CurrentRayAngle(ray).radians() - angle0;
			float	dz = factor*(1.-cos(angle))*z;
			float	dw = sample_rate.rad_sec()/n_samples;
			
			float	rot_factor = dw*dz/sound_speed.cm_sec();			
			
			
//			for(size_t i = 0; i < n_samples; i++) CurrentRay[i] *= polar(1, -float(i)*rot_factor);
			for(size_t i = 0; i < n_samples; i++) CurrentRay[i] *= polar(1, -float(n_samples/2)*rot_factor);
			FFT(CurrentRay, ftReverse);
			}
		
		Display("Rotated back");
		}

	}

void	NoiseTwister :: Batch()
	{
	if(YesOrNo("Twist noise?", true))TwistNoise();
	Process_Group(); 
	}
	
XRAD_END