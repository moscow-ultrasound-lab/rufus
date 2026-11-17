#include "Pre.h"

#include "SignalProcessing/_rare_/AdaptiveFocusing/zpathfinder.h"



XRAD_BEGIN

void	some()
{
	
//	vector<int>::iterator	p1, p2, p3;
	DataArray<int>::iterator	p1, p2, p3;
	//int	*p1, *p2, *p3;
	std::copy(p1, p2, p3);
}

bool atl_data = false;



ZPathfinder :: ZPathfinder():Pathfinder_Base(){}

ZPathfinder :: ~ZPathfinder(){}

void	ZPathfinder :: InitWork()
	{
	delta_Sample = n_samples;
	Pathfinder_Base :: InitWork();
	
	atl_data = false; //Decide2("Choose data type", "Simulated data", "ATL data", true);
	
	strcpy(SIMIO::Process_Name,"ZPathfinder");
	Interp_Buffer.realloc(FFT_Length, Path_Len);
	Display("Read-in signal");
//	focus_Algorithm = EMULATED_PATH;
	SetOutputFileName(Signal_IP_File_Name, "СEMU");
	ISignal_Write();
	}

void	ZPathfinder :: EndWork()
	{
	Write_Data();
	Display("Processed signal");
	}

void	ZPathfinder :: Batch()
	{
	Process_Group(); 
	}

bool	both_focusing = true;

void	ZPathfinder :: Process_Group()
	{
//	int	i;
//	int	Sample, Harmonic;
	float	d_omega;
	ComplexFunctionF32	Phasors(FFT_Length);
	
	
	Interpolate();		
	
	both_focusing = YesOrNo("Are both TX and RX aperture focused", true);
	if(YesOrNo("Process ZPicture?", false)) Process_ZPicture();
	
	//SetDepthUnits(CENTIMETRES);
//	SetFrequencyUnits(BACK_SECONDS);
	
	dZ = depth_range().cm()/n_samples;
	d_omega = sample_rate.rad_sec()/FFT_Length;
	
	for(size_t i = 0; i < FFT_Length; i++)
		{
		float	Phase = 2.*d_omega*i*dZ/sound_speed.cm_sec();
		if(!atl_data)	Phasors[i] = polar(1, -Phase);
		else		Phasors[i] = polar(1, Phase);
		}
	GUIProgressBar	progress;
	progress.start("Processing paths...", n_samples);
	for(size_t Sample = 0; Sample < n_samples; Sample ++)
		{
		Process_Sample(Sample);
		for(size_t Harmonic = 0; Harmonic < FFT_Length; Harmonic ++)
			{
			for(size_t i = 0; i < Path_Len; i++)
				{
				Interp_Buffer.at(Harmonic,i) *= Phasors[Harmonic];
				}
			}
		++progress;
		}
	progress.end();
	}

