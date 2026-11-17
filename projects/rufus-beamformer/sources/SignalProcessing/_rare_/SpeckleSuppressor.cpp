#include "Pre.h"
#include "SpeckleSuppressor.h"

XRAD_BEGIN

int	Speckle_Suppressor :: Speckle_Suppressor_Count = 0;	
int	Speckle_Suppressor :: n_Cases = 0;	
int	Speckle_Suppressor :: Use_Array_Factor = 0;


Speckle_Suppressor :: Speckle_Suppressor()
	{
	char	message[256];
	float	Ro_1;

	Personal_No = Speckle_Suppressor_Count;
	sprintf(message, "%s, additional case %lu", "Ro index", Personal_No);
	if(!Speckle_Suppressor_Count)
		{
		strcpy(SIMIO::Process_Name,"Speckle-Suppressor");
		n_Cases = GetSigned("N additional cases to process", 0, 0, 7);		
		}
	else switch(Personal_No)
		{
		case 0:	Ro_Index = GetFloating(message, -0.5, -2, 1);
			break;
		case 1:	Ro_Index = GetFloating(message, 0, -2, 1);
			break;
		case 2:	Ro_Index = GetFloating(message, -0.75, -2, 1);
			break;
		case 3:	Ro_Index = GetFloating(message, 0.5, -2, 1);
			break;
		case 4:	Ro_Index = GetFloating(message, -1.25, -2, 1);
			break;
		case 5:	Ro_Index = GetFloating(message, -0.875, -2, 1);
			break;
		case 6:	Ro_Index = GetFloating(message, -1.5, -2, 1);
			break;
		default:Ro_Index = GetFloating(message, -.5, -2, 1);
			break;
		}

	Speckle_Suppressor_Count ++;		
	Interp_Buffer.realloc(FFT_Length, n_rays);
	if(n_rays > 256) FatalError("Too many rays per sector");

	if(n_rays <= 256) Path_Len = 256;
	if(n_rays <= 128) Path_Len = 128;
	if(n_rays <= 64) Path_Len = 64;
	
	Ro_1 = fabs(Ro_Index + 1);
		
	if(Ro_1 < 1) Path_Len *= 2;
	if(Ro_1 < .5) Path_Len *= 2;
	if(Ro_1 < .25) Path_Len *= 2;
	if(Personal_No < n_Cases) NextCase = new Speckle_Suppressor;
	}

Speckle_Suppressor :: ~Speckle_Suppressor()
	{
	if(NextCase) delete NextCase;
	Speckle_Suppressor_Count --;
	}

void	Speckle_Suppressor :: init_Work()
	{
	Display("Read-in signal");
//	focus_Algorithm = EMULATED_PATH;
	ISignal_Write();
	}


void	Speckle_Suppressor :: end_Work()
	{
	Write_Data();
	Display("Processed signal");
	}

void	Speckle_Suppressor :: Batch()
	{
	GUIProgressBar	progress;
	progress.start("Processing paths...",
		(n_Cases + 1)*FFT_Length);
	Process_Group();
	progress.end();
	}
	
void	Speckle_Suppressor :: Process_Interval()
	{
	inherited :: Process_Interval();
	if(Personal_No < n_Cases) NextCase -> Process_Interval();
	}
	
void	Speckle_Suppressor :: Interpolate()
	{
	inherited :: Interpolate();
	if(Personal_No < n_Cases) NextCase -> Interpolate();		
	}

void	Speckle_Suppressor :: ReInterpolate()
	{
//	size_t	Sample, Path, Harmonic;
//	int	centre_Point = n_rays/2;
	ComplexFunctionF32 FFT_Buffer(FFT_Length);
	
	//SetAngleUnits(RADIANS);
	//SetDepthUnits(CENTIMETRES);
	dZ = depth_range().cm()/n_samples;
	size_t	sub = 0;
	for(size_t	ray = 0; ray < n_rays; ++ray)
		{
		ComplexFunctionF32	CurrentRay;
		focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
//		SetCurrentRay(ray);
		int	point = ray;
		float	angle;
		float	deltaFi; 
	
//		SetFrequencyUnits(BACK_SECONDS);
		//SetAngleUnits(RADIANS);
		angle = (CentreAngle() - CurrentRayAngle(ray)).radians();

		deltaFi = Find_Schift()*
			sample_rate.rad_sec() * (r_min().cm() + dZ*From_Sample)*
			(1.-1/cos(angle))/(sound_speed.cm_sec()*FFT_Length);		
		
		for(size_t Harmonic = 0; Harmonic < FFT_Length; Harmonic ++)
			{
			float	Fi = (float)Harmonic*deltaFi;
			FFT_Buffer[Harmonic] = Interp_Buffer.at(Harmonic,point)*polar(1, Fi);
			}
			
		FFT(FFT_Buffer, ftReverse);
		for(size_t Path = 0, Sample = From_Sample; Sample < To_Sample; Path++, Sample++)
			{
			if(n_Cases > 1)
				{
				if(Personal_No)
					{
					CurrentRay[Sample] += cabs(FFT_Buffer[Path]);
					}
				else
					{
					CurrentRay[Sample] = cabs(FFT_Buffer[Path]);
					}
				}
			else
				{
				CurrentRay[Sample] = FFT_Buffer[Path];
				}
			}		
		}
	if(Personal_No < n_Cases) NextCase -> ReInterpolate();
	}
	
XRAD_END