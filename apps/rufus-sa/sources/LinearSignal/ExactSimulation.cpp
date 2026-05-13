#include "pre.h"
// #include "LimitedValue.h"
#include <StatisticUtils.h>
#include "LCSimulation.h"
#include "MovableMedia.h"


// static	float	df; // влиЯет на сужение полосы обработки, здесь временно

ComplexFunctionF32	LC_Simulator :: FocusRayExact(int r, scatterMedia &media)
	{
//	номера элементов решетки относительно апертуры:
	int	tx_ap_el, rx_ap_el;

//	координаты передающей и приемной апертур
	int	tx1, tx2, rx1, rx2;

//	частоты
	physical_frequency	dW = sampleRate/n_samples;
	
	int	ap2 = n_aperture_elements/2;
	int	rx_ap_shift = 0;
	double	max_distance = 2*distances[n_aperture_elements][n_samples-1];
		

	if(TX_Focusing) tx1 = 0, tx2 = n_aperture_elements;
	else tx1 = ap2, tx2 = ap2+1;

	if(RX_Focusing) rx1 = 0, rx2 = n_aperture_elements;
	else rx1 = ap2, rx2 = ap2+1;

	ComplexFunctionF32	result(n_samples, complexF32(0));

	
	// цикл по отражателЯм
	for(media.start(); media.end(); ++media)
		{
		NextQMEvent();
		float	x = range(media.x(), 0, n_array_elements-1);
		float	z = range(media.z(), 0, n_samples-1);

		// цикл по передающим элементам
		for(tx_ap_el = tx1; tx_ap_el < tx2; tx_ap_el++)
			{
			int	tx_el = range(r + tx_ap_el - ap2, 0, n_array_elements-1);			
			
			// цикл по приемным элементам
			for(rx_ap_el = rx1; rx_ap_el < rx2; rx_ap_el++)
				{
				int	rx_el = range(r + rx_ap_el - ap2 + rx_ap_shift, 0, n_array_elements-1);
				double	focus = lens[tx_ap_el] + lens[rx_ap_el];
				double	apodization = apod[tx_ap_el]*apod[rx_ap_el];
			
				double	distance = distances[labs(rx_el-x)][z] +
						distances[labs(tx_el-x)][z] +
						focus;

				if(distance < max_distance)
					{
					double	phase = dW.rad_sec()*distance/soundSpeed.cm_sec();
					complexF32	factor = polar(apodization, 0.);
					complexF32	phasor = polar(1., phase);

					//	цикл по частотам
//					for(int f = 0; f < n_samples; f++)
					for(ComplexFunctionF32::iterator it=result.begin(),ie=result.end(); it<ie; ++it)
						{
//						result[f] += (factor* *media);
// 						*it += (factor* *media);
						it->add_multiply(*media, factor);
						factor *= phasor;
						}
					}
				}
			}
		}
	result *= initPulse;
	FFT(result, ftReverse);
	
	return result;
	}
	
void	LC_Simulator :: SimulateSpeckleExact()
	{
	string	message;
	PrepareData();

	double	media_speed, speed_deviation;
	if(nRepeats > 1)
		{
		media_speed = GetDouble("Motion speed", 30, -HUGE_VAL, HUGE_VAL);
		speed_deviation = GetDouble("Speed deviation", media_speed/5, -HUGE_VAL, HUGE_VAL);
		comment = ssprintf("Data for colour doppler, repeat factor = %lu, speed = %g, deviation = %g", nRepeats, media_speed, speed_deviation);
		message = ssprintf("Simulating Doppler data, speed = %g", media_speed);
		halfWidth = omega0/10;
//		df = 3;
		GenerateInitPulse();
		}
	else
		{
		media_speed = 0;
		comment = ssprintf("Data for B-imaging");
		message = ssprintf("Simulating data for B-imaging");
		speed_deviation = 0;
//		df = n_samples;
		}
	
	printf("%s\n", comment); fflush(stdout);

	int	ray;
	scatterMedia	*media;

	if(Decide2("Media type", "Discrete", "Continuous", 1)) media = new ContinuousMedia(*this);
	else media = new DiscreteMedia(*this);
	
	media->Init(media_speed, speed_deviation);//cm/sec
	
	StartProgress(message, n_rays);
	for(ray = 0; ray < n_rays; ray++)
	{
//		int	doppler_ray = ray - ray%nRepeats + nRepeats/2;
		int	doppler_ray = ray/nRepeats;
		row(ray) = FocusRayExact(doppler_ray, *media);
		media->Move(0);
		NextProgress();
		}
	EndProgress();

//	DisplayFrame(0, "Result");
	DisplayMathFunction2D(*this, "Result", GetFrameDimensions(1));
	}



void	LC_Simulator :: SimulateSpeckle()
	{
	LinearSignal	speckle_pattern(*this);
	float	dH = soundSpeed.cm_sec()/(2*sampleRate.Hz());
	float	dV = arrayPitch.cm();
	float	width = n_rays * dV;
	float	height = n_samples * dH;
	float	r = min(width, height)/4;
	r = GetDouble("Speckle spot radius", r, 0, min(width, height)/2);
//	float	w = r/arrayPitch.cm();
//	float	h = 2*r*sampleRate.Hz()/soundSpeed.cm_sec();
	float	w0 = n_rays/2;
	float	h0 = n_samples/2;

	int	i,j;
	
	speckle_pattern.fill(complexF32(0));
	for(i = 0; i < n_rays; i++)
		{
		for(j = 0; j < n_samples; j++)
			{
			float	x = (i-w0) * dV;
			float	y = (j-h0) * dH;
			if(sqrt(x*x + y*y) < r) speckle_pattern[i][j] = polar(1, RandomUniformF64(0, two_pi()));
			}
		}
	speckle_pattern.DisplayFrame(0, "Speckle pattern");
	for(i = 0; i < n_rays; i++)
		{
		FFT(row(i), ftForward);
		speckle_pattern[i].roll_half(true);
		FFT(speckle_pattern[i], ftForward);
		}
	for(j = 0; j < n_samples; j++)
		{
		FFT(col(j), ftForward);
		speckle_pattern.col(j).roll_half(true);
		FFT(speckle_pattern.col(j), ftForward);
		}

	*this *= speckle_pattern;

	for(i = 0; i < n_rays; i++) FFT(row(i), ftReverse);
	for(j = 0; j < n_samples; j++) FFT(col(j), ftReverse);
	}


