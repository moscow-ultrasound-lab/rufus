#include "Pre.h"
#include "SignalProcessing/_rare_/AdaptiveFocusing/ExactPathfinder.h"

XRAD_BEGIN


Exact_Pathfinder :: Exact_Pathfinder():Pathfinder_Base(){}

Exact_Pathfinder :: ~Exact_Pathfinder(){}

void	Exact_Pathfinder :: InitWork()
	{
	delta_Sample = n_samples;
	Pathfinder_Base :: InitWork();
	strcpy(SIMIO::Process_Name,"Exact_Pathfinder");
	Interp_Buffer.realloc(FFT_Length, Path_Len);
	Display("Read-in signal");
//	focus_Algorithm = EMULATED_PATH;
	SetOutputFileName(Signal_IP_File_Name, "СEMU");
	ISignal_Write();
	}

void	Exact_Pathfinder :: EndWork()
	{
	Write_Data();
	Display("Processed signal");
	}

void	Exact_Pathfinder :: Batch()
	{
	Process_Group();
	}


void	Exact_Pathfinder :: Process_Group()
	{
	size_t	i;
	size_t	Sample, Harmonic;
	float	deltaOmega;
	ComplexFunctionF32	Phasors(FFT_Length);
	
	//Visible = 0;
	
	Interpolate();		
	//SetDepthUnits(CENTIMETRES);
//	SetFrequencyUnits(BACK_SECONDS);
	
	dZ = depth_range().cm()/n_samples;
	deltaOmega = sample_rate.rad_sec()/FFT_Length;
	
	for(i = 0; i < FFT_Length; i++)
		{
		float	Phase = 2.*deltaOmega*i*dZ/sound_speed.cm_sec();
		Phasors[i] = polar(1, -Phase);
		}
	GUIProgressBar	progress;
	progress.start("Processing paths", n_samples);
	for(Sample = 0; Sample < n_samples; Sample ++)
		{
		ProcessSample(Sample);
		for(Harmonic = 0; Harmonic < FFT_Length; Harmonic ++)
			{
			for(i = 0; i < Path_Len; i++)
				{
				Interp_Buffer.at(Harmonic,i) *= Phasors[Harmonic];
				}
			}
		++progress;
		}
	progress.end();
	}


	
void	Exact_Pathfinder :: ProcessSample(size_t Sample)
	{
	size_t	Harmonic;
	
	float	dOMEGA;
	float	cosFI;
		
//	float	Ro_1 = fabs(Ro_Index+1);
	ptrdiff_t	Bound = (ptrdiff_t(Path_Len) - n_rays)/2;
	size_t	point;
	ComplexFunctionF32 FFT_Buffer(Path_Len);
	

	//SetAngleUnits(RADIANS);
	//SetDepthUnits(CENTIMETRES);
//	SetFrequencyUnits(BACK_SECONDS);
	
	dAngle = angle_range().radians()/n_rays;
	
	cosFI = cos(CentreAngle().radians());
	
	dZ = depth_range().cm()/n_samples;
	R0 = (r_min().cm() + dZ*Sample);

	if(FX_Point) R_Equivalent = R0*FX_Point/(FX_Point - R0);
	else	R_Equivalent = R0;
	
	dX = R0 * dAngle;
	a = array_Pitch.cm() * n_elements / (2.*cosFI);
	dOMEGA = (two_pi()/dX)/Path_Len;
	Scale_Factor = Find_Scale_Factor();
	
	size_t	sub = 0;
	
	for(size_t ray = 0; ray < n_rays; ++ray)
		{
		ComplexFunctionF32	CurrentRay;
		focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
//		SetCurrentRay(ray);
		ForceUpdateGUI();
		for(Harmonic = 0; Harmonic < FFT_Length; Harmonic ++)
			{
			float	Frequency = sample_rate.rad_sec()*Harmonic/FFT_Length;
			float	wave_k = Frequency/sound_speed.cm_sec();
			
			Ro = - (Ro_Index + 0.5) * 2.* wave_k/R0;
			
			if(wave_k)Zeta = -R0*R0/(8.*R_Equivalent*wave_k*(Ro_Index+1.));
			else Zeta = 0;
			
			if(wave_k)alpha = 0.5*(R0/wave_k)*(R0/wave_k)/(a*a*2.*(Ro_Index+1.));
			else	alpha = 0;				
			Find_Fresnel(dOMEGA);
			
			for(point = 0; point < Path_Len; point ++)
				{
				float	OMEGA = dOMEGA * (point - Path_Len/2);
					//- Receptor_X*wave_k/R0;
				complexF32 buffer;
//				float	old_Abs = cabs(FFT_Buffer[point]);
				float	factor = real(Fresnel_Buffer[point]);
				float	phase = Zeta*OMEGA*OMEGA + imag(Fresnel_Buffer[point]);
				
				FFT_Buffer[point] += Interp_Buffer.at(Harmonic,point) * polar(factor,phase);
				}
			}
		}
	FFT_Buffer.roll_half(true);
	FFT(FFT_Buffer, ftReverse);
		
	for(size_t ray = 0; ray < n_rays; ++ray)
		{
		ComplexFunctionF32	CurrentRay;
		focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
//		SetCurrentRay(ray);
		point = ray + Bound;
		CurrentRay[Sample] = FFT_Buffer[point];
		}
	}
	

void	Exact_Pathfinder :: Interpolate()
	{
	size_t	Path, Harmonic, Sample, point, n_Points = n_rays;
//	int	centre_Point = n_Points/2;
	ptrdiff_t	Bound = (Path_Len - n_rays)/2;//positive
	
	ComplexFunctionF32 FFT_Buffer(FFT_Length);
	
	size_t	sub = 0;
		
	for(point = 0; point < n_Points; point ++)
		{
		float	angle;
//		SetCurrentRay(point);
		ComplexFunctionF32	CurrentRay;
		focused_data.GetRow(CurrentRay, {sub, point, slice_mask(0)});
		
//		SetFrequencyUnits(BACK_SECONDS);
		//SetAngleUnits(RADIANS);
		
		angle = (CentreAngle() - CurrentRayAngle(point)).radians();
		
		for(Path = 0;Path < FFT_Length; Path ++)
			Interp_Buffer.at(Path,point) = 0;
		for(Sample = 0; Sample < n_samples; Sample ++)
			Interp_Buffer.at(Sample,point) = CurrentRay[Sample];
		
		for(Path = 0; Path < FFT_Length; Path ++)
			FFT_Buffer[Path] = Interp_Buffer.at(Path,point);

		FFT(FFT_Buffer, ftForward);
		for(Harmonic = 0; Harmonic < FFT_Length; Harmonic ++)
			Interp_Buffer.at(Harmonic,point) = FFT_Buffer[Harmonic];
		}
	FFT_Buffer.resize(Path_Len);
	for(Harmonic = 0; Harmonic < FFT_Length; Harmonic ++)
		{
		for(point = 0; point < Path_Len; point ++) FFT_Buffer[point] = 0;
		for(point = 0; point < n_Points; point ++)
			{
			FFT_Buffer[point + Bound] = Interp_Buffer.at(Harmonic,point);
			}
		FFT(FFT_Buffer, ftForward);
		FFT_Buffer.roll_half(true);
		for(point = 0; point < Path_Len; point ++)
			Interp_Buffer.at(Harmonic,point) = FFT_Buffer[point];

		}
	}

	
XRAD_END
