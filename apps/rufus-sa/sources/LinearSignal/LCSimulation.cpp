#include "pre.h"
#include <NANHandling.h>
#include <StatisticUtils.h>
#include <MathFunctionTypes.h>
#include "LCSimulation.h"
#include "LinearConvexFile.h"

physical_length	arraySize0 = cm(6);		//cm
physical_length	arraySize = arraySize0;	//cm
physical_length	depthRange = cm(1.5);	//cm

long	nPoints = 3;
physical_length	FX_Point = cm(6);

physical_length	centreZ = cm(6);


//Moving point:

physical_speed	speedX = cm_sec(0);
physical_speed	speedZ = cm_sec(0);

physical_frequency	LC_Simulator :: omega0 = MHz(3.0);		//MHz
physical_frequency	LC_Simulator :: halfWidth = MHz(1.5);		//MHz


//--------------------------------------------------
//
//	инициализациЯ констант
//
//--------------------------------------------------

void	LC_Simulator :: GenerateInitPulse()
	{
	initPulse.realloc(n_samples);
	for(int i = 0; i < n_samples; i++)
		{
		double	w = i*sampleRate.MHz()/n_samples;
		initPulse[i] = gauss(w-omega0.MHz(), halfWidth.MHz());
		}
	}

void	LC_Simulator :: ComputeDistances()
	{
	distances.realloc(n_rays, n_samples);
	for(int i = 0; i < n_rays; i++)
		{
		for(int j = 0; j < n_samples; j++)
			{
			float	x = i*arrayPitch.cm();
			float	z = rMin.cm() + j*(rMax-rMin).cm()/n_samples;
			distances[i][j] = sqrt(x*x + z*z) - rMin.cm();
			}
		}
	}

void	LC_Simulator :: ComputeLens()
	{
	int	ap2 = n_aperture_elements/2;
	lens.realloc(n_aperture_elements);
	using std::fabs;

	for(int ap_el = 0; ap_el < n_aperture_elements; ap_el++)
		{
		float	x = (ap_el - ap2 + .5)*arrayPitch.cm();
		lens[ap_el] = fabs(TX_Focus.cm()) - sqrt(x*x + square(TX_Focus.cm()));
		}
	if(TX_Focus.cm() < 0) lens *= -1;
	}

void	LC_Simulator :: ComputeApodization(const abstract_window_function &wf)
	{
	apod.realloc(n_aperture_elements);
	apod.fill(1);
	ApplyWindowFunction(apod, wf);
	}

void	LC_Simulator :: Init(physical_frequency carrierFrequency, physical_length apertSize)
	{
	using std::floor;
	physical_length	lambda;
	soundSpeed = cm_sec(1.5e5); 	//cm/sec
	omega0 = carrierFrequency;	//MHz
	halfWidth = omega0/6;
	sampleRate = omega0*2;

	lambda = cm(soundSpeed.cm_sec()/omega0.Hz());

	arrayPitch = lambda*arrayQuotient;
	n_array_elements = ceil_fft_length(floor(arraySize/arrayPitch));
	arraySize = arrayPitch*n_array_elements;

	n_aperture_elements = apertSize/arrayPitch;

	n_samples = ceil_fft_length(4.*depthRange/lambda);

	convexRadius = cm(0);

	n_rays = n_array_elements;

	rMin = cm(5.2);
	rMax = rMin + 0.5*n_samples*cm(soundSpeed.cm_sec()/sampleRate.Hz());
	}

