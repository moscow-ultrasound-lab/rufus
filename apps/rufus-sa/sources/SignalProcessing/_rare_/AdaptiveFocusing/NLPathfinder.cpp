#include "Pre.h"
#include "SignalProcessing/_rare_/AdaptiveFocusing/NLPathfinder.h"

#define FACTOR 6.5

#pragma message ("This file is now inavilable")

#if 0
static	CS_Operator *crossTermFiltre = NULL;
static	CS_Operator *temporalFiltre = NULL;
static	twoD_Statistic *firstFPS = NULL, *secondFPS = NULL;
static	spcFiltre *SL_Filtre = NULL;



short	NL_Pathfinder :: Pathfinder_Count = 0;
Boolean	NL_Pathfinder :: Use_Frequency_Filtre;
short	NL_Pathfinder :: do_Focusing = 1;

short	NL_Pathfinder :: FFT_Length;
short	NL_Pathfinder :: From_Sample;
short	NL_Pathfinder :: To_Sample;
short	NL_Pathfinder :: delta_Sample;

float	NL_Pathfinder :: Receptor_X;
float	NL_Pathfinder :: FX_Point;


float NL_Pathfinder :: Amplification(float OMEGA)
	{
	float	Re1 = 2.*Mu*OMEGA*sqrt(alpha);	
	float	Re2 = OMEGA*OMEGA*alpha;
	//float	Re = Re1+Re2;
	float	Re = Re1;
	float	result = exp(Re);
	
	if(result > U_Bound || !result) result = 0;	
	
	//if(OMEGA > 1./sqrt(alpha) || !result) result = 0;	
	//if(OMEGA < 0 && fabs(Re1) < fabs(Re2)) result = 0;
	return result;
	}


void	NL_Pathfinder :: ProcessInitDialog()
	{
	short	answer = 0;
	
	qmMacDialog	theDialog(2000);

	theDialog.SetItemValue(3, delta_Sample);
	theDialog.SetItemValue(6, Ro_Index);
	theDialog.MakeItemNEditText(6);
	
	theDialog.SetItemValue(8, FX_Point);
	
	
	theDialog.Set_Control_Value(12, do_Focusing);
	theDialog.HideItem(13);
	theDialog.HideItem(2);
	theDialog.ShowItem(12);
	theDialog.Set_Control_Value(7, Visible);
	
	if(!FX_Point)
		{
		theDialog.HideItem(8);
		theDialog.HideItem(9);
		}
	theDialog.HideItem(10);//'group overlapping'
	theDialog.Show();
	
	do
		{
		short	prevAnswer;
		short	RealEvent = theDialog.modelessDialog(&answer);
		if(answer != prevAnswer || theDialog.SpecNumItem(answer))
			{
			delta_Sample = theDialog.NumItemRange(3, (float)delta_Sample, 2, n_samples);
			Ro_Index = theDialog.NumItemRange(6, Ro_Index, -2.0, 1.0);
			if(FX_Point) FX_Point = theDialog.NumItemRange(8, FX_Point, r_min, 10000);
			theDialog.SelectItemText(answer);
			}
		if(RealEvent)switch(answer)
			{
			case	12:
				do_Focusing = !theDialog.Get_Control_Value(answer);
				theDialog.Set_Control_Value(answer, do_Focusing);
				break;
			case	7:
				Visible = !theDialog.Get_Control_Value(answer);
				theDialog.Set_Control_Value(answer, Visible);
				break;
			case	10: // 'group overlapping'
				break;
			case	11:
				throw canceled_operation("Operation canceled");
				break;
			}
		prevAnswer = answer;
		}while(answer != 1);
	}

