#include "Pre.h"
#include "Attenuator.h"
#include "interpolation_many_obsolete/Interpolation.h"
//#include <ImageUtils.h>

XRAD_BEGIN

Attenuator :: Attenuator():SignalProcessor()
	{
	char	Msg[255];
	strcpy(SIMIO::Process_Name,"Attenuator");
	n_intervals = 5; //GetSigned("Number of nonРoverlapping intervals to determine frequency:", 5, 1, 9);
	interval_increment = (n_samples - 2)/n_intervals;
	if(interval_increment * n_intervals > n_samples-2) interval_increment --;
	
	n_intervals = n_samples;
	
	sprintf(Msg, "there will be %lu intervals.", int(n_intervals));
	//Show_String("Remember, that ", Msg);
	Coefficient_Map = new GrayScanConverter(n_rays, n_samples);
	//Coefficient_Map -> Attach(this);
	ExportScanConverterOptions(*Coefficient_Map);
	}
	
Attenuator :: ~Attenuator()
	{
	delete	Coefficient_Map;
	}

void	Attenuator :: SaveCoefMap()
	{
	string	op_filename;
	try
		{
		op_filename = GetFileNameWrite("Create file for coefficient map", Signal_IP_File_Name.c_str(), ".frequency");
		}
	catch(canceled_operation &)
		{
		return;
		}
	FILE	*op_file = fopen(op_filename.c_str(), "wb");
	if(!op_file) return;

//	for(int i = 0; i < n_samples; i++) Coefficient_Map->col(i).write_numbers(op_file, ioFloatLE);	
	fclose(op_file);
	}


void	Attenuator :: ReadCoefMap()
	{
	string	ip_filename;
	try
		{
		ip_filename = GetFileNameRead("Open file with coefficient map");
		}
	catch(canceled_operation &)
		{
		return;
		}
	
	FILE	*ip_file = fopen(ip_filename.c_str(), "rb");
	if(!ip_file) return;
	
//	for(int i = 0; i < n_samples; i++) Coefficient_Map->col(i).read_numbers(ip_file, ioFloatLE);	
	fclose(ip_file);
	}


void	Attenuator :: InitWork()
	{
	Display("Signal before");
	}
	
void	Attenuator :: EndWork()
	{
	ptrdiff_t	index = 0;
	
	if(1)
		{
//		int	i;
		RealFunctionF32	ray_accumulator(n_rays);
		
//		Coefficient_Map -> SmoothRays(0.1);
		
//		SetAngleUnits(RADIANS);
		double	d_fi = angle_range().radians()/n_rays;
		for(size_t i = 0; i < n_samples; i++) ray_accumulator += Coefficient_Map->col(i);
		ray_accumulator /= double(n_samples);

		DisplayMathFunction(ray_accumulator, start_angle().radians(), d_fi, "ray_accumulator");
		if(YesOrNo("Correct parabolic?", true))
			{
			double	a0 = 0, a1 = 0, a_1 = 0;
			
			size_t	c0 = n_rays/2, c1 = n_rays/2 + n_rays/4, c_1 = n_rays/2-n_rays/4;
			size_t	n = n_rays/4;
			for(int i = -int(n); i < int(n); i++)
				{
				a0 += ray_accumulator[i+c0];
				a1 += ray_accumulator[i+c1];
				a_1 += ray_accumulator[i+c_1];
				}
			a0 /= (2*n + 1);
			a1 /= (2*n + 1);
			a_1 /= (2*n + 1);
			
			double	drv1 = (a1 - a0)/(c1-c0);
			double	drv_1 = (a0 - a_1)/(c0-c_1);
			double	drv2 = (drv1 - drv_1)/((c1 - c_1)/2);
			
	//		printf("\na0=%g,\ta1=%g,\ta_1=%g,", a0, a1, a_1);
	//		printf("\nc0=%d,\tc1=%d,\tc_1=%d,", c0, c1, c_1);
	//		printf("\ndrv1=%g,\tdrv_1=%g,\tdrv2=%g,", drv1, drv_1, drv2);
			
			fflush(stdout);
			
			for(int i = 0; i < int(n_rays); i++)
				{
				double	x = double(i-n_rays/2);
				double	removal = x*x*drv2/2;
				ray_accumulator[i] -= removal;
				Coefficient_Map->row(i) -= removal;
				}
			DisplayMathFunction(ray_accumulator, start_angle().radians(), d_fi, "ray_accumulator, no parabola");
			}
		}
	
	do
		{
		index = GetButtonDecision("Choose display item:", //4,
		{
			"Signal",
			"Coefficient",
			"Colour mapping",
			"Exit display"
		});
		if(index == 0) SectorData :: Display("Signal");
		if(index == 1) DisplayMathFunction2D(*Coefficient_Map, "Coefficient");
		if(index == 2) DisplayColour();
		}while(index != 3);
	}

