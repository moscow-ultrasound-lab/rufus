#include "Pre.h"
#include "SignalProcessing/_rare_/AdaptiveFocusing/Pathfinder.h"

XRAD_BEGIN

Pathfinder :: Pathfinder():Pathfinder_Base(){}
Pathfinder :: ~Pathfinder(){}


void	Pathfinder :: InitWork()
	{
	Pathfinder_Base :: InitWork();
	Interp_Buffer.realloc(FFT_Length, n_rays);
	TX_Focus.realloc(0);
	RX_Focus.realloc(0);
	Display("Read-in signal");
//	focus_Algorithm = EMULATED_PATH;
	SetOutputFileName(Signal_IP_File_Name, "СEMU");
	ISignal_Write();
	}

void	Pathfinder :: EndWork()
	{
	Write_Data();
	Display("Processed signal");
	}

void	Pathfinder :: Batch()
	{
	GUIProgressBar	progress;
	progress.start("Processing paths . . .", n_intervals*FFT_Length);

	Process_Group();
	progress.end();
	}

void	Pathfinder :: Process_Group()
	{
	size_t	Interval;
	for(Interval = 0; Interval < n_intervals; Interval ++)
		{
		From_Sample = Interval * delta_Sample;
		To_Sample = From_Sample + delta_Sample;
		if(To_Sample > n_samples) To_Sample = n_samples;
		
		Interpolate();		
		Process_Interval();
		ReInterpolate();
		}
	}
	
void	Pathfinder :: Process_Interval()
	{
	size_t	Path;
	size_t	Interp_FT_Len = Path_Len;
	
	float	dOMEGA;
	float	cosFI;
		
	float	Ro_1 = fabs(Ro_Index+1);
	float	R1;
	
	//complexF32 *Interp_FT_Buffer;
	//complexF32 *FFT_Buffer;

	//SetAngleUnits(RADIANS);
	//SetDepthUnits(CENTIMETRES);
//	SetFrequencyUnits(BACK_SECONDS);
	

	dAngle = angle_range().radians()/n_rays;
	
	cosFI = cos(CentreAngle().radians());
	
	dZ = depth_range().cm()/n_samples;
	R0 = (r_min().cm() + dZ*From_Sample);
	
	R1 = R0 + dZ*delta_Sample/2;

	if(FX_Point) R_Equivalent = R1*FX_Point/(FX_Point - R1);
	else	R_Equivalent = R1;
	
	dX = R0 * dAngle;
	//a = array_Pitch * n_elements / (2.*cosFI);
	a = array_Pitch.cm() * n_elements/2.;
	
	dOMEGA = (two_pi()/dX)/Path_Len;
	
	Scale_Factor = Find_Scale_Factor();

	if(Ro_1 >= 1)
		{
		Interp_FT_Len *= 2;
		Scale_Factor /= 2;
		}
	if(Ro_1 >= 2)
		{
		Interp_FT_Len *= 2;
		Scale_Factor /= 2;
		}
	if(Ro_1 >= 4)
		{
		Interp_FT_Len *= 2;
		Scale_Factor /= 2;
		}
	
	ComplexFunctionF32	Interp_FT_Buffer(Interp_FT_Len);
	ComplexFunctionF32	FFT_Buffer(Path_Len);

	ComplexFunction2D_F32	focusing_factors(FFT_Length, Path_Len);
	ComplexFunction2D_F32	filter1(FFT_Length, Path_Len), filter2(FFT_Length, Path_Len);


	for(Path = 0; Path < FFT_Length; Path ++)
		{
		float	Frequency = sample_rate.rad_sec()*float(Path) / FFT_Length;		
		float	wave_k = Frequency/sound_speed.cm_sec();
		
		Ro = - (Ro_Index + 0.5) * 2.* wave_k/R0;
		
		if(wave_k)Zeta = -R0*R0/(8.*R_Equivalent*wave_k*(Ro_Index+1.));
		else Zeta = 0;
				
		if(wave_k)alpha = 0.5*(R0/wave_k)*(R0/wave_k)/(a*a*2.*(Ro_Index+1.));
		else	alpha = 0;
					
		Find_Fresnel(dOMEGA);

		for(size_t point = 0; point < Path_Len; point ++)
			{
			float	OMEGA = dOMEGA * (point - Path_Len/2)-
				Receptor_X*wave_k/R0;
			complexF32 buffer;
			float	factor = real(Fresnel_Buffer[point]);
			float	factor_phase = 
				// 2 for RX only focusing
				// 2*
				Zeta*OMEGA*OMEGA + imag(Fresnel_Buffer[point]);
			
			focusing_factors.at(Path,point) = polar(factor, factor_phase);
			}
		}
	for(Path = 0; Path < FFT_Length; Path ++)
		{
		float	y = 2*float(Path-FFT_Length/2)/FFT_Length;
		for(size_t point = 0; point < Path_Len; point ++)
			{
			float	x = 2*float(point-Path_Len/2)/Path_Len;
			filter1.at(Path,point) = (1-gauss(x,0.25)) * (1-gauss(y, 0.5));
			filter2.at(Path,point) = gauss(x,0.1);// - gauss(x,0.01);
			}
		}
	
	
	static	bool	do_noise_suppress = YesOrNo("Do noise suppression?", true);
	if(do_noise_suppress)
		{
		focusing_factors.roll_half(true);
		focusing_factors *= filter1;
		{do_once DisplayMathFunction2D(focusing_factors, "Focusing factors");}
		FFTf(focusing_factors, fftFwdRollAfter, fftFwdRollAfter);//2016_09_16
		focusing_factors *= filter2;
		{do_once DisplayMathFunction2D(focusing_factors, "Focusing factors FFT");}
		FFTf(focusing_factors, fftRevRollBoth, fftRevRollBoth);//2016_09_16
		{do_once DisplayMathFunction2D(focusing_factors, "Focusing factors total");}
		}
	for(Path = 0; Path < FFT_Length; Path ++)
		{
		size_t	point, centre_point;
		size_t	Bound;
		
		float	Frequency = sample_rate.rad_sec()*float(Path) / FFT_Length;
		
		float	wave_k = Frequency/sound_speed.cm_sec();
		centre_point = n_rays/2;
		Bound = (Path_Len - n_rays)/2.;
		
		Ro = - (Ro_Index + 0.5) * 2.* wave_k/R0;
										
		for(point = 0; point < Path_Len; point ++) FFT_Buffer[point] = 0;
		for(point = 0; point < Interp_FT_Len; point ++) Interp_FT_Buffer[point] = 0;
		for(point = 0; point < n_rays; point ++)
			{
			float	x0 = dX * (point - centre_point);
			FFT_Buffer[point + Bound] = Interp_Buffer.at(Path,point) * polar(1,x0*x0*Ro);
			}


		FFTf(FFT_Buffer, fftFwdRollBoth);
		FFT_Buffer *= focusing_factors.row(Path);
		FFT_Buffer.roll_half(true);
		
		for(point = 0; point < Path_Len/2; point ++) Interp_FT_Buffer[point] = FFT_Buffer[point];
		for(point = 1; point < Path_Len/2; point ++) Interp_FT_Buffer[Interp_FT_Len - point] = FFT_Buffer[Path_Len - point];
		
		FFT(Interp_FT_Buffer, ftReverse);
		
		for(point = 0; point < Path_Len/2; point ++) FFT_Buffer[point] = Interp_FT_Buffer[point];
		for(point = 1; point <= Path_Len/2; point ++) FFT_Buffer[Path_Len - point] = Interp_FT_Buffer[Interp_FT_Len - point];
		
		FFT_Buffer.roll_half(true);
				
		for(point = 0; point < n_rays; point ++)
			{
			size_t	n = Path_Len/2 + (point + Bound - Path_Len/2)/Scale_Factor;
			if(n >= 0 && n < Path_Len) Interp_Buffer.at(Path,point) = FFT_Buffer[n];
			else Interp_Buffer.at(Path,point) = 0;
			}
//		++progress;
		}
	}
	
	
