#include "pre.h"

#include "SignalProcessing/SectorData.h"
#include "Interpolation.h"
//#include <Graph.h>

XRAD_BEGIN

Interpolator :: Interpolator()
	{
	nFiltres = nTaps = 0;
	Filtres = NULL;
	}
	
	
void	Interpolator :: I_Interpolator(size_t nF, size_t nT, size_t filtreType, double Carrier)
	{
	size_t	i, j, k, s;
//	float	*frequencyFiltre = CreatePointer(float, nT);
	RealFunctionF32	frequencyFiltre(nT);
	RealFunctionF32	tmpWindow(nT);
//	float	*tmpWindow = CreatePointer(float, nT);
	float	acc = 0;
	
	nFiltres = nF;
	nTaps = nT;
	centreTap = nTaps/2;
		
	if(Carrier < 0 || Carrier > 1) Error("I_Interpolator, Coefficient 'Carrier' must be from 0 to 1");
	s = nTaps*Carrier;
	
	
	if(filtreType  == Custom) {}//xrad::Get_Function(frequencyFiltre, "Frequency filtre for convolutor");
	else	for(i = 0; i < nT; i++) switch(filtreType)
		{
		case deltaF:
			frequencyFiltre[i] = 0;
			if(i == centreTap) frequencyFiltre[i] = 1;
			acc += frequencyFiltre[i];
			break;
		case roof4:
			frequencyFiltre[i] = 0;
			if(i == centreTap) frequencyFiltre[i] = 1;
			if(fabs(int(i)-centreTap) == 1) frequencyFiltre[i] = 0.5;
			acc += frequencyFiltre[i];
			break;
		case roof8:
			frequencyFiltre[i] = 0;
			if(i == centreTap) frequencyFiltre[i] = 1;
			if(fabs(int(i)-centreTap) == 1) frequencyFiltre[i] = 0.75;
			if(fabs(int(i)-centreTap) == 2) frequencyFiltre[i] = 0.5;
			if(fabs(int(i)-centreTap) == 3) frequencyFiltre[i] = 0.25;
			acc += frequencyFiltre[i];
			break;
		case Gauss:
			frequencyFiltre[i] = exp(-2.*(float)(i - centreTap + 0.5)*(i - centreTap + 0.5)/(nTaps - 0.5));
			acc += frequencyFiltre[i];
			break;
		case Hamming:
			frequencyFiltre[i] = 0.54 + 0.46*cos(two_pi()*(float)(i - centreTap + 0.5)/(nTaps - 0.5));
			acc += frequencyFiltre[i];
			break;
		case rectangle: frequencyFiltre[i] = 1;
			acc += frequencyFiltre[i];
			break;
		default: frequencyFiltre[i] = 1;
			acc += frequencyFiltre[i];
			break;
		}
	for(i = 0; i < nT; i++) frequencyFiltre[i] /= (acc/(4*nT));

	for(i = 0; i < nT; i++)
		tmpWindow[i] = 0.54 + 0.46*cos(two_pi()*(float)(i - centreTap + 0.5)/(nTaps - 0.5));
		
	//grafit(tmpWindow, nTaps, 0, 1, "Interpolation filtre", "", "");
	
	Filtres = CreatePointer<complexF32*>(nF);
	for(i = 0; i < nF; i++)	Filtres[i] = CreatePointer<complexF32>(nTaps);
		
	
	for(i = 0; i < nF; i++)
		{
		for(j = 0; j < nT; j ++)
			{
			for(k = 0; k < nT; k++)
				{
				float	Phase = (float)(k+s-centreTap)*(j  - (centreTap + (float)i/nF))*
					(two_pi()/nT);
				Filtres[i][j] += polar(tmpWindow[j]*frequencyFiltre[k]/nT, Phase);
				}
			}
		}
	//for(i = 0; i < nF; i ++) 
		//grafc(Filtres[centreTap], nT, centreTap, 1, "Filtre", "");
	
	maxWhere = 0;
//	DestroyPointer(frequencyFiltre);
//	DestroyPointer(tmpWindow);
	}

void	Interpolator :: Analyze()
	{
	size_t	No;
	do
		{
		No = GetSigned("Filtre No (0=exit)", 0, 0, nFiltres);
		ComplexFunctionF32	ff;
		ff.UseData(Filtres[No-1], nTaps);
		if(No)DisplayMathFunction(ff, 0, 1, "Interpolation filtre", "Taps", "", false);
		}while(No);
	}
		
extern size_t SSS, nSSS, PPP, nPPP;

complexF32 Interpolator :: Interpolate(complexF32 *What, double Where)
	{
	size_t	W = Where;
	size_t	dW = (size_t)((Where - W)*nFiltres);
	size_t	i;
	
	complexF32 res(0);
	complexF32 *start = What + W - centreTap;

	for(i = 0; i < nTaps-1; i++)
		{
		if(i+W-centreTap > 0 && i+W-centreTap < maxWhere)
			{
			res += Filtres[dW][i]*start[i];
			}
		}
	return	res;
	}
	
void	Interpolator :: SetMaxWhere(size_t w)
	{
	maxWhere = w;
	}
	
Interpolator :: ~Interpolator()
	{
	size_t	i;
	for(i = 0; i < nFiltres; i++) free(Filtres[i]);
	free(Filtres);
	}
	
	
XRAD_END