double	c0_weight = 0;

ColorSampleF64	color_palette(double value, double max_value)
	{
	double	index = value/max_value;
		
	ColorSampleF64	result(0.,0.,0.);
	double	r0 = 0;
	double	r1 = 0.75, r2 = 1, r3 = 1;	// деление положительного диапазона
//	double	r_1 = 0.5, r_2 = 0.75, r_3 = 1;	// деление отрицательного диапазона (макс. зеленое)
	double	r_1 = 0.75, r_2 = 1, r_3 = 1;	// деление отрицательного диапазона (макс. голубое)

//	ColorSampleF64 c0(96, 0, 0), c1(256,0,0), c2(256,160,0), c3(200,256,0);
//	ColorSampleF64 c_0(96, 0, 96), c_1(0,128,256), c_2(0,256,196), c_3(0,256,0);
	ColorSampleF64 c0(256, 256, 256), c1(256,0,0), c2(256,160,0), c3(200,256,0);
	ColorSampleF64 c_0(256, 256, 256), c_1(0,128,256), c_2(0,256,196), c_3(0,256,0);

 // чувствительность к малым значениЯм
	if(fabs(index) < r0) return result;
	
//	positive
	if(index > 0)
		{
		if(index > r0 && index <= r1)
			{
			result = c0 + (c1-c0)*(index-r0)/(r1-r0);
			}
		else if(index > r1 && index <= r2)
			{
			result = c1 + (c2-c1)*(index-r1)/(r2-r1);
			}	
		else if(index > r2 && index <= r3)
			{
			result = c2 + (c3-c2)*(index-r2)/(r3-r2);
			}	
		}
	
//	negative
	else
		{
		index = -index;

		if(index > r0 && index <= r_1)
			{
			result = c_0 + (c_1-c_0)*(index - r0)/(r_1 - r0);
			}
		else if(index > r_1 && index <= r_2)
			{
			result = c_1 + (c_2-c_1)* (index - r_1)/(r_2 - r_1);
			}
		else if(index > r_2 && index <= r_3)
			{
			result = c_2 + (c_3-c_2)*(index-r_2)/(r_3-r_2);
			}
		}
	
	result += c0*c0_weight;
	return result;
	}



void	DetectLinearPart_temp(RealFunctionF32 &f, double *a0, double *a1)
	{
	size_t	k, N = f.size();
	double	sk(0), skfk(0), sfk(0), sk2(0);
	for(k = 0; k < N; k++)
		{
		sk += k;
		sk2 += k*k;
		sfk += f[k];
		skfk += k*f[k];
		}
	*a1 = (N*skfk - sk*sfk)/(N*sk2 - sk*sk);
	*a0 = (sfk - *a1*sk)/N;
	
	RealFunctionF32	f0(f.size());
	}



