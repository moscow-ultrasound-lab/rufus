#ifndef __movable_media_h
#define __movable_media_h

#include "LinearSignal.h"
#include "ScatterMedia.h"

XRAD_BEGIN

//-----------------------------------------
//
//	среда, имитирующаЯ сплошной фон
//	и кровеносные сосуды (долго считаетсЯ)
//
//-----------------------------------------

#define	nHoles	7
class	ContinuousMedia : private LinearSignalOptions, private ComplexFunction2D_F32, public scatterMedia{
	double	speed, speed_deviation;
	

	//	подвижные области
	ComplexFunction2D holes[nHoles];
	void	InitHoles();
	void	PutHoles();
	int	holes_v[nHoles], holes_h[nHoles];
public:
	ContinuousMedia(LinearSignal &original) : LinearSignalOptions(*(LinearSignalOptions*)&original)
		{
		realloc(n_array_elements, n_samples);
		speed = 0;
		speed_deviation = 0;
		n_points = vsize()*hsize();
		no = 0;
		};
	void	Init(double s, double d);

	void	SetSpeed(double s, double d){speed = s; speed_deviation = d;}; // скорость кровотока
	void	Move(double delay = 0);

	int	nPoints(){return vsize()*hsize();};
	int	start();

	void	operator ++();
	bool	end();
	};

//-----------------------------------------
//
//	набор точечных отражателей на черном фоне
//	предназначен длЯ оценки диаграмм направленности
//
//-----------------------------------------

class	DiscreteMedia : private LinearSignalOptions, public scatterMedia{
private:
	RealFunctionF32 pointsX, pointsZ, pointsSpeed;
	ComplexFunctionF32 pointsB;
	
public:
	DiscreteMedia(LinearSignal &original):LinearSignalOptions(*(LinearSignalOptions*)&original){};
	void	Init(double s, double d = 0);
	void	Move(double delay = 0);	

	int	nPoints(){return n_points;};
	int	start();
	void	operator ++();
	bool	end();
	};

XRAD_END

#endif //__movable_media_h