NL_Pathfinder :: NL_Pathfinder():()
	{
	int	defaultDS = 64;
	char	message[256];
	float	Ro_1;

	Mu = 1.4;
		
	side_Lobe = 0.26;
	U_Bound = 20;
	
		
	first_SL = second_SL = 0;
	
	if(!strcmp(Data_Genesis, SYNTH_OF_SIM_GENESIS))
		{
		first_SL = 1.54*PI/180.;
		second_SL = 2.75*PI/180.;
		first_SL_Level = 0.47;
		second_SL_Level = -0.15;
		}
	if(!strcmp(Data_Genesis, SYNTH_OF_EXPT_GENESIS))
		{
		first_SL = 1.54*PI/180.;
		second_SL = 3.1*PI/180.;
		first_SL_Level = -0.35;
		}

	strcpy(Process_Name,"NL_Pathfinder");
	
	if(n_elements == 1) Use_Frequency_Filtre = 1;
	else	Use_Frequency_Filtre = 0;
	Intelligent_SLs = 1;
			
	if(n_samples < defaultDS)
		delta_Sample = n_samples;
	else	delta_Sample = defaultDS;
	
	//SetDepthUnits(CENTIMETRES);	
	if(n_elements == 1)
		//FX_Point = GetFloating("Focal point at", 7.5, r_min, r_max);
		//FX_Point = GetFloating("Focal point at", (r_min + r_max)/2, 0, r_max);
		FX_Point = (r_min + r_max)/2;
	else	FX_Point = 0;
	
	FX_Point = 0;
	Receptor_X = 0;
	
	if(TX_Focus < 0 || RX_Focus < 0) do_Focusing = 0;
	else	do_Focusing = 1;
	
	Ro_Index = -0.5;
	ProcessInitDialog();

	if(n_samples%delta_Sample) n_Intervals = n_samples/delta_Sample + 1;
	else n_Intervals = n_samples/delta_Sample;
	FFT_Length = CeilFFTLength(delta_Sample);

	
			
	Interp_Buffer1.realloc(FFT_Length, n_rays);
	Interp_Buffer2.realloc(FFT_Length, n_rays);
	if(n_rays > 256) FatalError("Too many rays per sector");

	if(n_rays <= 256) Path_Len = 256;
	if(n_rays <= 128) Path_Len = 128;
	if(n_rays <= 64) Path_Len = 64;
	
	Ro_1 = fabs(Ro_Index + 1);	
	if(Ro_1 < 0.5) Path_Len *= 2;
	
	FPS_On = 0;
	Cross_Terms = 0;
	second_SLs = 0;
	Intelligent_SLs = 0;

	if(!Get_Checkbox_Decision("How to suppress SLs?", 5,
		"Cross terms", &Cross_Terms,
		"FPS filtre", &FPS_On,
		"Intelligent", &Intelligent_SLs,
		"Frequency filtre", &Use_Frequency_Filtre,
		"2nd SLs", &second_SLs))
		{
		throw canceled_operation("Operation canceled");
		}
	if(Cross_Terms)
		{
		crossTermFiltre = new spcFiltre(this);
		crossTermFiltre -> EditFiltre("Cross terms filtre"); 
		crossTermFiltre->SetMessage("Reducing cross terms");
		}

	Pathfinder_Count ++;
	}
	
NL_Pathfinder :: ~NL_Pathfinder()
	{
	//if(Interp_Buffer1)Delete2D_Complex(&Interp_Buffer1);
	//if(Interp_Buffer2)Delete2D_Complex(&Interp_Buffer2);
	}
	
	
void	NL_Pathfinder :: InitWork()
	{
	Display("Read-in signal");
	focus_Algorithm = EMULATED_PATH;
	
	Set_OP_FN(Signal_IP_File_Name, "СSR");
	ISignal_Write();

	if(FPS_On)
		{
		short	stages = YesOrNo("Two stage FPS?", YES);
		
		firstFPS = (twoD_Statistic *) new twoD_FPS(this);

		if(stages)
			{
			firstFPS -> SetDefaults(15, 15, 3, 1, 1);
			if(!firstFPS -> EditFiltre("First FPS")) DestroyObject(firstFPS);
			else firstFPS -> SetMessage("First FPS");
			
			secondFPS = (twoD_Statistic *) new twoD_FPS(this);
			secondFPS -> SetDefaults(5, 5, 1, 1, 0);
			if(!secondFPS -> EditFiltre("Second FPS")) DestroyObject(secondFPS);
			else secondFPS -> SetMessage("Second FPS");

			}
		else
			{
			firstFPS -> SetDefaults(15, 15, 3, 1, 1);
			if(!firstFPS -> EditFiltre("FPS")) DestroyObject(firstFPS);
			else firstFPS -> SetMessage("FPS");
			}
		}
	}
	