void	Attenuator :: DisplayColour()
	{
//	int	i,j,k;
	ColorScanConverter	SC(n_rays, n_samples);
	GrayScanConverter	b_img(n_rays, n_samples);
//	b_img.Attach(this);
//	SC.Attach(this);
	
	ExportScanConverterOptions(b_img);
	ExportScanConverterOptions(SC);
//	size_t	sub = 0;

	for(size_t i = 0; i < n_rays; i++)
		{
		ComplexFunctionF32	CurrentRay;
//		SetCurrentRay(i);
		for(size_t j = 0; j < n_samples; j++)
			{
			complexF32 acc(0,0);
			for(size_t k = 0; k < n_frames; k++)
				{
//				SetCurrentFrame(k);
				focused_data.GetRow(CurrentRay, {k, i, slice_mask(0)});
				acc += CurrentRay[j];
				}
			b_img.at(i,j) += cabs(acc);
			}
		}
		
//	double	dynRange = GetFloating("Dynamic range for colour image (0 = auto)", 0, 0, HUGE_VAL);

//	b_img.SmoothRays(0.025);//approx. lambda/2
	LogCompress(b_img);
	double	black_point = GetFloating("Black point level", 0, 0, 1);
	NormalizeImage(b_img, black_point, 1.);
	b_img.at(0,0) = 0;
	
	//double	coef0 = 2.67;
	//double	coef_dev = 1;
	
//	double	coef0 = 2.67;
//	double	coef_dev = 1;
	
	
	if (YesOrNo("Correct linear part", true))
		{
		RealFunctionF32	f(n_samples);
		for(size_t i = 0; i < n_rays; i++) f += Coefficient_Map->row(i);
		f /= double(n_rays);
		double	a0, a1;
		DetectLinearPart_temp(f, &a0, &a1);
		ShowFloating("Frequency deviation is", a1*n_samples);
		for(size_t i = 0; i < n_rays; i++)
			{
			for(size_t j = 0; j < n_samples; j++)
				{
				(*Coefficient_Map).at(i,j) -= j*a1;
				}
			}
		}
	double	coef0 = AverageValue(*Coefficient_Map);
	double	coef_dev = 0;/*10*Coefficient_Map->StandardDeviation();*/
	
	coef0 = GetFloating("Center frequency", coef0, 0, 6);
	coef_dev = GetFloating("Frequency deviation", coef_dev, 0, 3);
	double	dev_power = GetFloating("Deviation power", 0.67, 0, HUGE_VAL);
	c0_weight = GetFloating("Desaturation", 1, 0, HUGE_VAL);
	
	for(size_t i = 0; i < n_rays; i++)
		{
		for(size_t j = 0; j < n_samples; j++)
			{
			//double	x = 2*((*Coefficient_Map).at(j,i) - 2.72);
			double	x = 2*((*Coefficient_Map).at(i,j) - coef0);
			
			x /= coef_dev;			
			x /= pow(fabs(x), 1.-dev_power);
			
			SC.at(i,j) = color_palette(-x, 1) * b_img.at(i,j);
			}
		}

	NormalizeImage(SC, 0,255);
	DisplayMathFunction2D(SC.GetConvertedImage(), "SC");
	}

void	CalculateSimultaneousFrequency(ComplexFunctionF32 &f, RealFunctionF32 &r)
	{
	size_t	s0 = r.size();
	size_t	s1 = f.size();
	size_t	i, j;
	if(s0 > s1 || !s0 || !s1)
		{
		Error(ssprintf("Improper array sizes (%lu, %lu)!", s0, s1));
		return;
		}
	size_t	step = s1/s0;
	size_t	k_step = 3;
	
	for(i = 0; i < s0 - k_step; i++)
		{
		complexF32 acc(0);
		size_t	i0 = i*step;
		for(j = 0; j < step*k_step; j++)
			{
			if(!i && !j) j=1;
			acc += f[i0+j]%f[i0+j-1];
			}
		r[i] = -arg(acc);
		}
	for(i = s0 - k_step; i < s0; i++) r[i] = r[s0-k_step-1];
	}

