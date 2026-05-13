#include "Pre.h"
#include "SignalProcessing/_rare_/AdaptiveFocusing/CurvePathfinder.h"


XRAD_BEGIN

void	Curve_Pathfinder :: ProcessInitDialog()
	{
//	short	answer = 0;

	if(n_rays != 64 && n_rays != 128 && n_rays != 32 && n_rays != 256)
		FatalError("Wrong rays number (must be 2^n for this method)!");

	delta_Sample = n_samples;
	Ro_Index = -0.5;
	//SetDepthUnits(CENTIMETRES);	
	
	if(TX_Focus.size() != 1 || RX_Focus.size() != 1)
		FatalError("Signal is dynamically focused!");
	if(TX_Focus[0] != RX_Focus[0])
		FatalError("Can't focus since TX/RX apertures are focused differently!");
	FX_Point = TX_Focus[0].cm();


// 	qmMacDialog	theDialog(2000);
// 	
// 	theDialog.MakeItemNEditText(3);
// 	theDialog.SetItemValue(3, delta_Sample);
// 	theDialog.SetItemValue(6, Ro_Index);
// 	theDialog.SetItemValue(8, FX_Point);
// 	theDialog.Set_Control_Value(7, true/*Visible*/);
// 	theDialog.Set_Control_Value(13, Do_Fresnel_Correction);
// 	
// 	theDialog.HideItem(12);
	
// 	if(FX_Point)
// 		{
// 		theDialog.ShowItem(8);
// 		theDialog.ShowItem(9);
// 		}
// 	else
// 		{
// 		theDialog.HideItem(8);
// 		theDialog.HideItem(9);
// 		}
// 	theDialog.HideItem(10);
// 	// it was 'group overlapping'
// 	
// 	theDialog.Show();
	
// 	do
// 		{
// 		short	prevAnswer;
// 		short	RealEvent = theDialog.ModelessDialog(&answer);
// 
// 		if(answer != prevAnswer || theDialog.SpecNumItem(answer))
// 			{
// 			delta_Sample = theDialog.NumItemRange(3, (float)delta_Sample, 2, n_samples);
// 			Ro_Index = theDialog.NumItemRange(6, Ro_Index, -2.0, 1.0);
// 			if(FX_Point) FX_Point = theDialog.NumItemRange(8, FX_Point, r_min().cm(), 10000);
// 			theDialog.SelectItemText(answer);
// 			}
// 		if(RealEvent)switch(answer)
// 			{
// 			case	2:
// 				//Use_Emulated_Array = !theDialog.Get_Control_Value(answer);
// 				//theDialog.Set_Control_Value(answer, Use_Emulated_Array);
// 				break;
// 			case	7:
// 				//Visible = !theDialog.Get_Control_Value(answer);
// 				//theDialog.Set_Control_Value(answer, Visible);
// 				break;
// 			case	10:// it was 'group overlapping'. now empty (reserved) checkbox
// 				break;
// 			case	13:
// 				Do_Fresnel_Correction = !theDialog.Get_Control_Value(answer);
// 				theDialog.Set_Control_Value(answer, Do_Fresnel_Correction);
// 				break;
// 			case	11:
// 				throw canceled_operation("Operation canceled");
// 				break;
// 			}
// 		prevAnswer = answer;
// 		}while(answer != 1);
	}

float	Corr = 1;

Curve_Pathfinder :: Curve_Pathfinder():Pathfinder_Base(){}
	
Curve_Pathfinder :: ~Curve_Pathfinder(){}

void	Curve_Pathfinder :: InitWork()
	{
	float	Ro_1;
	int	i;
	int	intrrrL;
	
	strcpy(SIMIO::Process_Name,"Curve-Pathfinder");
	Pathfinder_Base :: InitWork();

	Receptor_X = 0;
	FFT_Length = delta_Sample;
	n_intervals = n_samples/delta_Sample + 1;
			
	Interp_Buffer.realloc(n_rays, FFT_Length);
	if(n_rays > 256) FatalError("Too many rays per sector");
	
	Path_Len = n_rays;
	Ro_1 = fabs(Ro_Index + 1);	

	intrrrL = 16;
	
	intrrrL = GetSigned("Interpolator length", intrrrL, 4, 32);


	nRoofs = GetSigned("Number of bands", 8, 1, intrrrL);
	Intrrr = (Interpolator **)calloc(nRoofs, sizeof(Interpolator *));

	for(i = 0; i < nRoofs; i ++)
		{
		int	filtreNo = roof8;
				
		if(intrrrL/nRoofs <= 4) filtreNo = roof8;
		if(intrrrL/nRoofs <= 2) filtreNo = roof4;
		if(intrrrL/nRoofs <= 1) filtreNo = deltaF;
		
		Intrrr[i] = new Interpolator;
		Intrrr[i] -> I_Interpolator(16, intrrrL, filtreNo, (float)(i+Corr)/(nRoofs+Corr));
		//Intrrr[i] -> Analyze();
		}
	Display("Read-in signal");
//	focus_Algorithm = EMULATED_PATH;
	SetOutputFileName(Signal_IP_File_Name, "СCurv");
	ISignal_Write();
	}