#define	fwd	ftReverse
#define rev	ftForward

void	Pathfinder :: Interpolate()
	{
	size_t	path, harmonic, point, n_points = n_rays;
//	int	centre_Point = n_Points/2;
	ComplexFunctionF32 FFT_Buffer(FFT_Length);
	
//	SetFrequencyUnits(M_HERZ);
	//SetAngleUnits(RADIANS);
	//SetDepthUnits(CENTIMETRES);
	
	size_t	sub = 0;

	for(point = 0; point < n_points; point ++)
		{
		float	angle;
//		SetCurrentRay(point);
		ComplexFunctionF32	CurrentRay;
		focused_data.GetRow(CurrentRay, {sub, point, slice_mask(0)});

//		SetFrequencyUnits(BACK_SECONDS);
		//SetAngleUnits(RADIANS);
		angle = (CentreAngle() - CurrentRayAngle(point)).radians();

		for(path = 0, point = From_Sample; point < n_samples && path < FFT_Length; path ++, point ++)
			Interp_Buffer.at(path,point) = CurrentRay[point];
		for(;path < FFT_Length; path ++)
			Interp_Buffer.at(path,point) = 0;
		
		for(path = 0; path < FFT_Length; path ++)
			{
			FFT_Buffer[path] = Interp_Buffer.at(path,point);
			}
		//FFT_Buffer.Display(0, 1, "Interp-1");
		FFT(FFT_Buffer, fwd);
		//FFT_Buffer.Display(0, 1, "Interp-2");
		for(harmonic = 0; harmonic < FFT_Length; harmonic ++)
			{
			Interp_Buffer.at(harmonic,point) = FFT_Buffer[harmonic];
			}
		}
	}

void	Pathfinder :: ReInterpolate()
	{
	size_t	Sample, Path, Harmonic;
//	int	centre_Point = n_rays/2;
	ComplexFunctionF32 FFT_Buffer(FFT_Length);

	//SetAngleUnits(RADIANS);
	//SetDepthUnits(CENTIMETRES);
	dZ = depth_range().cm()/n_samples;
	
	size_t	sub = 0;

	for(size_t ray = 0; ray < n_rays; ++ray)
		{
//		SetCurrentRay(ray);
		ComplexFunctionF32	CurrentRay;
		focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
		size_t	point = ray;
		float	angle;
		float	deltaFi; 
	
//		SetFrequencyUnits(BACK_SECONDS);
		//SetAngleUnits(RADIANS);
		angle = (CentreAngle() - CurrentRayAngle(ray)).radians();
	
		deltaFi = Find_Schift()*
			sample_rate.rad_sec() * (r_min().cm() + dZ*From_Sample)*
			(1.-1./cos(angle))/(sound_speed.cm_sec()*FFT_Length);		
		
		for(Harmonic = 0; Harmonic < FFT_Length; Harmonic ++)
			{
			float	Fi = (float)Harmonic*deltaFi;
			FFT_Buffer[Harmonic] = (complexF32)Interp_Buffer.at(Harmonic,point)*polar(1,Fi);
			}
		FFT(FFT_Buffer, rev);
		for(Path = 0, Sample = From_Sample; Sample < To_Sample; Path++, Sample++)
			{
			CurrentRay[Sample] = FFT_Buffer[Path];
			}		
		}
	}

XRAD_END