void	ComputeZeros(const ComplexFunctionF32 &f0, RealFunctionF32 &r, size_t n){

	ComplexFunctionF32	f(f0);

	size_t	s0 = r.size();
	size_t	s1 = f.size();
	size_t	i_ratio = s1/s0;

//	int	i, j;
	if(r.size() > s1)
		{
		Error(ssprintf("Improper array sizes (%lu > %lu)!", r.size(), s1));
		return;
		}
	if(n > s1/2)
		{
		Error(ssprintf("Too large n (%lu > %lu/2)!", n, s1));
		return;
		}
	
	r.fill(0);
	
	int	n_shifts = 1;
	complexF32 phase_shift = polar(1, pi()/n_shifts);
	int	sh;
	
	for(sh = 0; sh < n_shifts; sh ++)
		{
		for(size_t i = 1; i < s1-1; i ++)
			{
			size_t	i0 = i/i_ratio;
			for(size_t j = 0; j < n; j++)
				{
				size_t 	k = i+j;
				if(k >= s1) k -= n;
				
				complexF32 v1 = f[k-1];
				complexF32 v2 = f[k];
	
				if(v1.re >= 0 && v2.re < 0) r[i0] ++;
				else if(v1.re <=0 && v2.re > 0) r[i0] ++;
				
				if(v1.im >= 0 && v2.im < 0) r[i0] ++;
				else if(v1.im <=0 && v2.im > 0) r[i0] ++;
	
				}
			}
		f *= phase_shift;
		}

	r[0] = r[1];
	r[s0-1]=r[s0-2];
	r *= pi()/(n*n_shifts);
	}



void	Attenuator :: Batch()
	{
	if(YesOrNo("Read pre-computed coefficient map?", true))
		{
		ReadCoefMap();
		return;
		}


//	int	i,j;
	size_t	phase_drv = Decide2("Calculation algorithm", "Crosszero", "Phase drv.", 0);
	int	n_samps_zero = 32;
	int	iFactor = 16;
	if(!phase_drv)
		{
		n_samps_zero = GetSigned("N samps to compute zeros", n_samps_zero, 1, n_samples/2);
		iFactor = 2;
		}
	else iFactor = GetSigned("Interpolation factor", 16, 2, 64);
	
	size_t	ns2 = n_samples * iFactor;
	ComplexFunctionF32 buffer(n_samples), iBuffer(ns2);
	RealFunctionF32	rBuffer(n_samples);
	
	RealFunction2D_F32	result(n_rays, n_samples);
	
	//buffer.InitInterpolator(iFactor, -0.5);
	
	GUIProgressBar	progress;
	progress.start("Computing coefficient", n_rays);
	for(size_t ray = 0; ray < n_rays; ++ray)
		{
//		SetCurrentRay(ray);
		size_t	i = ray;
//		int	sub;
		RealFunction2D_F32	pd(n_frames, n_samples);
		
		
		for(size_t sub = 0; sub < n_frames; sub++)
			{
//			SetCurrentFrame(sub);
			ComplexFunctionF32	CurrentRay;
			focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
			
			buffer.CopyData(CurrentRay);
			ComplexFunctionF32 DCBuffer(buffer);
			DCBuffer.FilterGauss(3);
			buffer -= DCBuffer;
			
			if(iFactor > 1) for(size_t j = 0; j < ns2; j ++)
				{
				double	x = double(j)/iFactor;
				iBuffer[j] = buffer.in(x, &interpolators::complex_sincT);
				}
			else iBuffer.CopyData(buffer);
			
			if(CapsLock())
				{
				DisplayMathFunction(buffer, 0, 1, "Before interpolation");
				DisplayMathFunction(iBuffer, 0, 1./iFactor, "After interpolation");
				}
	
			if(phase_drv)
				{
				CalculateSimultaneousFrequency(iBuffer, rBuffer);
				//SetFrequencyUnits(M_HERZ);
				rBuffer *= double(iFactor)*sample_rate.MHz()/two_pi();
				// интерполЯциЯ по-разному влиЯет на результат 
				// при подсчете нулей и при анализе фазовой производной
				}
			else
				{
				ComputeZeros(iBuffer, rBuffer, iFactor*n_samps_zero);
				//SetFrequencyUnits(M_HERZ);
				rBuffer *= sample_rate.MHz()/(two_pi()*double(iFactor));
				// см. комментарий выше
				}
	
			if(CapsLock()) DisplayMathFunction(rBuffer, 0, 1., "Frequency");
			if(1) if(phase_drv)
				{
				rBuffer.FilterMedian(3);
				rBuffer[0] = rBuffer[1];
				if(CapsLock()) DisplayMathFunction(rBuffer, 0, 1., "Frequency after median");
				}
	
			rBuffer.FilterGauss(3, 0.3, extrapolation::by_last_value);
	
			if(CapsLock()) DisplayMathFunction(rBuffer, 0, 1., "Frequency after Gauss");
			for(size_t j = 0; j < n_samples; j++)
				{
				pd.at(sub,j) = rBuffer[j];
				}
			}


		++progress;
		for(size_t j = 0; j < n_samples; j++)
			{
			if(ControlPressed()) DisplayMathFunction(pd.col(j), 0,1,"PD column");
			if(1){// average value
				result.at(i,j) = AverageValue(pd.col(j));
				}
			else if (0){ // median
				reorder_ascent(pd.col(j));
				result.at(i,j) = pd.col(j)[n_frames/2];
				}
			else{// std. deviation
				double	 ave = AverageValue(pd.col(j));
				double	dev(0);
				for(size_t s = 0; s < n_frames; s++) dev += square(pd.at(s,j) - ave);
				result.at(i,j) = sqrt(dev/n_frames);
				}
			
			//result.at(i,j) = pd.col(j)[0];
			}
		}
	progress.end();
	
	FIRFilterKernel2DMask<bool>	theFilter;
	theFilter.InitFilter(square3, 0.5);
	
	result.Filter(theFilter);
	//result.Display("Attenuation coeff. filtered");
	//theFilter.IFilterGaussian(22);
	//result.Filter(theFilter);
	//result.Display("Attenuation coef. filtered gauss");
	
	CutHistogramEdges(result, range1_F64(0.001, 0.999));
	
	for(size_t i = 0; i < n_rays; i++)
		{
		for(size_t j = 0; j < n_samples; j++)
			{
			(*Coefficient_Map).at(i,j) = result.at(i,j);
			}
		}
	if(YesOrNo("Save computed coefficient map?", true)) SaveCoefMap();
	}

