#include "pre.h"
#include <StatisticUtils.h>
#include "MovableMedia.h"


//--------------------------------------------
//	
//	итерации
//	
//--------------------------------------------


int	DiscreteMedia :: start()
{
	no = 0;
	value = pointsB[no];
	xp = pointsX[no];
	zp = pointsZ[no];
	return 0;
}


void	DiscreteMedia :: operator ++()
{
	no ++;
	if(no < n_points)
	{
		value = pointsB[no];
		xp = pointsX[no];
		zp = pointsZ[no];
	}
}

bool	DiscreteMedia :: end()
{
	return no < n_points;
}

//--------------------------------------------
//	
//	инициализациЯ
//	
//--------------------------------------------

enum{n=9};
//static	int	n = 9;
static	float	pZ[n] = {0.5,	0.5,	0.5, 	0.25,	0.25,	0.25,	0.75,	0.75,	0.75};
static	float	pX[n] = {0.5,	0.25,	0.75, 	0.5,	0.25,	0.75,	0.5,	0.25,	0.75};
static	float	pS[n] = {0,	1,	1, 	0,	0,	0,	0,	0,	0};
static	complexF32 pB[n] = {complexF32(1,0),	complexF32(1,0),	complexF32(1,0),	complexF32(1,0),	complexF32(1,0),	complexF32(1,0),	complexF32(1,0),	complexF32(1,0),	complexF32(1,0)};


void	DiscreteMedia :: Init(double s, double /*d*/){
	n_points = n;

	pointsX.ImportData(pX, n_points,1);
	pointsZ.ImportData(pZ, n_points,1);
	pointsB.ImportData(pB, n_points,1);
	pointsSpeed.ImportData(pS, n_points,1);


	pointsX *= n_array_elements;
	pointsZ *= n_samples;

	pointsSpeed *= s;
}

//--------------------------------------------
//	
//	движение
//	
//--------------------------------------------


void	DiscreteMedia :: Move(double /*delay*/)
{
	physical_length	maxDistance = cm(15);
	int	i;
	float	tau = 2*maxDistance.cm()/soundSpeed.cm_sec();
	physical_length	dZ = (rMax-rMin)/n_samples;


	for(i = 0; i < n_points; i++)
	{
		double	delta = tau*pointsSpeed[i]/dZ.cm();
		pointsZ[i] -= delta;
		if(!in_range(pointsZ[i], 0, n_samples)) pointsB[i] = 0;
	}
}