void	Curve_Pathfinder :: EndWork()
	{
	for(int i = 0; i < nRoofs; i ++) delete Intrrr[i];
	Write_Data();
	Display("Processed signal");
	}

void	Curve_Pathfinder :: Batch()
	{
	int	startTime, endTime;
	
//	focus_Algorithm = EMULATED_PATH;
	GUIProgressBar	progress;
	progress.start("Processing paths...", FFT_Length);
	
	startTime = clock();
	Process_Group();
	endTime = clock();
	
	progress.end();
	ShowFloating("Full processing time",((double)endTime - startTime)/60.);
	}

void	Curve_Pathfinder :: Process_Group()
	{
	From_Sample = 0;
	To_Sample = n_samples;
	
	Interpolate();		
	Process_Interval();
	ReInterpolate();
	}
	
void	Curve_Pathfinder :: Process_Interval()
	{
	size_t	Path;
	size_t	Interp_FT_Len = Path_Len;
	
	float	dOMEGA;
	float	cosFI;
		
	float	Ro_1 = fabs(Ro_Index+1);
	
	//SetAngleUnits(RADIANS);
	//SetDepthUnits(CENTIMETRES);
//	SetFrequencyUnits(BACK_SECONDS);
	
	dAngle = angle_range().radians()/n_rays;
	cosFI = cos(CentreAngle().radians());
	
	dZ = depth_range().cm()/n_samples;
	
	a = array_Pitch.cm() * n_elements / (2.*cosFI);
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
	size_t	sub = 0;
	for(Path = 0; Path < FFT_Length; Path ++)
		{
		size_t	point;
//		float	a1 = 2./(a*a);	
//		float	Mu_Factor;
		size_t	Sample = Path;
		int	r;
		
		R0 = r_min().cm() + Path*dZ;
		dX = R0 * dAngle;
		dOMEGA = two_pi()/(dX*Path_Len);

		if(FX_Point)R_Equivalent = R0*FX_Point/(FX_Point - R0);
		else	R_Equivalent = R0;
		if(R0 == FX_Point) R_Equivalent = 1.0e4;
		
		
//		Mu_Factor = 6.5*(Ro_Index + 0.5);		
//		Mu_Factor = sqrt(1. + Mu_Factor*Mu_Factor/2);
		
//		Mu = 1.4;
//		Mu = 1.4/Mu_Factor;
		/*
		deltaY = Mu*PI*R0/(2*wave_k*a);
		if(fabs(deltaY) < fabs(dX/8)) deltaY = dX/8;
		if(!wave_k) deltaY = dX*Path_Len;
		*/
		
		for(point = 0; point < Path_Len/2; point ++)
			{
			ComplexFunctionF32	CurrentRay;
			focused_data.GetRow(CurrentRay, {sub, point, slice_mask(0)});
			CurrentRay[Sample] = 0;
			focused_data.GetRow(CurrentRay, {sub, Path_Len-point-1, slice_mask(0)});
			CurrentRay[Sample] = 0;

// 			SetCurrentRay(point);
// 			SetCurrentRay(Path_Len - point - 1);
			
			for(r = 0; r < nRoofs; r ++)
				{
				float	OMEGA;
				float	tau_i;
				ptrdiff_t	sPoint;
				float	sPath;
				complexF32 Interpolated;
				complexF32 fresnelFactor;
				float	phase, factor;
				complexF32 Fresnel;
				
				float	Frequency = sample_rate.rad_sec()*(r+Corr)/(nRoofs+Corr); //Here may be problem
				float	wave_k = Frequency/sound_speed.cm_sec();
				OMEGA = dOMEGA*point;
				tau_i = R0*R0*sound_speed.cm_sec()*OMEGA*OMEGA/(4.*R_Equivalent*Frequency*Frequency);
				sPath = (float)Path - tau_i * sample_rate.rad_sec()/(two_pi());
				if(sPath < 0) sPath = 0;
				if(sPath > FFT_Length - 1) sPath = FFT_Length - 1;
				
				
				Ro = - (Ro_Index + 0.5) * 2.* wave_k/R0;
				
				if(wave_k) Zeta = -R0*R0/(4.*R_Equivalent*wave_k);
				else Zeta = 0;
				Zeta *= 2;
				if(wave_k)	alpha = 0.5*(R0/wave_k)*(R0/wave_k)/(a*a);
				else	alpha = 0;

				Intrrr[r] -> SetMaxWhere(FFT_Length);
				Fresnel = Find_Fresnel1(dOMEGA, Path_Len/2 - point-1, wave_k);
				
				factor = real(Fresnel);
				phase = Zeta*OMEGA*OMEGA + imag(Fresnel);
				
				fresnelFactor = polar(factor, phase);
				
//				SetCurrentRay(point);
				focused_data.GetRow(CurrentRay, {sub, point, slice_mask(0)});
				
				Interpolated = Intrrr[r] -> Interpolate(&Interp_Buffer.at(point,0), sPath);
								
				CurrentRay[Sample] += fresnelFactor*Interpolated;

				
				sPoint = ptrdiff_t(Path_Len) - point - 1;
				Fresnel = Find_Fresnel1(dOMEGA, Path_Len/2 - point, wave_k);
				OMEGA = -dOMEGA*(point+1);
				tau_i = R0*R0*sound_speed.cm_sec()*OMEGA*OMEGA/(4.*R_Equivalent*Frequency*Frequency);
				sPath = (float)Path - tau_i * sample_rate.rad_sec()/(two_pi());
				if(sPath < 0) sPath = 0;
				if(sPath > FFT_Length - 1) sPath = FFT_Length - 1;
				
				factor = real(Fresnel);
				phase = Zeta*OMEGA*OMEGA + imag(Fresnel);
				fresnelFactor = polar(factor, phase);
				
//				SetCurrentRay(sPoint);
				focused_data.GetRow(CurrentRay, {sub, size_t(sPoint), slice_mask(0)});
				Interpolated = Intrrr[r] -> Interpolate(&Interp_Buffer.at(sPoint,0), sPath);
								
				CurrentRay[Sample] += fresnelFactor*Interpolated;
				}
			}
//		++progress;
		}
	}
	