void	NL_Pathfinder :: EndWork()
	{
	Write_Data();
	Display("Processed signal");
	}
	
void	NL_Pathfinder :: Batch()
	{
	
	StartProgress("Processing paths (SResolution):",
		n_Intervals*n_frames*FFT_Length);

	Process_Group();
	EndProgress();
	}
	
void	NL_Pathfinder :: Process_Group()
	{
	short	Interval;
	short	Sample, Subaperture;
	float	p = 2;//GetFloating("Power to suppress SLs", 3, 0, 13);
	short	Do_Power = YesOrNo("Do power?", YES);
	
	for(Interval = 0; Interval < n_Intervals; Interval ++)
		{
		From_Sample = Interval * delta_Sample;
		To_Sample = From_Sample + delta_Sample;
		if(To_Sample > n_samples) To_Sample = n_samples;
		
		for(Subaperture = 0; Subaperture < n_frames; Subaperture ++)
			{
			Interpolate(Subaperture);		
			ProcessBuffer();
			ReInterpolate(Subaperture);
			}
		}
	if(Do_Power)power_Group(1./p);
	//sqrt_Group();
	
	
	if(!Intelligent_SLs){

		SL_Filtre = new spcFiltre(this);
		SL_Filtre -> SetMessage("Side lobes suppressing");
		if(n_elements == 1)
			{
			SL_Filtre -> sideLobeTerm(0.0125, 0);// Hamming for 'Sonoline 3000И data
			}
		else
			{
			SL_Filtre -> sideLobeTerm(0.025, 0.55);	// 1st side lobe			
			SL_Filtre -> sideLobeTerm(0.05, 0.12);	// 2nd side lobe
			}
		SL_Filtre -> FiltreGroup();
		delete SL_Filtre;
		SL_Filtre = NULL;
		}
	else
		{
		
		if(Cross_Terms)
			{
			//Filtre_Across(); // cross terms filtre
			crossTermFiltre -> FiltreGroup();
			}
			
		if(second_SLs)
			{
			Error("—асть методов пока недействительна!");
			/*
			initSL_Filtre(second_SL, second_SL_Level); // 2nd side lobe
			Filtre_Across();
			Dispose_Across_Filtre();
			*/
			}
		if(Intelligent_SLs)Intelligent_Suppress(first_SL, first_SL_Level);		
		}

	if(Do_Power)power_Group(p/2);
	else power_Group(0.5);

	if(firstFPS)
		{
		firstFPS -> FiltreGroup();
		DestroyObject(firstFPS);
		}
	if(secondFPS)
		{
		secondFPS -> FiltreGroup();
		DestroyObject(secondFPS);
		}
	}
	
