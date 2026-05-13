#include "Pre.h"
#include "PathfinderBase.h"

XRAD_BEGIN


size_t	Pathfinder_Base :: Pathfinder_Count = 0;

size_t	Pathfinder_Base :: FFT_Length;
size_t	Pathfinder_Base :: From_Sample;
size_t	Pathfinder_Base :: To_Sample;
size_t	Pathfinder_Base :: delta_Sample;

float	Pathfinder_Base :: Receptor_X;
float	Pathfinder_Base :: FX_Point;



void	Pathfinder_Base :: ProcessInitDialog()
	{
#if 0	
	short	answer = 0;
	qmMacDialog	theDialog(2000);

	theDialog.SetItemValue(3, delta_Sample);
	theDialog.SetItemValue(6, Ro_Index);
	theDialog.SetItemValue(8, FX_Point);
	
	//theDialog.Set_Control_Value(2, Use_Emulated_Array);
	theDialog.HideItem(2);
	
	theDialog.Set_Control_Value(7, true/*Visible*/);
	theDialog.Set_Control_Value(13, Do_Fresnel_Correction);
	
	theDialog.HideItem(10); // 'group overlapping'
	theDialog.HideItem(12);

	
	
	if(FX_Point)
		{
		theDialog.ShowItem(8);
		theDialog.ShowItem(9);
		}
	else
		{
		theDialog.HideItem(8);
		theDialog.HideItem(9);
		}
	
	theDialog.Show();
	do
		{
		short	prevAnswer;
		//ModalDialog(0L, &answer);
		short	realEvent = theDialog.ModelessDialog(&answer);
		if(realEvent)
			{
			if(answer != prevAnswer || theDialog.SpecNumItem(answer))
				{
				delta_Sample = theDialog.NumItemRange(3, (float)delta_Sample, 2, n_samples);
				Ro_Index = theDialog.NumItemRange(6, Ro_Index, -2.0, 1.0);
				if(FX_Point) FX_Point = theDialog.NumItemRange(8, FX_Point, 0, 1e10);
				theDialog.SelectItemText(answer);
				}
			switch(answer)
				{
				case	2:
					//Use_Emulated_Array = !theDialog.Get_Control_Value(answer);
					//theDialog.Set_Control_Value(answer, Use_Emulated_Array);
					// this checkbox is now reserved
					break;
				case	7:
					//Visible = !theDialog.Get_Control_Value(answer);
					//theDialog.Set_Control_Value(answer, Visible);
					break;
				case	10: // 'group overlapping'
					break;
				case	13:
					Do_Fresnel_Correction = !theDialog.Get_Control_Value(answer);
					theDialog.Set_Control_Value(answer, Do_Fresnel_Correction);
					break;
				case	11:
					DebugQuit();
					break;
				}
			prevAnswer = answer;
			}
		}while(answer != 1);
	#endif
	}

Pathfinder_Base :: Pathfinder_Base()
	{
	n_intervals = 0;
	interval_increment = 0;

	/*
	Mu = 1.4;
	side_Lobe = 0.26;
	U_Bound = 20;
	*/
	Do_Fresnel_Correction = false;

	if(!Pathfinder_Count && 1)
		{
		size_t	defaultDS = 64;
		strcpy(SIMIO::Process_Name,"PATHFINDER");
			
		if(n_samples < defaultDS)
			delta_Sample = n_samples;
		else	delta_Sample = defaultDS;
		
		Ro_Index = -0.5;
		//SetDepthUnits(CENTIMETRES);	
		
		if(TX_Focus.size() != 1 || RX_Focus.size() != 1)
			FatalError("Signal is dynamically focused!");
		if(TX_Focus[0] != RX_Focus[0])
			FatalError("Can't focus since TX/RX apertures are focused differently!");
		FX_Point = TX_Focus[0].cm();
		
		}
	Pathfinder_Count ++;
	}

Pathfinder_Base :: ~Pathfinder_Base()
	{
	}


void	Pathfinder_Base :: InitWork()
	{
	ProcessInitDialog();		
	
	//SetDepthUnits(CENTIMETRES);

	// TODO ниже расчет для одноэлементного приемника, это важно для метода
	/*if(n_subaperture_elements_active*n_frames == n_elements)
		Receptor_X = 0;
	else*/	Receptor_X = 0.5*n_elements*array_Pitch.cm();
	
	FFT_Length = ceil_fft_length(delta_Sample);
	if(n_samples % delta_Sample) n_intervals = n_samples/delta_Sample + 1;
	else	n_intervals = n_samples/delta_Sample;
			
	if(n_rays > 256) FatalError("Too many rays per sector");
	
	if(n_rays <= 256) Path_Len = 256;
	if(n_rays <= 128) Path_Len = 128;
	if(n_rays <= 64) Path_Len = 64;
	if(n_rays <= 32) Path_Len = 32;
		
//	float	Ro_1 = fabs(Ro_Index + 1);	
//	if(Ro_1 < 1) Path_Len *= 2;
	
	Fresnel_Buffer.realloc(Path_Len);
	}
	
	
float	Pathfinder_Base :: Find_Scale_Factor()
	{
	if(Ro_Index > -1.0625 && Ro_Index < -0.9375) return 1.;
	return	2.*(Ro_Index + 1);
	}
	
	
float	Pathfinder_Base :: Find_Schift()
	{
	float	Precomputed[] = {6, 4, 1.5, 0., -0.5, -0.5};
	float	SF = Find_Scale_Factor();
	
	float	f_Index = 2*(Ro_Index+2);
	short	index = f_Index;
	
	f_Index -= index;
	
	double a1 = Precomputed[index] + f_Index*(Precomputed[index+1]-Precomputed[index]);
	return a1/SF;
#if 0	
	if(Ro_Index < -0.5 && Ro_Index > -1) return	1./SF;
	if(Ro_Index <= -1 && Ro_Index > -1.5) return	2./SF;
	if(Ro_Index <= -1.5 && Ro_Index >= -2) return	4./SF;
	return -1/(2.*SF);
#endif
	}


void	Pathfinder_Base :: Find_Fresnel(float dOMEGA)
	{
	for(size_t i = 0; i < Path_Len; i ++)
		{
		float	f = sample_rate.rad_sec()*float(i) / Path_Len;
		float	wave_k = f/sound_speed.cm_sec();
		Fresnel_Buffer[i] = Find_Fresnel1(dOMEGA, i, wave_k);
		}
	}
	
complexF32 Pathfinder_Base :: Find_Fresnel1(float dOMEGA, ptrdiff_t point, float wave_k)
	{
	if(Do_Fresnel_Correction)
		{
		complexF32 res(0);
		float	OMEGA = dOMEGA * (point - Path_Len/2) - Receptor_X*wave_k/R0;
		float	OMEGA_r;
		float	K;
		
		complexF32 buffer(0);
		
		if(wave_k) OMEGA_r = OMEGA*R0/wave_k;
		else OMEGA_r = 0;
				
		K = (a - fabs(OMEGA_r)/2)*sqrt(2.*wave_k/fabs(pi()*R_Equivalent));
		if(!is_number(K)) return complexF32(1);
		buffer = xrad::fresnel_integral(K); 
		
		if(cabs(buffer))
			{
			if(2*a - fabs(OMEGA_r) < 0) res = 0;
			else
				{
				imag(res)  = -arg(buffer);
				real(res)  = (a - fabs(OMEGA_r)/2)/cabs(buffer);
				}
			}
		else	res = 0;
		return res;
		}
	
	else return complexF32(1);
	}
	
XRAD_END