void	Curve_Pathfinder :: Interpolate()
	{
	size_t	Path, Sample, point;
	ComplexFunctionF32 FFT_Buffer(Path_Len);
	
	size_t	sub = 0;

	for(Path = 0, Sample = From_Sample; Sample < n_samples && Path < FFT_Length; Path ++, Sample ++)
		{
		for(size_t ray = 0; ray < n_rays; ++ray)
			{
			ComplexFunctionF32	CurrentRay;
			focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
//			SetCurrentRay(ray);
			point = ray;
			FFT_Buffer[point] = CurrentRay[Sample];
			}
		FFT(FFT_Buffer, ftForward);
		for(size_t ray = 0; ray < n_rays; ++ray)
			{
			ComplexFunctionF32	CurrentRay;
			focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
//			SetCurrentRay(ray);
			point = ray;
			CurrentRay[Sample] = FFT_Buffer[point];
			Interp_Buffer.at(point,Path) = CurrentRay[Sample];
			}
		}
//	delete[] FFT_Buffer;
	}
	
void	Curve_Pathfinder :: ReInterpolate()
	{
	size_t	Path, Sample, point;
	ComplexFunctionF32 FFT_Buffer(Path_Len);

	size_t	sub = 0;

	for(Path = 0, Sample = From_Sample; Sample < n_samples && Path < FFT_Length; Path ++, Sample ++)
		{
		for(size_t ray = 0; ray < n_rays; ++ray)
			{
			ComplexFunctionF32	CurrentRay;
			focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
//			SetCurrentRay(ray);
			point = ray;
			FFT_Buffer[point] = CurrentRay[Sample];
			}
		FFT(FFT_Buffer, ftReverse);
		for(size_t ray = 0; ray < n_rays; ++ray)
			{
			ComplexFunctionF32	CurrentRay;
			focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
//			SetCurrentRay(ray);
			point = ray;
			CurrentRay[Sample] = FFT_Buffer[point];
			}
		}
	}

XRAD_END