void	NL_Pathfinder :: ProcessBuffer()
	{
	int	Path;
	int	Interp_FT_Len = Path_Len;
	
	float	a;
	float	dAngle, dX, dZ, dOMEGA;
	float	R_Equivalent;
	float	Ro_1 = fabs(Ro_Index+1);

	//SetAngleUnits(RADIANS);
	//SetDepthUnits(CENTIMETRES);
	SetFrequencyUnits(BACK_SECONDS);
	
	dAngle = (end_Angle - start_Angle)/n_rays;
	
	dZ = (r_max - r_min)/n_samples;
	R0 = (r_min + dZ*From_Sample);
	dX =  R0*dAngle;

	if(FX_Point) R_Equivalent = R0*FX_Point/(FX_Point - R0);
	else	R_Equivalent = R0;
	
	a = array_Pitch * n_elements / 2.;
	
	if(FX_Point) a *= fabs(FX_Point - R0)/FX_Point;
	
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
		
	ComplexFunctionF32	Interp_FT_Buffer1(Interp_FT_Len);
	ComplexFunctionF32	Interp_FT_Buffer2(Interp_FT_Len);
	ComplexFunctionF32	FFT_Buffer1(Path_Len);
	ComplexFunctionF32	FFT_Buffer2(Path_Len);
	
	
	for(Path = 0; Path < FFT_Length; Path ++)
	{
	static	int	Side;
		float	*array_Buffer = (float *)calloc(Path_Len, sizeof(float));
		float	Frequency = sample_Rate*Path/FFT_Length;
		float	wave_k = Frequency/v_Tissue;
		int	point, centre_Point = n_rays/2;
		int	Bound = (Path_Len - n_rays)/2.;
		int	Bound2 = (Interp_FT_Len - n_rays)/2.;
		
		float	Mu_Factor;
		Ro = - (Ro_Index + 0.5) * 2.* wave_k /R0;
		
		if(wave_k)Zeta = -R0*R0/(8.*R_Equivalent*wave_k*(Ro_Index+1.));
		else Zeta = 0;
		if(!do_Focusing) Zeta = 0;

		if(wave_k)alpha = 0.5*(R0/wave_k)*(R0/wave_k)/(a*a*2.*(Ro_Index+1.));
		else	alpha = 0;

		//if(wave_k)alpha = (R0/(wave_k*a)), alpha *= alpha;
		//else	alpha = 0;
							
		Mu_Factor = FACTOR*(Ro_Index + 0.5);
		Mu_Factor = 1. + Mu_Factor*Mu_Factor/2;
		
		for(point = 0; point < Path_Len; point ++)
			{
			FFT_Buffer1[point] = 0;
			FFT_Buffer2[point] = 0;
			}
		for(point = 0; point < Interp_FT_Len; point ++)
			{
			Interp_FT_Buffer1[point] = 0;
			Interp_FT_Buffer2[point] = 0;
			}
		for(point = 0; point < n_rays; point ++)
			{
			float	x0 = dX * (point - centre_Point);
			FFT_Buffer1[point + Bound] = Interp_Buffer1.at(Path,point) * polar(1,x0*x0*Ro);
			}
		FFT_Buffer1.roll();
		FFT_Buffer1.FFT(ftForward);
		FFT_Buffer1.roll();
				
		//if(Path == FFT_Length/2) grafc(FFT_Buffer1, Path_Len, 0, 1, "After FFT", "");		
				
		for(point = 0; point < Path_Len; point ++)
			{
			float	OMEGA = dOMEGA * (point - Path_Len/2);
			/*
			float	cosine = cos(Zeta*OMEGA*OMEGA);
			float	sine = sin(Zeta*OMEGA*OMEGA);
			*/
			complexF32 phaseCorrection = polar(1, Zeta*OMEGA*OMEGA);
			
			float factor;
			
			
			FFT_Buffer1[point] *= phaseCorrection;
			FFT_Buffer2[point] = FFT_Buffer1[point];

			factor = Amplification(OMEGA);
			array_Buffer[point] = factor;

			FFT_Buffer1[point] *= factor;
			FFT_Buffer2[point] *= Amplification(-OMEGA);
			}
		
		
		FFT_Buffer1.roll();	
		FFT_Buffer2.roll();	
		
		for(point = 0; point < Path_Len/2; point ++)
			{
			Interp_FT_Buffer1[point] = FFT_Buffer1[point];
			Interp_FT_Buffer2[point] = FFT_Buffer2[point];
			}
		for(point = 1; point < Path_Len/2; point ++)
			{
			Interp_FT_Buffer1[Interp_FT_Len - point] = FFT_Buffer1[Path_Len - point];
			Interp_FT_Buffer2[Interp_FT_Len - point] = FFT_Buffer2[Path_Len - point];
			}
		Interp_FT_Buffer1.FFT(ftReverse);
		Interp_FT_Buffer2.FFT(ftReverse);
				
		Interp_FT_Buffer1.roll();
		Interp_FT_Buffer2.roll();
		
		for(point = 0; point < n_rays; point ++)
			{
			int	n = Interp_FT_Len/2 + (point - n_rays/2)/Scale_Factor;
			
			if(n >= 0 && n < Interp_FT_Len){ 								
				Interp_Buffer1.at(Path,point) = Interp_FT_Buffer1[n];	
				Interp_Buffer2.at(Path,point) = Interp_FT_Buffer2[n];
				}
			else
				{
				Interp_Buffer1.at(Path,point) = 0;
				Interp_Buffer2.at(Path,point) = 0;
				}
			}
		NextProgress();
		free(array_Buffer);
		}	
	}

