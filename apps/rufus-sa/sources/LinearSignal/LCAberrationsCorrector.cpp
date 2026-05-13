#include "pre.h"
//#include "LimitedValue.h"
#include <StatisticUtils.h>
#include "LCSimulation.h"
#include "MovableMedia.h"



#pragma message ("это неоконченнаЯ попытка внести аберрации в функцию FocusRay")

#if 0
ComplexFunctionF32	LC_Simulator :: FocusRayWithAberrations(long r, scatterMedia &media){
//	положение отражателей (в дискретных величинах
	limited_float	x(0, nElements-1);
	limited_float	z(0, n_samples-1);

//	номера элементов решетки относительно апертуры:
	long	tx_ap_el, rx_ap_el;

//	номера элементов решетки относительно решетки
	limited_int	tx_el(0, nElements-1), rx_el(0, nElements-1);

//	координаты передающей и приемной апертур
	long	tx1, tx2, rx1, rx2;

//	частоты
	double	dW = tau()*1e6*sampleRate/n_samples;
//	float	df = 3;
	long	f0 = float(n_samples)/n_samples;
	long	f1 = (n_samples-1)*f0; // диапазон

	long	ap2 = nApertElements/2;
	long	rx_ap_shift = 0;
	double	max_distance = 2*distances[nApertElements][n_samples-1];
	ComplexFunctionF32	result(n_samples);

	if(TX_Focusing) tx1 = 0, tx2 = nApertElements;
	else tx1 = ap2, tx2 = ap2+1;

	if(RX_Focusing) rx1 = 0, rx2 = nApertElements;
	else rx1 = ap2, rx2 = ap2+1;


	result.fill(0);

	// цикл по отражателЯм
	for(media.start(); media.end(); ++media)
		{
		// цикл по передающим элементам
		for(tx_ap_el = tx1; tx_ap_el < tx2; tx_ap_el++)
			{
			tx_el = r + tx_ap_el - ap2;
			int	a1 = max_of(0, tx_el-nApertElements);
			int	a2 = min_of(nElements, tx_el+nApertElements);

			// цикл по приемным элементам
			if(in_range(x, a1, a2)) for(rx_ap_el = rx1; rx_ap_el < rx2; rx_ap_el++)
				{
				rx_el = r + rx_ap_el - ap2 + rx_ap_shift;
				double	focus = lens[tx_ap_el] + lens[rx_ap_el];
				double	apodization = apod[tx_ap_el]*apod[rx_ap_el];
				x = media.x();
				z = media.z();
				double	distance = distances[labs(rx_el-x)][z] +
						distances[labs(tx_el-x)][z] +
						focus;

				if(distance < max_distance)
					{
					double	phase = dW*distance/soundSpeed;
					complexF	factor = polar(apodization, phase*f0);
					complexF	phasor = polar(1, phase);

					//	цикл по частоте
					for(register int f = f0; f < f1; f++)
						{
						result[f] += factor*media;
						factor *= phasor;
						}
					}
				}
			}
		}
	result *= initPulse;
	result.FFT(REVERSE);

	return result;
	}
#endif