void	LC_Simulator :: PrepareData()
	{
//	set frequency range
	omega0 = MHz(GetDouble("Carrier Frequency (MHz)", omega0.MHz(), 0, inf()));
	sampleRate = omega0*2;
	halfWidth = omega0/6;

	physical_length	lambda = cm(soundSpeed.cm_sec()/omega0.Hz());

//	set array pitch
	arrayQuotient = GetDouble("Array step (lambdas/\?)", arrayQuotient, .25, 4);
	arrayPitch = lambda*arrayQuotient;
	n_array_elements = 128;
	arraySize = n_array_elements*arrayPitch;
	physical_length apertSize = cm(GetDouble("Aperture size", 16*arrayPitch.cm(), arrayPitch.cm(), arraySize.cm()));

	n_aperture_elements = apertSize/arrayPitch;

	depthRange = cm(GetDouble("Min. depth range required", 32.*lambda.cm(), 8*lambda.cm(), inf()));
	centreZ = cm(GetDouble("Distance to central point:", 6, 0, inf()));

	n_samples = ceil_fft_length(4.*depthRange/lambda); //step = lambda/4
	convexRadius = cm(GetDouble("Convex radius (0 for linear array)", 0, 0, 10));


	rMin = centreZ - 0.5*n_samples*lambda/4;
	rMax = rMin + n_samples*lambda/4;

	TX_Focus = RX_Focus = FX_Point =
		cm(GetDouble("Focal point", FX_Point.cm(), -HUGE_VAL, HUGE_VAL));

	TX_Focusing = YesOrNo("Do TX Focusing?", YES);
	RX_Focusing = YesOrNo("Do RX Focusing?", YES);

	nRepeats = GetLong("Doppler repeats", 1, 1, n_samples/2); // количество повторов длЯ „оплера
	n_rays = n_array_elements*nRepeats;

	realloc(n_rays, n_samples);

//	initialize constants

	GenerateInitPulse();
	ComputeDistances();
//	ComputeApodization(gauss_win);
	ComputeApodization(constant_win());
	ComputeLens();
	}

physical_length	LC_Simulator :: ComputeDistance(short element, physical_length deltaX, physical_length pointZ)
	{
	physical_length	lens;
	physical_length	distance;

	if(!convexRadius.cm())
		{
		if(RX_Focusing < 0 || TX_Focusing < 0)
			lens = -cm((square(element*arrayPitch.cm()))/(2*pointZ.cm()));
		else if(RX_Focus.cm()) lens = -cm(square(element*arrayPitch.cm())/(2*RX_Focus.cm()));
		else lens = cm(0);

		distance = lens + cm(sqrt(square(pointZ.cm())+ square(deltaX.cm())));
		}

	else
		{
		physical_length	initCurve = -cm(square(element*arrayPitch.cm())/(2*convexRadius.cm()));
		physical_angle	angle = radians(deltaX/convexRadius);
		physical_length	pointDistance = cm(sqrt(
			square(convexRadius.cm())+
			(convexRadius + pointZ).cm()*(convexRadius + pointZ).cm() -
			2*(convexRadius + pointZ).cm()*convexRadius.cm()*cosine(angle))); //NB: теорема косинусов

		if(RX_Focusing < 0 || TX_Focusing < 0)
			lens = -cm(square(element*arrayPitch.cm())/(2*pointZ.cm()));
		else if(RX_Focus.cm()) lens = -cm(square(element*arrayPitch.cm())/(2*RX_Focus.cm()));
		else lens = cm(0);
		distance = lens + initCurve + pointDistance;
		}
	return distance;
	}
#pragma message ("ATTENTION!")