float	*Frequency_Filtre = NULL;
	
void	NL_Pathfinder :: Interpolate(short Subaperture)
	{
	int	Path, Harmonic, Sample, point, n_Points = n_rays;
	int	centre_Point = n_Points/2;
	complexF32 *FFT_Buffer;
	if(!Frequency_Filtre)
		{
		Frequency_Filtre = (float *)calloc(FFT_Length, sizeof(float));
		if(Use_Frequency_Filtre)
			Get_Function(Frequency_Filtre, FFT_Length, "Frequency filtre");
		else 	for(Harmonic = 0; Harmonic < FFT_Length; Harmonic ++)
			Frequency_Filtre[Harmonic] = 1;
		}
	FFT_Buffer = new complexF32[FFT_Length];
		
	/*
	SetFrequencyUnits(M_HERZ);
	for(Harmonic = 0; Harmonic < FFT_Length; Harmonic ++)
		{
		float	f = (sample_Rate/FFT_Length)*Harmonic;
		float	f0 = omega0 - half_Width;
		float	s = half_Width/4;
		
		if(fabs((f-f0)/(2*s)) < 1)
			Frequency_Filtre[Harmonic] = 0.54 + 0.46*cos(PI*(f-f0)/(2.*s));
		else	Frequency_Filtre[Harmonic] = 0;
		if(!Use_Frequency_Filtre) Frequency_Filtre[Harmonic] = 1.;
		}
	
	*/
	
	////SetDepthUnits(CENTIMETRES);
	//dZ = (r_max - r_min)/n_samples;
	
	for(point = 0; point < n_Points; point ++)
		{
		SetCurrentRay(point);
		
		for(Path = 0, Sample = From_Sample; Sample < n_samples && Path < FFT_Length; Path ++, Sample ++)
			Interp_Buffer1.at(Path,point) = CurrentRay[Subaperture][Sample];
		for(;Path < FFT_Length; Path ++) Interp_Buffer1.at(Path,point) = 0;
		
		for(Path = 0; Path < FFT_Length; Path ++)
			{
			FFT_Buffer[Path] = Interp_Buffer1.at(Path,point);
			}
		
		FFT(FFT_Buffer, FFT_Length, ftForward);
		for(Harmonic = 0; Harmonic < FFT_Length; Harmonic ++)
			{
			Interp_Buffer1.at(Harmonic,point) = FFT_Buffer[Harmonic]*
				Frequency_Filtre[Harmonic];
			}
		}
	delete[] FFT_Buffer;
	//free(Frequency_Filtre);
	}

