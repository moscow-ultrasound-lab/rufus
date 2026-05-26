#include "pre.h"

#include "LinearSignalProcessing.h"
#include <StatisticUtils.h>



void	LinearSignalProcessor :: ExportNonDiffractionSetup(non_diffraction_setup &s, int n_focusing_intervals)
	{
	//TODO важное изменение: раньше все глубины считались от поверхности датчика, теперь от его центра кривизны. поэтому поправка
	s.r_min = rMin - convexRadius;
	s.r_max = rMax - convexRadius;
	s.n_elements = n_array_elements;
	s.n_aperture_elements = n_aperture_elements;
	s.array_pitch = arrayPitch;
	s.convex_radius = convexRadius;
	if(!(n_focusing_intervals%2)) ++n_focusing_intervals;

	s.original_focuses.realloc(n_focusing_intervals, TX_Focus);
	s.target_focuses.realloc(n_focusing_intervals);
	
	physical_length	interval_step = (s.r_max-s.r_min)/(n_focusing_intervals+1);
	for(int i = 0; i < n_focusing_intervals; ++i)
		{
		s.target_focuses[i] = s.r_min + (i+1)*interval_step;
		}
	
	s.sample_rate = sampleRate; //MHz
	s.sound_speed = cm_sec(1.54e5);

	s.tx_focusing = TX_Focusing;
	s.rx_focusing = RX_Focusing;
	s.zeta_factor = -1;//GetDouble("Factor to Zeta:", -1, -4, 4);
	}




void	LinearSignalProcessor :: SynthAperture()
	{
	ComplexFunction2D Buffer(n_rays, n_rays);
	short	i , harmonic;
	short	ray;
       // float	deltaZ = (rMax-rMin)/n_samples;
	physical_frequency	deltaOmega = sampleRate/n_samples;
	
	DisplayFrame(0, "Before synthesize");
	
	
	StartProgress("Computing spectrum", n_rays);
	for(i = 0; i < n_rays; i++)
		{
		FFT(row(i), ftForward);
		NextProgress();
		}
	EndProgress();	
	
	physical_length z0 = (rMax + rMin)/2.;
	short	ftLen = ceil_fft_length(n_rays);
	ComplexFunctionF32	fBuffer(ftLen);
	ComplexFunctionF32	ftBuffer(ftLen);
	
	StartProgress("Focusing", n_samples);
	for(harmonic = 0; harmonic < n_samples; harmonic ++)
		{
		physical_frequency	W = deltaOmega*harmonic;
		
		for(ray = 0; ray < ftLen; ray ++)
			{
			physical_length	x = (ray - ftLen/2)*arrayPitch;
			physical_length	dZ = cm(sqrt(square(x.cm()) + square(z0.cm()))) - z0;
			//TODO откуда 2?
			fBuffer[ray] = polar(1., -2.*W.rad_sec()*dZ.cm()/soundSpeed.cm_sec());
			if(ray < n_rays)ftBuffer[ray] = at(ray,harmonic);
			else	ftBuffer[ray] = complexF32(0);
			}
		fBuffer.roll_half(true);
		FFT(ftBuffer, ftForward);
		FFT(fBuffer, ftForward);
		ftBuffer *= fBuffer;
//		for(ray = 0; ray < ftLen; ray ++) ftBuffer[ray] *= fBuffer[ray];
		FFT(ftBuffer, ftReverse);
		for(ray = 0; ray < n_rays; ray ++) at(ray,harmonic) = ftBuffer[ray];
		NextProgress();
		}
	EndProgress();	

	StartProgress("Computing signal", n_rays);
	for(i = 0; i < n_rays; i++)
		{
		FFT(row(i), ftReverse);
		NextProgress();
		}
	EndProgress();
	
	DisplayFrame(0, "Synthetic aperture");
	}