/*
Previous frequency determination
{
		for(Interval = 0; Interval < n_intervals; Interval ++)
			{
			double	Coefficient, Frequency, Last_Frequency;
			From_Sample = Interval * interval_increment/2;
			To_Sample = From_Sample + interval_increment;
			Frequency = Process_Interval();
			if(!Interval) Last_Frequency = Frequency;
			SetFrequencyUnits(M_HERZ);
			SetDepthUnits(CENTIMETRES);
			
			//Coefficient = 8.686 * (Last_Frequency - Frequency)*
			//	(n_intervals+1.)/(2.* band_half_width * band_half_width * (r_max - r_min));
			
			Coefficient = Frequency;
			(*Coefficient_Map)[Interval-1][RayNo()] =  Coefficient;
			Last_Frequency = Frequency;
			}
		++progress;
		}

*/

complexF32 *IntervBuffer = NULL;
int	Interp = 4;

double	Attenuator :: Process_Interval()
	{
//	int	Subaperture, i;
	double	Result = 0, fSample;
	double	res, lastRes = 0;
	
	if(!IntervBuffer) IntervBuffer = (complexF32 *)calloc(interval_increment*Interp, sizeof(complexF32));
	
	size_t	ray = 0;

	for(size_t Subaperture = 0; Subaperture < n_frames; Subaperture ++)
		{
//		SetCurrentFrame(Subaperture);
		size_t i;
		for(fSample = From_Sample, i = 0; fSample < To_Sample; i++, fSample +=1./Interp)
			{
			//IntervBuffer[i] = Intrrr -> Interpolate(CurrentRay[Subaperture], fSample);
//			IntervBuffer[i] = CurrentRay.in(fSample, positive_oscillation);
			ComplexFunctionF32	CurrentRay;
			focused_data.GetRow(CurrentRay, {Subaperture, ray, slice_mask(0)});

		}
		//grafc_f(CurrentRay[Subaperture] + From_Sample, interval_increment, 0, 1, "Before", "");
		//grafc_f(IntervBuffer, interval_increment*Interp, 0, 1, "After", "");
		for(i = 1; i < interval_increment * Interp; i ++)
			{
			complexF32 x, y, z;
			x = IntervBuffer[i];
			y = IntervBuffer[i-1];
			z = x%y;
			 
			res = -arg(z)/(interval_increment*n_frames);
			if(i > 1) Result += two_pi()*8.686*(res - lastRes);
			//if(i > 1) Result += res;
			lastRes = res;
			}
		}
	return	Result;
	}
	