void	NL_Pathfinder :: ReInterpolate(short Subaperture)
	{
	int	Sample, Path, Harmonic ;
	int	centre_Point = n_rays/2;
	int	n_Points = n_rays;
	
	complexF32 *FFT_Buffer1 = NULL;
	complexF32 *FFT_Buffer2 = NULL;
	
	if(!FFT_Buffer1)FFT_Buffer1 = new complexF32[FFT_Length];
	if(!FFT_Buffer2)FFT_Buffer2 = new complexF32[FFT_Length];
	//SetAngleUnits(RADIANS);
	//SetDepthUnits(CENTIMETRES);
	dZ = (r_max - r_min)/n_samples;
	
	for(Start_Ray(); End_Ray(); Next_Ray())
		{
		int	point = ray;
		float	angle;
		float	deltaFi; 
	
		SetFrequencyUnits(BACK_SECONDS);
		//SetAngleUnits(RADIANS);
		angle = CentreAngle() - CurrentRayAngle();

		deltaFi = Find_Schift()*
			sample_Rate * (r_min + dZ*From_Sample)*
			(1.-1./cos(angle))/(v_Tissue*FFT_Length);		
		
		for(Harmonic = 0; Harmonic < FFT_Length; Harmonic ++)
			{
			float	Fi = (float)Harmonic*deltaFi;
			complexF32 Phasor = polar(1, Fi);
			
			FFT_Buffer1[Harmonic] = Interp_Buffer1.at(Harmonic,point)*Phasor;
			FFT_Buffer2[Harmonic] = Interp_Buffer2.at(Harmonic,point)*Phasor;
			}
		FFT(FFT_Buffer1, FFT_Length, ftReverse);
		FFT(FFT_Buffer2, FFT_Length, ftReverse);
		
		/*
		if(point == 32 || point == 75)
			{
			grafc(FFT_Buffer1, delta_Sample, 0, 1, "Buffer 1", "Jabber"),
			grafc(FFT_Buffer2, delta_Sample, 0, 1, "Buffer 2", "Jabber");		
			}
		*/
		for(Path = 0, Sample = From_Sample; Sample < To_Sample; Path++, Sample++)
			{
			float	some_Value = real(FFT_Buffer1[Path]%FFT_Buffer2[Path]);

			if(!Subaperture)CurrentRay[Sample] = some_Value;
			else
				{
				CurrentRay[Sample] += some_Value;
				CurrentRay[Subaperture][Sample] = 0;
				}
			}
		}		
	free(FFT_Buffer1);
	free(FFT_Buffer2);
	}

	
	
	
float	NL_Pathfinder :: Find_Scale_Factor()
	{
	float	f_Index, Precomputed[] = {
			-1.01, -.96, -.92, -.88, -.84, -.8, -.76, -.72, -.68, -.64,
		//	1.5	  1.45	     1.4	1.35	   1.3		
			-.6, -.56, -.52, -.48, -.44, -.41, -.38, -.36, -.34, -.33,
		//	1.25	   1.2	      1.15	  1.1	    1.05
			
			.3, .305, .31, .32, .33, .35, .38, .41, .45, .49,
			.53, .57, .61, .65, .7, .75, .8, .85, .9, .95, .1
			};
	int	Index;
	if(Ro_Index >= -0.5 || Ro_Index <= -1.5) return	2.*(Ro_Index + 1);
	if(Ro_Index > -1.0625 && Ro_Index < -0.9375) return 1.;
	
	//f_Index = 20. * 2. * (Ro_Index + 1.5);
	f_Index = 20. + 20. * 2. * (Ro_Index + 1.);
	Index = (int)f_Index;
	f_Index -= Index;
	return	Precomputed[Index] + f_Index * (Precomputed[Index+1] - Precomputed[Index]);
	}
	
	
float	NL_Pathfinder :: Find_Schift()
	{
	float	Precomputed[] = {6, 4, 1.5, 0., -0.5, -0.5};
	float	SF = Find_Scale_Factor();
	
	float	a;
	float	f_Index = 2*(Ro_Index+2);
int	index = f_Index;
	
	f_Index -= index;
	
	a = Precomputed[index] + f_Index*(Precomputed[index+1]-Precomputed[index]);
	return a/SF;
	
	if(Ro_Index < -0.5 && Ro_Index > -1) return	1./SF;
	if(Ro_Index <= -1 && Ro_Index > -1.5) return	2./SF;
	if(Ro_Index <= -1.5 && Ro_Index >= -2) return	4./SF;
	return -1/(2.*SF);
	}


void	NL_Pathfinder :: power_Group(float p){
#pragma message "этой функции место в классе NL_Pathfinder"
	int	Sample;
	
	for(Start_Ray(); End_Ray(); Next_Ray())
		{
		for(Sample = 0; Sample < n_samples; Sample ++)
			{
			if(real(CurrentRay[Sample]) < 0)
				real(CurrentRay[Sample]) = -pow(-real(CurrentRay[Sample]), p);
			else real(CurrentRay[Sample]) = pow(real(CurrentRay[Sample]), p);
			imag(CurrentRay[Sample]) = 0;
			}
		}
	}