#if 0
void	LC_Simulator :: Compute()
	{
	PrepareData();
	short	movingPoint = YesOrNo("Moving point?", YES);

	nPoints = GetLong("Number of scattering points:", 1, 1, 7);
	
	MathFunction<physical_length, double>	pointsX(nPoints), pointsZ(nPoints);
	
	for(int i = 0; i < nPoints; ++i)
		{
		pointsX[i] = cm(RandomUniform(2,7));
		pointsZ[i] = cm(RandomUniform(2,7));
		}
	
	long	i, TX, RX, ray;
	long	ns2 = nSamples/2;
	bool	doApodization = YesOrNo("Do apodization\?", YES);


	physical_frequency	dW = sampleRate/nSamples;
	ComplexFunctionF32	currentPulse(nSamples);


	convexRadius = cm(GetDouble("Convex radius (cm)", 0, 0, HUGE_VAL));
	TX_Focus = RX_Focus = FX_Point;

	if(movingPoint)
		{
		speedX = cm_sec(GetDouble("Point speed X (cm/s)", 0, -HUGE_VAL, HUGE_VAL));
		speedZ = cm_sec(GetDouble("Point speed Z (cm/s)", 0, -HUGE_VAL, HUGE_VAL));
		}

	sprintf(comment, "Synthesized by Fresnel algorithm.");
	if(movingPoint) sprintf(comment, "%s\nPoint speed X = %3.1f, Point speed Y = %3.1f;",
		comment, speedX, speedZ);

	realloc(nRays, nSamples);
//	Allocate();

	RealFunctionF32 apodizationFunc(nApertElements);
	float	*ap = &apodizationFunc [nApertElements/2];//TODO как нехорошо!!!
//	apodizationFunc = CreatePointer(float, nApertElements);


//	for(i = 0; i < nSamples; i ++)
//		{
//		initPulse[i] = gauss(dW*i-omega0, halfWidth);
//		}
	for(i = -nApertElements/2; i < nApertElements/2; i++)
		{
		if(doApodization)ap[i] = gauss((float)i, (float)nApertElements/sqrt(8.));
		else ap[i] = 1;
		}

	if(doApodization)
		{
		DisplayMathFunction(apodizationFunc, -nApertElements/2, 1,
			"Apodization function", "Aperture element", "Factor");
		}


	StartProgress("Computing rays", nRays);

	physical_time	tau = 2*rMax/soundSpeed;
	pointsX[0] -= speedX*tau*nRays/2;
	pointsZ[0] -= speedZ*tau*nRays/2;

	for(ray = 0; ray < nRays; ray ++)
		{
		float	centreEl = (float)ray*(float)nElements/nRays;
		long	Point;
		if(movingPoint)
			{
			pointsX[0] += speedX*tau;
			pointsZ[0] += speedZ*tau;

			}
		for(Point = 0; Point < nPoints; Point ++)
			{
			physical_length	x = pointsX[Point];
			physical_length	y = (pointsZ[Point] + centreZ);
			long	firstTX, lastTX;
			if(TX_Focusing)
				{
				firstTX = -nApertElements/2;
				lastTX = nApertElements/2;
				}
			else
				{
				firstTX = 0;
				lastTX = 1;
				}

			for(TX = firstTX; TX < lastTX; TX ++)
				{
				float	TXel;
				physical_length xTX;
				physical_length lensTX;
				physical_length distTX;
				long	firstRX, lastRX;

				if(RX_Focusing)
					{
					firstRX = -nApertElements/2;
					lastRX = nApertElements/2;
					}
				else
					{
					firstRX = -1;
					lastRX = 1;
					}

				TXel = range(centreEl + TX, 0, nElements);

				xTX = x - arrayPitch*TXel;
				distTX = ComputeDistance(TX, xTX, y);

				for(RX = firstRX; RX < lastRX; RX ++)
					{
					float	RXel;
					float	xRX;
					float	distRX;
					float	dist;


					RXel = range(centreEl + RX, 0, nElements);
					xRX = x - arrayPitch*RXel;
					distRX = ComputeDistance(RX, xRX, y);

					dist = distTX + distRX - 2.*rMin;

					if(dist < 2.*(rMax-rMin) && dist > 0)
					for(i = 0; i < nSamples; i ++)
						{
						float	W = dW*i*tau()*1e6*dist/soundSpeed;
						float	attenuation;
						if(distTX && distRX) attenuation = centreZ*centreZ/(distTX*distRX);
						else attenuation = 0;

						th[ray][i] +=
							initPulse[i]*polar(attenuation*
							pointsB[Point]*
							ap[TX]*ap[RX], W);
						}
					NextQMEvent();
					}
				}
			}
		NextProgress();
		}
	EndProgress();
	StartProgress("Computing temporary signal", nRays);

	for(ray = 0; ray < nRays; ray ++)
		{
		for(i = 0; i < nSamples; i ++)
			{
			currentPulse[i] = th[ray][i];
			}
		FFT(currentPulse, ftReverse);
		for(i = 0; i < nSamples; i ++)
			{
			th[ray][i] = currentPulse[i];
			}
		NextProgress();
		}
	EndProgress();


	if(!TX_Focusing && !RX_Focusing)
		{
		//SynthAperture();
		}
	DestroyPointer(apodizationFunc);
	}