void	LinearSignalProcessor :: OriginalFocusAlgorithm(){
//	float	newFocus = (rMax + rMin)/2;	
	physical_length	newFocus = rMin + (rMax - rMin)/4;
	
	long	harmT, harmS;
	long	ftSize = ceil_fft_length(n_rays);
	long	Bound = (ftSize - n_rays)/2;
	ComplexFunctionF32	FFT_Buffer(ftSize);

	physical_length	arrayWidth = n_array_elements*arrayPitch;
	
	physical_length	dX = arrayWidth/n_rays;
	float	strobWidth = 0;
	physical_length	arraySize = arrayPitch*n_array_elements;
	short	useExactFormula, useStrob;
	

	float	zetaFactor = GetDouble("Factor to Zeta:", 1, -4, 4);
	if(convexRadius.cm()) useExactFormula = 0;
	else	useExactFormula = YesOrNo("Use exact formula?", NO);
	
	if(TX_Focusing || RX_Focusing) useStrob = 0;
	else	useStrob = YesOrNo("Use strob?", YES);
	if(useStrob) strobWidth = GetDouble("Strob width (cm):", arraySize.cm()/3, arrayPitch.cm(), HUGE_VAL);
	
	//TODO здесь напутано
//	if(convexRadius.cm()) dX = arrayWidth/(convexRadius*n_rays);
	// ‚ случае конвексной решетки используетсЯ полЯрнаЯ система координат.
		
	
//	tmpFT(ftForward);
	for(int ray = 0; ray < n_rays; ray++)
		{
		FFT(row(ray), ftForward);
		}
	
	
	StartProgress("Focusing", n_samples);
	for(harmT = 0; harmT < n_samples; harmT ++)
		{
		physical_length	distance = newFocus;
		float	Zeta;
		float	Strob;
		float	k;
		physical_length	oldFocus = TX_Focus;
//		float	W = two_pi()*1.e6*sampleRate*harmT/n_samples;
		physical_frequency W = sampleRate*harmT/n_samples;
		
		if(distance == cm(0)) distance = cm(0.001);
		k = W.rad_sec()/soundSpeed.cm_sec();

		
		for(harmS = 0; harmS < ftSize; harmS ++)FFT_Buffer[harmS] = 0;
		for(int ray = 0; ray < n_rays; ray ++)
			{
			FFT_Buffer[ray + Bound] = at(ray,harmT);
			}

		FFT_Buffer.roll_half(true);
		FFT(FFT_Buffer, ftForward);
		FFT_Buffer.roll_half(true);

			
		if(!useExactFormula)
			{
			float	beta1, beta2;
			if(!convexRadius.cm())
				{
				beta1 = -0.5*k/(oldFocus.cm() - distance.cm());
				beta2 = 0.5*k/distance.cm();
				}
			else{
				physical_length	fullDistance = convexRadius + distance;
				physical_length	fEquiv = convexRadius*(oldFocus/(convexRadius + oldFocus));
				physical_length	rEquiv = convexRadius*(distance/fullDistance);
				double	factor = square(convexRadius.cm());

				beta1 = -0.5*k/(fEquiv.cm() - rEquiv.cm());
				beta2 = 0.5*k/rEquiv.cm();
				
				beta1 *= factor;
				beta2 *= factor;
				}
			
			if(TX_Focusing && RX_Focusing) beta2 = beta1;
			if(!TX_Focusing && !RX_Focusing) beta1 = beta2;
								
			if(W.Hz()) Zeta = 0.25/(beta1 + beta2);
			else Zeta = 0;
			}
		else{
			physical_length a = .5*arrayPitch*n_aperture_elements;
			float	a4 = pow(a.cm(), 4);//*a*a*a;
			
			if(TX_Focusing && RX_Focusing)
				{
				float	k2F = k/(2*oldFocus.cm());
				float	k2Z = k/(2*distance.cm());
				float	v0 = k2F + 1./(a4*k2F);
				
				float	Up = k2Z - v0;
				
				if(k) Zeta = -Up/(4.*2.*k2Z*v0);
				else Zeta = 0;
				}
			
			else if(TX_Focusing || RX_Focusing)
				{
				float	p1 = 1./distance.cm() - 2./oldFocus.cm();
				if(k) Zeta = .25*k*p1/(k*k*p1*p1 + 16/a4) + distance.cm()/(4*k);
				else Zeta = 0;
				}
			
			else FatalError("No formula for synthetic aperture!");
			}
		
		Zeta *= zetaFactor;
		
		if(!useStrob || !k) Strob = 0;
		else{
			complexF32 spatial = complexF32(-1./strobWidth, -k/distance.cm());
			complexF32 freq = polar(.25/cabs(spatial), -arg(spatial));
			
			Zeta = freq.im;
			Strob = freq.re;
			}
		
		for(harmS = 0; harmS < ftSize; harmS ++)
			{
			float	OMEGA = (harmS - ftSize/2.)*two_pi()/(dX.cm()*ftSize);
			float	omega2 = OMEGA*OMEGA;
			
			FFT_Buffer[harmS] *= polar(exp(Strob*omega2),Zeta*omega2);
			}
		

		FFT_Buffer.roll_half(true);
		FFT(FFT_Buffer, ftReverse);
		FFT_Buffer.roll_half(true);
		
		for(int ray = 0; ray < n_rays; ray ++)
			{
			at(ray,harmT) = FFT_Buffer[ray + Bound];
			}
		NextProgress();
		}
	EndProgress();
	
//	tmpFT(ftReverse);
	for(int ray = 0; ray < n_rays; ray++)
		{
		FFT(row(ray), ftReverse);
		}
	}



