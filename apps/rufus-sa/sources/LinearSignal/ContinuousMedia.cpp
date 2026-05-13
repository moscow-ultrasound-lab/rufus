#include "pre.h"
#include <time.h>
#include <StatisticUtils.h>
#include "MovableMedia.h"
//#include "LimitedValue.h"

XRAD_BEGIN


float	dB(float x)
{
	return pow(10, x/20);
}

//--------------------------------------------
//	
//	итерации
//	
//--------------------------------------------

int	ContinuousMedia :: start()
{
	xp = 0;
	zp = 0;
	no = 0;

	value = at(xp,zp);
	return 0;
}


void	ContinuousMedia :: operator ++()
{
	value = 0;

	while(value == 0 && no < n_points)
	{
		no++;
		zp = no%hsize();
		xp = no/hsize();

		if(no < n_points) value = at(xp,zp);
		else value = 0;
	}
}


bool	ContinuousMedia :: end()
{
	return no < n_points;
}

//--------------------------------------------
//	
//	инициализациЯ
//	
//--------------------------------------------


void	ContinuousMedia :: Init(double s, double d)
{
//	int	i,j;

	int	vb = n_array_elements/4;
	int	hb = n_samples/8;

	fill(complexF32(0));
	srand(clock());

	//	фон
	for(int i = vb; i < n_array_elements-vb; i++)
	{
		for(int j = hb; j < n_samples-hb; j++)
		{
			at(i,j) = polar(1, RandomUniformF64(0, two_pi()));
		}
	}

	//	Яркие точки
	at(vb/2,3*n_samples/4) = dB(20);
	at(vb/2,n_samples/4) = dB(20);
	at(vb/2,n_samples/2) = dB(20);
	at(n_array_elements-vb/2,3*n_samples/4) = dB(20);
	at(n_array_elements-vb/2,n_samples/4) = dB(20);
	at(n_array_elements-vb/2,n_samples/2) = dB(20);

	speed = s;
	speed_deviation = d;
	InitHoles();
	PutHoles();

	//for(i = 4; i < n_array_elements; i+=8) 		for(j = 0; j < n_samples; j++) at(i,j] = j%2;

	DisplayMathFunction2D(*this, "Media");
}

void	ContinuousMedia :: InitHoles()
{
	int	hs_large = n_samples/2;
	int	hs_small = n_samples/4;
	int	vs_large = n_array_elements/8;
	int	vs_small = n_array_elements/16;

	int	hh[] = {n_samples/2,	n_samples/3,	2*n_samples/3,	n_samples/3,	2*n_samples/3,	n_samples/2,	n_samples/2};
	int	hv[] = {n_array_elements/2,	n_array_elements/3,	n_array_elements/3,	2*n_array_elements/3,	2*n_array_elements/3,	77*n_array_elements/192,	115*n_array_elements/192};


	for(int i = 0; i < nHoles; i++)
	{
		int	hvs, hhs;
		if(!i)	hvs = vs_large, hhs = hs_large;
		else if(i < 5)	hvs = vs_small, hhs = hs_small;
		else hvs = vs_small/2, hhs = hs_small/2;


		holes_v[i] = hv[i] - hvs/2;
		holes_h[i] = hh[i] - hhs/2;
		holes[i].realloc(hvs, hhs);

		for(int k = 0; k < hvs; k++)
		{
			for(int l = 0; l < hhs; l++)
			{
				holes[i][k][l] = polar(dB(-30), RandomUniformF64(0, two_pi()));
			}
		}
		//if(!i) holes[i][hvs/2][hhs/2] = dB(20); //ЯркаЯ точка, нужна, чтобы оценить движение
	}
}

void	ContinuousMedia :: PutHoles()
{
	for(int i = 0; i < nHoles; i++) holes[i].PutDataSegment(*this, holes_v[i], holes_h[i]);
}


//--------------------------------------------
//	
//	движение
//	
//--------------------------------------------


void	ContinuousMedia :: Move(double /*delay*/)
{
	using std::fabs;

	physical_length	maxDistance = cm(15); // cm
	double	tau = 2*maxDistance.cm()/soundSpeed.cm_sec();
	double	dZ = (rMax-rMin).cm()/n_samples;

	int	h,i,j;

	if(0)
	{
		printf("\nDoing once!\n");
		fflush(stdout);
		for(h = 0; h < nHoles; h++)
		{
			int	vs1 = holes[h].vsize();
			//	развертка по акустическим строкам
			for(i = 0; i < vs1; i++)
			{
				if(!(i%2)) holes[h][i] *= 0.1;
				// временно -- отражение частиц с минусовой скоростью в десЯть раз меньше
			}
		}
	}

	for(h = 0; h < nHoles; h++)
	{
		int	hs1 = holes[h].hsize();
		int	vs1 = holes[h].vsize();
		ComplexFunctionF32	f1(hs1);

		//	развертка по акустическим строкам
		for(i = 0; i < vs1; i++)
		{
			//			double	sd2 = speed_deviation/2;
			double	sdn = speed_deviation/nHoles;
			double	current_speed;

			current_speed = speed - h*sdn;
			double	speedFactor;
			int	v2 = vs1/2;

			speedFactor = 1.- fabs(float(v2-i)/v2); // скорость менЯетсЯ параболически от 0 до 1 и обратно
			if(!h)
			{
				speedFactor = 1.- 2.*fabs(float(v2-i)/v2);
				// в самом большом сосуде обратный ток у стенок (стеноз)
				// ? зависимость не параболическаЯ, а кусочно-линейнаЯ
			};

			current_speed *= speedFactor;

			double	delta = tau*current_speed/dZ;

			for(j = 0; j < hs1; j++) f1[j] = polar(1, delta*two_pi()*j/hs1);

			FFT(holes[h][i], ftForward);
			holes[h][i] *= f1;
			FFT(holes[h][i], ftReverse);
		}
	}
	PutHoles();
}


XRAD_END