void	LC_Simulator :: ComputeNearZone()
	{
	PrepareData();
	bool	doApodization;
	nPoints = GetLong("Number of scattering points:", 7, 1, 7);
	if(TX_Focusing || RX_Focusing)
		doApodization = YesOrNo("Do Apodization?", YES);
	else	doApodization = 0;


	short	nHarm = ceil_fft_length(nElements);
	short	centreHarmS = nHarm/2;
	short	ray, harmS, harmT, el;
	float	*initDelays = (float *)calloc(nHarm, sizeof(float));
	float	*aperture = (float *)calloc(nHarm, sizeof(float));

	ComplexFunctionF32	initPulseTX(nHarm);
	ComplexFunctionF32	initPulseRX(nHarm);
	ComplexFunctionF32	pointsBufferTX(nHarm);
	ComplexFunctionF32	pointsBufferRX(nHarm);

	float	deltaOmega = tau()*1e6*sampleRate/nSamples;

	convexRadius = 0;


	for(el = -nApertElements/2; el < nApertElements/2; el ++)
		{
		float	x = el*arrayPitch;
		short	index;

		if(el >= 0) index = el;
		else index = nHarm + el;

		initDelays[index] = -(sqrt(x*x + FX_Point*FX_Point) - fabs(FX_Point));
		if(FX_Point <= 0) initDelays[index] = -initDelays[index];

		if(doApodization)
			{
			float	argument = 2.*(float)el/nApertElements;
			aperture[index] = exp(-argument*argument);
			}
		else	aperture[index] = 1;
		}

	sprintf(comment, "Synthesized by exact nearzone algorithm.");

	Start_Progress("Simulating", nSamples);
	float	*gFactorDisplay = new float[nSamples];

	for(harmT = 0; harmT < nSamples; harmT ++)
		{
		float	omega = harmT*deltaOmega;
		float	k = omega/soundSpeed;
		float	dKX = tau()/(arrayPitch*nHarm);
		float	gaussFactor = gauss((sampleRate*harmT)/nSamples-omega0, halfWidth);
		short	displaySample = nSamples/4;

		for(ray = 0; ray < nHarm; ray ++)
			{
			float	apTX, apRX;

			if(TX_Focusing || ray == 0) apTX = aperture[ray];
			else	apTX = 0;
			if(RX_Focusing || ray == 0) apRX = aperture[ray];
			else	apRX = 0;

			initPulseTX[ray] = polar(apTX, initDelays[ray]*k);
			initPulseRX[ray] = polar(apRX, initDelays[ray]*k);
			pointsBufferTX[ray] = 0;
			pointsBufferRX[ray] = 0;
			NextQMEvent();
			}

		FFT(initPulseTX, ftForward);
		FFT(initPulseRX, ftForward);

		for(harmS = 0; harmS < nHarm; harmS ++)
			{
			float	kX;
			float	kZ2;
			float	kZ, dissipZ;

			if(harmS <= centreHarmS) kX = harmS*dKX;
			else	kX = (harmS - nHarm)*dKX;

			kZ2 = k*k - kX*kX;

			if(kZ2 >= 0)
				{
				kZ = sqrt(kZ2);
				dissipZ = 0;
				}
			else
				{
				kZ = 0;
				dissipZ = sqrt(-kZ2);
				}

			float	pZ = centreZ;

			pointsBufferTX[harmS] = exp(-dissipZ*pZ) * polar(1, kZ*pZ);

			pointsBufferRX[harmS] = pointsBufferTX[harmS];

			pointsBufferTX[harmS] *= initPulseTX[harmS];
			pointsBufferRX[harmS] *= initPulseRX[harmS];
			NextQMEvent();
			}

		FFT(pointsBufferRX, ftReverse);
		FFT(pointsBufferTX, ftReverse);

		for(harmS = 0; harmS < nHarm; harmS ++)
			{
			pointsBufferRX[harmS] *= pointsBufferTX[harmS];
			}

		FFT(pointsBufferRX, ftForward);

		for(harmS = 0; harmS < nHarm; harmS ++)
			{
			float	kX;
			short	point;
			complexF32 pointsFactor = 0;

			if(harmS <= centreHarmS) kX = harmS*dKX;
			else	kX = (harmS - nHarm)*dKX;

			for(point = 0; point < nPoints; point ++)
				{
				float	pX = pointsX[point];
				pointsFactor += polar(pointsB[point], kX*pX - 2.*k*rMin.cm());
				}

			pointsBufferRX[harmS] *= pointsFactor;
			NextQMEvent();
			}
		gFactorDisplay[harmT] = gaussFactor;

		FFT(pointsBufferRX, ftReverse);


		//if(harmT == displaySample) grafc_f(pointsBuffer, nHarm, 0, arrayPitch, "Final pulse", "");
		for(ray = 0; ray < nRays; ray ++) at(ray,harmT) = pointsBufferRX[ray]*gaussFactor;
		NextProgress();
		}
	EndProgress();
	//grafit_f(gFactorDisplay, nSamples, 0, 1, "gFactor", "", "");

	for(ray = 0; ray < nRays; ray ++)
		{
		FFT(row(ray), ftReverse);
		}
	Display("Simulated");
	}

#endif