/*

ђаботающий упрощенный вариант

void	LinearSignalProcessor :: Focus()
	{
	float	newFocus = rMin + (rMax - rMin)*.8/5.2;
	float	oldFocus = TX_Focus;
	long	harmT, harmS;
	
	float	dX = arrayPitch;
	

	zetaFactor = GetDouble("Factor to Zeta:", sqrt(PI), 0.5, 2);	
		
	long	i;
	for(i = 0; i < n_rays; i++) th[i].FFT(ftForward);
	
	
	Start_Progress("Focusing", 3*n_samples);
	for(harmT = 0; harmT < n_samples; harmT ++){

		Columns[harmT].roll();		
		Columns[harmT].FFT(ftForward);
		Columns[harmT].roll();
		Next_Progress();
		}
		
		
	for(harmT = 0; harmT < n_samples; harmT ++)
		{
		float	Zeta;
		float	W = two_pi()*1.e6*sampleRate*harmT/n_samples;
		if(W) Zeta = (newFocus - oldFocus)*soundSpeed/(4*W);
		else Zeta = 0;

		
		Zeta *= zetaFactor;
		
		for(harmS = 0; harmS < n_rays; harmS ++)
			{
			float	OMEGA = (harmS - n_rays/2.)*two_pi()/(dX*n_rays);
			float	omega2 = OMEGA*OMEGA;
			
			Columns[harmT][harmS] *= polar(1,Zeta*omega2);
			}
		Next_Progress();
		}

	for(harmT = 0; harmT < n_samples; harmT ++)
		{
		Columns[harmT].roll();
		Columns[harmT].FFT(ftReverse);
		Columns[harmT].roll();
		Next_Progress();
		}

	End_Progress();
	
	//tmpFT(ftReverse);
	for(i = 0; i < n_rays; i++) th[i].FFT(ftReverse);
	}

*/