void	ZPathfinder :: Process_ZPicture()
	{
	
	//SetAngleUnits(RADIANS);
//	SetFrequencyUnits(BACK_SECONDS);
	//SetDepthUnits(CENTIMETRES);
	
	size_t	path2 = Path_Len/2;
	
	double	dz = 1./40;
	if(YesOrNo("Detailed scale?", true)) dz = 1./400;
	float	dx = dz;
	
	
//	float	dR = depth_range().cm()/n_samples;
//	float	dFi = angle_range().radians()/Path_Len;
	
	float	xmin = r_max().cm()*sin(start_angle().radians());
	float	xmax = r_max().cm()*sin(end_angle().radians());
	float	zmin = min(r_min().cm()*cos(end_angle().radians()), r_min().cm()*cos(start_angle().radians()));
	float	zmax = r_max().cm();
	
	size_t	zs = (zmax-zmin)/dz;
	size_t	xs = (xmax-xmin)/dz;
	
	
	ComplexFunction2D_F32	zmatrix(zs, xs);
	
//	int	i,j,p,q;
	float	da = angle_range().radians()/n_rays;
	
	float	dOMEGA = two_pi()/(da*Path_Len);
	float	d_omega = sample_rate.rad_sec()/FFT_Length;

//	Zeta = (z-FX_Point)/(z*wave_k*FX_Point);

	float	ROI_zmax = GetFloating("Maximum z:", r_max().cm(), r_min().cm(), r_max().cm());
	
	DisplayMathFunction2D(Interp_Buffer, "IB Before");


	GUIProgressBar	progress;
	progress.start("Creating ZPicture", zs);
	for(size_t i = 0; i < zs; i++)
		{
		float	z = zmin + dz*i;
		float	zeta0 = (z-FX_Point)/(4*z*FX_Point);
		if(FX_Point) zeta0 = (z-FX_Point)/(4*z*FX_Point);
		else zeta0 = -1/(4*z);
		
		if(z < ROI_zmax) for(size_t j = 0; j < xs; j++)
			{
			float	x = xmin + j*dx;
			
			float	fi = std::atan2(x,z);
			float	fi0 = fi - start_angle().radians();
			// fi0 -- аргумент преобразованиЯ фурье, который пробегает значениЯ [0, end_angle-start_angle)
			float	r = sqrt(x*x + z*z);
			float	t = 2*(r - r_min().cm())/sound_speed.cm_sec();
//			float	cosFi = z/r;
//			float	sinFi = x/r;
			
			float	z1 = z;
			
			if(FX_Point) zeta0 = (z1-FX_Point)/(4*z1*FX_Point);
			else zeta0 = -1/(4*z1);

			//zeta0 /= (1+x*x);
			zeta0 /= sqrt(1 + fi*fi);
						
			//if(x > -1.5 && x < 0.5)
			if(r > r_min().cm() && r < r_max().cm() && fi0 < angle_range().radians() && fi0 > 0)for(size_t p = 0; p < FFT_Length; p++)
				{
				ForceUpdateGUI();
				float	omega = p*d_omega;
				//float	Zeta;
				float	wave_k = omega/sound_speed.cm_sec();
				if(wave_k) Zeta = zeta0/wave_k;
				else Zeta = 0;
				if(!both_focusing) Zeta *= 2;
				
				
				if(!is_number(Zeta)) Error(ssprintf("Invalid Zeta = %g, (z0=%g, k=%g)", Zeta, zeta0, wave_k));
				for(size_t q = 0; q < Path_Len; q++)
					{
					float	OMEGA = (q - path2)*dOMEGA;
					complexF32 increment = Interp_Buffer.at(p,q)*polar(1, -(omega*t - OMEGA*fi0) + Zeta*OMEGA*OMEGA);
					if(!is_number(real(increment)) || !is_number(imag(increment)))
						{
						increment = 0;
						}
					zmatrix.at(i,j) += increment;
					}
				}
			else zmatrix.at(i,j) = 0;
			}
		++progress;
		}
	DisplayMathFunction2D(zmatrix, "Computed zmatrix");
	DisplayMathFunction2D(Interp_Buffer, "IB after");
	progress.end();
	}

	