void	NL_Pathfinder :: sqrt_Group()
	{
	int	Sample;
	
	for(Start_Ray(); End_Ray(); Next_Ray())
		{
		for(Sample = 0; Sample < n_samples; Sample ++)
			{
			if(real(CurrentRay[Sample]) < 0)
				real(CurrentRay[Sample]) = -sqrt(-real(CurrentRay[Sample]));
			else real(CurrentRay[Sample]) = sqrt(real(CurrentRay[Sample]));
			
			if(imag(CurrentRay[Sample]) < 0)
				imag(CurrentRay[Sample]) = -sqrt(-imag(CurrentRay[Sample]));
			else imag(CurrentRay[Sample]) = sqrt(imag(CurrentRay[Sample]));
			}
		}
	}

void	NL_Pathfinder :: Detect_1st_SL(short Sample, float *Where, float *How){
static	float	oldMean = 0;
	
	float	mean;
	float	c_Min;
	
	short	nPoints = n_rays;
	short	corrLen = nPoints/4;
	short	point, i;
	short	stopFlag;

	float	mainLobe = 0, firstLobe = 0;
	
	RealFunctionF32	corrBuffer(nPoints);
	RealFunctionF32	Correl(corrLen);


	for(Start_Ray(); End_Ray(); Next_Ray())
		{
		corrBuffer[ray] = real(CurrentRay[Sample]);
		}
	mean = corrBuffer.AverageValue();

	for(point = 0; point < nPoints; point ++) corrBuffer[point] -= mean;

	for(point = 0; point < nPoints*0.75; point ++)
		{
		for(i = 0; i < corrLen; i++)
			{
			Correl[i] += corrBuffer[point]*corrBuffer[point + i];
			}
		}

	//Correl.Display(0, 1, "Correlation");
	for(point = 1; point < corrLen; point ++) Correl[point] /= Correl[0];
	
	float	slPos;
	
	/*
	Correl[0] = 1;
	c_Min = 1;
	stopFlag = 0;
	for(i = 0; i < corrLen && !stopFlag; i++)
		{
		if(c_Min > Correl[i])
			c_Min = Correl[i],
			whereI = i;
		if(c_Min < Correl[i]) stopFlag = 1; // To the first local minimum
		}

	for(i = 0; i < nPoints-whereI; i++)
		{
		if(corrBuffer[i] > 0 && corrBuffer[i + whereI] < 0)
			mainLobe += corrBuffer[i],
			firstLobe -= corrBuffer[i+whereI];

		if(corrBuffer[i] < 0 && corrBuffer[i + whereI] > 0)
			mainLobe += corrBuffer[i+whereI],
			firstLobe -= corrBuffer[i];
		}

	if(Correl[whereI] && whereI >= 1 && whereI < corrLen-1)
		{
		float	wi_div = 2*Correl[whereI] - (Correl[whereI+1] + Correl[whereI-1]);
		float	wi_f = (Correl[whereI+1] - Correl[whereI-1]);
		if(wi_div)wi_offset = wi_f/wi_div;
		else wi_offset = 0;
		}
	else wi_offset = 0;
	slPos = whereI + wi_offset;
	*/

	Correl[0] = 1;
	c_Min = 1;
	stopFlag = 0;
	float	dPos = 1./32.;
	/*
	RealFunctionF32	interpF(corrLen/dPos);
	for(slPos = 0; slPos < corrLen; slPos += dPos)
		{
		int	index = slPos/dPos;
		interpF[index] = Correl(slPos);
		}
	interpF.Display(0, 1, "Interpolated");
	*/
	for(slPos = 1; slPos < corrLen && !stopFlag; slPos += dPos)
		{
		float	value = Correl(slPos);
		if(c_Min > value) c_Min = value;
		if(c_Min < value) stopFlag = 1; // Till the first local minimum
		}

	for(i = 0; i < nPoints-slPos; i++)
		{
		if(corrBuffer(i) > 0 && corrBuffer(i + slPos) < 0)
			mainLobe += corrBuffer(i),
			firstLobe -= corrBuffer(i + slPos);

		if(corrBuffer(i) < 0 && corrBuffer(i + slPos) > 0)
			mainLobe += corrBuffer(i+slPos),
			firstLobe -= corrBuffer(i);
		}


	//SetAngleUnits(RADIANS);

	if(firstLobe < (float)HUGE_VAL){

		*Where = slPos*(end_Angle - start_Angle)/nPoints;
		*How = 0.6*firstLobe/mainLobe;

		*How = firstLobe/(mainLobe + firstLobe);
		}

	else *Where = *How = 0;

	//SetAngleUnits(DEGREES);

	if(!(*How))
		{
		//Show_Double("How", *How);

		//Show_Double("Where", *Where);
		//Show_Double("Where (samples)", slPos);
		//Show_Double("Where offset", wi_offset);

		corrBuffer.Display(0, (end_Angle - start_Angle)/nPoints, "Signal");
		Correl.Display(0, (end_Angle - start_Angle)/(nPoints), "Correlation function");
		oldMean = mean;
		}
	//SetAngleUnits(RADIANS);
	}