void	LinearSignalProcessor :: Focus(){
//	float	newFocus = rMin + (rMax - rMin)/2;
	physical_length	oldFocus = TX_Focus;
	long	harmT, harmS;
	
	physical_length	dX = arrayPitch;
	

	float	zFactor = 0.5 * sqrt_pi() * GetDouble("Factor to Zeta (1 for synth. data, 2 for o'Brien phantom):", 1, 0.25, 4);
		
	long	i;

//ComplexFunctionF32 :: InitInterpolator(16, 0.5);

//long	oldRays = n_rays;
//long	oldSamples = n_samples;
//long	ftRays = ceil_fft_length(n_rays);
//long	ftSamples = ceil_fft_length(n_samples);

//realloc(ftRays, ftSamples);

//	Start_Progress("Focusing", 4*n_samples + 2*n_rays);
	for(i = 0; i < n_rays; i++)
		{
		FFT(row(i), ftForward);
//		Next_Progress();
		}
	
	
	for(harmT = 0; harmT < n_samples; harmT ++)
		{
		col(harmT).roll_half(true);
		FFT(col(harmT), ftForward);
		col(harmT).roll_half(true);
//		Next_Progress();
		}
		
	float	dW = sampleRate.rad_sec()/n_samples;
	for(harmT = 0; harmT < n_samples; harmT ++)
		{
		float	a = arrayPitch.cm()*n_aperture_elements/2;
		float	a4 = pow(a,4);
		
		float	z2;//, z2approx;
		float	W = harmT * dW;

		float	k2f = W/(2*soundSpeed.cm_sec()*oldFocus.cm());

		if(W) z2 = zFactor/((k2f + 1./(a4*k2f))*8);
		else z2 = 0;

		//if(W) z2approx = zFactor*oldFocus*soundSpeed/(4*W);
		//else z2approx = 0;
		
		for(harmS = 0; harmS < n_rays; harmS ++)
			{
			float	OMEGA = (harmS - n_rays/2.)*two_pi()/(dX.cm()*n_rays);
			float	omega2 = OMEGA*OMEGA;
			
			col(harmT)[harmS] *= polar(1,-z2*omega2);
			}
//		Next_Progress();
		}

/*

{
	//	вычислениЯ по старому алгоритму (без преобразованиЯ частотной координаты), не стирать!
	
			float	a = .5*arrayPitch*n_aperture_elements;
			float	a4 = a*a*a*a;
			
			if(TX_Focusing && RX_Focusing)
				{
				float	k2F = k/(2*oldFocus);
				float	k2Z = k/(2*distance);
				float	v0 = k2F + 1./(a4*k2F);
				
				float	Up = k2Z - v0;
				
				//if(k) Zeta = -Up/(4.*2.*k2Z*v0);
				//if(k) Zeta = 1./(4.*2.*k2Z) - 1/(4.*2.*v0);;
				if(k) Zeta = z*c/(4.*W) - 1/(4.*2.*v0);;
				else Zeta = 0;
				}
			
			else if(TX_Focusing || RX_Focusing)
				{
				float	p1 = 1./distance - 2./oldFocus;
				if(k) Zeta = .25*k*p1/(k*k*p1*p1 + 16/a4) + distance/(4*k);
				else Zeta = 0;
				}
			
			else Fatal_Error("No formula for synthetic aperture!");
			}

*/


	for(harmS = 0; harmS < n_rays; harmS ++)
		{
		ComplexFunctionF32	buffer(row(harmS));
		float	OMEGA = (harmS - n_rays/2.)*two_pi()/(dX.cm()*n_rays);
		RealFunctionF32	WD(n_samples);
		
		for(harmT = 1; harmT < n_samples; harmT ++){

			float	W2 = harmT * dW;
			float	D1 = zFactor*square(OMEGA * soundSpeed.cm_sec())/2;
			float	D = square(W2) + D1;
			if(D > 0) D = sqrt(D);
			else D = 0;
			float	W = 0.5 *(W2 + D);
			float	indexW = W/dW;
			
			float	z1 = zFactor*rMin.cm()*soundSpeed.cm_sec()/(4*W);
			
			complexF32 zFactor = polar(W/D, z1*square(OMEGA));
			
			if(D > 0) at(harmS,harmT) = buffer.in(indexW, &interpolators::complex_sincT) * zFactor;// * (1 + W2/D)/2;
			else at(harmS,harmT) = 0;
			}
		//Next_Progress();
		}

	for(harmT = 0; harmT < n_samples; harmT ++)
		{
		col(harmT).roll_half(true);
		FFT(col(harmT), ftReverse);
		col(harmT).roll_half(true);
		//Next_Progress();
		}

	
	//tmpFT(ftReverse);
	for(i = 0; i < n_rays; i++)
		{
		FFT(row(i), ftReverse);
		//Next_Progress();
		}
//	End_Progress();
//realloc(oldRays, oldSamples);
	}




void	LinearSignalProcessor :: AddNoise(float noiseLevelDB)
	{
	short	ray, i;	
	float	noiseLevel = pow(10,-noiseLevelDB/20);
	float	maxNoise = cabs(MaxValue(*this))*noiseLevel;
	
	for(ray = 0; ray < n_rays; ray ++)
		{
		for(i = 0; i < n_samples; i++)
			{
			float	n1 = RandomUniformF64(-maxNoise, maxNoise);
			float	n2 = RandomUniformF64(-maxNoise, maxNoise);
			at(ray,i) += complexF32(n1, n2);
			}
		}
	}