void	ZPathfinder :: Process_Sample(size_t Sample)
	{
//	int	Harmonic;
	size_t	sub = 0;
	float	dOMEGA;
		
//	float	Ro_1 = fabs(Ro_Index+1);
	ptrdiff_t	Bound = (ptrdiff_t(Path_Len) - n_rays)/2;
//	int	point;
	ComplexFunctionF32 FFT_Buffer(Path_Len);
	

	//SetAngleUnits(RADIANS);
	//SetDepthUnits(CENTIMETRES);
//	SetFrequencyUnits(BACK_SECONDS);
		
	dAngle = angle_range().radians()/n_rays;
	
	
	dZ = depth_range().cm()/n_samples;
	R0 = (r_min().cm() + dZ*Sample);

	if(FX_Point) R_Equivalent = R0*FX_Point/(FX_Point - R0);
	else	R_Equivalent = R0;
	
	dX = R0 * dAngle;
	a = array_Pitch.cm() * n_elements;
	dOMEGA = two_pi()/(dX*Path_Len);
	Scale_Factor = Find_Scale_Factor();
	
		
	
	for(size_t Harmonic = 0; Harmonic < FFT_Length; Harmonic ++)
		{
		ForceUpdateGUI();
		float	Frequency = sample_rate.rad_sec()*Harmonic/FFT_Length;
		float	wave_k = Frequency/sound_speed.cm_sec();
		
		Ro = - (Ro_Index + 0.5) * 2.* wave_k/R0;
		
		if(wave_k)Zeta = -R0*R0/(8.*R_Equivalent*wave_k*(Ro_Index+1.));
		else Zeta = 0;

		if(!both_focusing) Zeta *= 2;
		
		if(wave_k)alpha = 0.5*(R0/wave_k)*(R0/wave_k)/(a*a*2.*(Ro_Index+1.));
		else	alpha = 0;
			
		/*
		Mu = 1.4;
		deltaY = Mu*PI*R0/(2*wave_k*a);
		if(fabs(deltaY) < fabs(dX/8)) deltaY = dX/8;
		if(!wave_k) deltaY = dX*Path_Len;
		*/
		Find_Fresnel(dOMEGA);
		
		for(size_t point = 0; point < Path_Len; point ++)
			{
			float	OMEGA = dOMEGA * (point - Path_Len/2);
				//- Receptor_X*wave_k/R0;
			complexF32 buffer;
//			float	old_Abs = cabs(FFT_Buffer[point]);
			float	factor = real(Fresnel_Buffer[point]);
			float	phase = Zeta*OMEGA*OMEGA + imag(Fresnel_Buffer[point]);
			
			FFT_Buffer[point] += Interp_Buffer.at(Harmonic,point) * polar(factor,phase);
			}
		}

	FFT_Buffer.roll_half(true);
	FFT(FFT_Buffer, ftReverse);		

		
	for(size_t ray = 0; ray < n_rays; ++ray)
		{
		ComplexFunctionF32	CurrentRay;
		focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
//		SetCurrentRay(ray);
		size_t point = ray + Bound;
		CurrentRay[Sample] = FFT_Buffer[point];
		}
	}
	

void	ZPathfinder :: Interpolate()
	{
//	int	Harmonic;//, point, n_Points = n_rays;
	ptrdiff_t	Bound = (ptrdiff_t(Path_Len) - n_rays)/2;
		
	
//	SetFrequencyUnits(M_HERZ);
	//SetAngleUnits(RADIANS);
	//SetDepthUnits(CENTIMETRES);
	
	Interp_Buffer.fill(complexF32(0));
	size_t	sub = 0;
		
	for(size_t ray = 0; ray < n_rays; ++ray)
		{
		ComplexFunctionF32	CurrentRay;
		focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
//		SetCurrentRay(ray);

		Interp_Buffer.col(ray).CopyData(CurrentRay);
		if(atl_data) FFT(Interp_Buffer.col(ray), ftForward);
		else FFT(Interp_Buffer.col(ray), ftReverse);
		}
	DisplayMathFunction2D(Interp_Buffer, "First FFT");
	for(size_t Harmonic = 0; Harmonic < FFT_Length; Harmonic ++)
		{
		ApplyWindowFunction(Interp_Buffer.row(Harmonic), cos2_window());
		Interp_Buffer.row(Harmonic).roll(Bound);
		FFT(Interp_Buffer.row(Harmonic), ftForward);
		Interp_Buffer.row(Harmonic).roll_half(true);
		ApplyWindowFunction(Interp_Buffer.row(Harmonic), cos2_window());
		}
	DisplayMathFunction2D(Interp_Buffer, "Second  FFT");
	}
	
XRAD_END