extern	funcParam defaultFuncParam;
extern	short	freezeFuncParams;



void	roll_c2(complexF32 *data, short len, short how)
	{
	short	i, j;
	complexF32 *Buffer;
	if(how >= len) while(how >= len){how -= len;}
	if(how < 0) while(how < 0){how += len;}
	Buffer = (complexF32 *)calloc(how, sizeof(complexF32));

	for(i = 0; i < how; i ++) Buffer[i] = data[i];
	for(i = 0, j = how; j < len; i++, j++) data[i] = data[j];

	for(i = 0, j = len - how; i < how; i ++, j++) data[j] = Buffer[i];

	free(Buffer);
	}


void	NL_Pathfinder :: Intelligent_Suppress(float Where, float How)
	{
	int	Sample;

	for(Sample = 0; Sample < n_samples; Sample ++)
		{
		Detect_1st_SL(Sample, &Where, &How);
		Single_Intelligent_Suppress(Sample, Where, How);
		}
	}




void	NL_Pathfinder :: Single_Intelligent_Suppress(short Sample, float Where, float How){

	RealFunctionF32 Buffer1(n_rays), Buffer2(n_rays);
	float	n_Radians_per_Pnt, dPoint;
	int	point, nPoints = n_rays;
	
	Buffer1.InitInterpolator(16);
	
	//SetAngleUnits(RADIANS);
	n_Radians_per_Pnt = (end_Angle - start_Angle)/n_rays;

	dPoint = Where/n_Radians_per_Pnt;
	//Show_Double("dPoint", dPoint);

	for(Start_Ray(); End_Ray(); Next_Ray())
		{
		point = ray;
		Buffer1[point] = real(CurrentRay[Sample]);
		Buffer2[point] = real(CurrentRay[Sample]);
		}
	
	
	for(point = 0; point < nPoints; point ++)
		Buffer2[point] = Buffer1(point);
	for(point = 0; point < nPoints; point ++)if(Buffer2[point] < 0)
		{
		float	howStore = How;
		float	p;
		//p = (float)point - dPoint;


		if(point > 2*dPoint && point < nPoints-2*dPoint-1)
			{
			float pos = 0;
			float neg = 0;
			short	index = 0;
			
			for(p = point - dPoint; p < point + dPoint; p += 0.5)
				{
				
				float	p0 = Buffer1(p);
				float	p1 = Buffer1(p-dPoint);
				float	p2 = Buffer1(p+dPoint);
				
				if(p1 + p2 > 0 && p0 < 0)
					{
					pos += (p1+p2);
					neg -= p0, index ++;
					}
				}
			if(index) How = neg/pos;
			else How = howStore;
			}
		p = (float)point - dPoint;
		if(p >= 0)
			{
			float	offset = How * Buffer1(p);
			if(offset > 0) Buffer2[point] += offset;
			}
		p = point + dPoint;
		if(p < nPoints - 1)
			{
			float	offset = How * Buffer1(p);
			if(offset > 0) Buffer2[point] += offset;
			}
		How = howStore;
		}
	for(Start_Ray(); End_Ray(); Next_Ray())
		{
		point = ray;
		CurrentRay[Sample] = Buffer2[point];
		}
	}

	
#endif