double	Attenuator :: Frequency(double value){// Non-modified frequency in MHz
	double	Orig_Table[] = {1.69, 1.73,	1.85,	1.97,	2.09,	2.21,	2.33,	2.41, 2.44, 2.46, 2.55};
	double	Res_Table[] = {1.65, 1.8,	1.95,	2.1,	2.25,	2.4,	2.55,	2.7, 2.85, 3.0, 3.15};
const	int	n_Items = 11;
	int	i = 0;
	double	result(0);
	
	do
		{
		i++;
		}while(value > Orig_Table[i] && i < n_Items-1);
	if(i == 0) result = value;
	if(i < n_Items - 1 && i > 0) result = Res_Table[i] + (Res_Table[i+1] - Res_Table[i])*
				(value - Orig_Table[i])/(Orig_Table[i+1] - Orig_Table[i]);
				
	if(i == n_Items - 1) result = Res_Table[i-1] + (Res_Table[i] - Res_Table[i-1])*
				(value - Orig_Table[i-1])/(Orig_Table[i] - Orig_Table[i-1]);
	return	result;
	}

/*
double	Attenuator :: Frequency(double value){// Non-modified frequency in MHz
	double	delta0 = 0, omega_g, deviation = 0, factor;
	double	delta_t = 0, tauI;
	double	Min_Cryterion;
	
	value *= 2.* pi() * 1.e6;		// To sec-1
	omega_g = value;
	SetFrequencyUnits(BACK_SECONDS);
	delta_t = two_pi()/sample_rate;
	tauI = 2./band_half_width;
	
	
	factor = 2.*delta_t/tauI;
	(*
	do
		{// for Gaussian shape
		delta0 = factor * factor/tan(2*omega_g*delta_t);
		deviation = omega_g - (value - .5*delta0/delta_t);
		omega_g = value - .5*delta0/delta_t;
		}while(fabs(deviation/(2.* pi() * 1.e6)) > .05 && (omega_g/(2.* pi() * 1.e6)) < 3.);	//MHz
	*)
	
	Min_Cryterion = fabs(Cryterion(0, value));
	for(delta0 = -.5 ; delta0 < .5; delta0 +=.005)
		{
		double	Crt = Cryterion(delta0, value - .5*delta0/delta_t);
		if(fabs(Crt) < Min_Cryterion)
			{
			Min_Cryterion = fabs(Crt);
			omega_g = value - .5*delta0/delta_t;
			}
		}
	
	
	return omega_g/(2.* pi() * 1.e6);
	}
*/

double	Attenuator :: Cryterion(double delta0, double omega_g){// delta0 is a phase; omega_g in rad/sec
	double	delta_t = 0, tauI;
	double	s, c, tg, x;
	double	power = 1.25;
	
	delta_t = two_pi()/sample_rate.rad_sec();
	tauI = 2./band_half_width.rad_sec();
	
	s = sin(delta0/2.);
	c = cos(delta0);
	tg = tan(2*omega_g*delta_t);
	x = 2*delta_t/tauI;
	x = exp(power*log(x));
	
	return	s*s + s*c*tg - .5*x;
	}
	
XRAD_END
