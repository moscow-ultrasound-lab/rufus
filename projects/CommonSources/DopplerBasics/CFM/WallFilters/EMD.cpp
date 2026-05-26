#include "pre.h"
#include "EMD.h"
#include <vector>

XRAD_BEGIN

class CubicSpline
{
private:
	// Структура, описывающая сплайн на каждом сегменте сетки
	struct spline_tuple
	{
		double a, b, c, d, x;
	};

//	spline_tuple *splines; // Сплайн
	vector<spline_tuple> splines; // Сплайн
	size_t n; // Количество узлов сетки

//	void free_mem(); // Освобождение памяти

public:
	CubicSpline(); //конструктор
	~CubicSpline(); //деструктор

	// Построение сплайна
	// x - узлы сетки, должны быть упорядочены по возрастанию, кратные узлы запрещены
	// y - значения функции в узлах сетки
	// n - количество узлов сетки
	void Build_Spline(RealFunctionF32 &x, RealFunctionF32 &y);

	// Вычисление значения интерполированной функции в произвольной точке
	double Interpolate_y(double x) const;
};

CubicSpline::CubicSpline() : splines(NULL)
{

}

CubicSpline::~CubicSpline()
{
	//free_mem();
}

void CubicSpline::Build_Spline(RealFunctionF32 &x, RealFunctionF32 &a)
{
	//free_mem();
	n=x.size();

	RealFunctionF32 y(n);
	for (size_t i = 0; i < n; ++i)
		{
		y[i]=a[x[i]];
		}

	// Инициализация массива сплайнов
//	splines = new spline_tuple[n];
	splines.resize(n);
	for (size_t i = 0; i < n; ++i)
	{
		splines[i].x = x[i];
		splines[i].a = y[i];
	}
	splines[0].c = 0.;

	// Решение СЛАУ относительно коэффициентов сплайнов c[i] методом прогонки для трехдиагональных матриц
	// Вычисление прогоночных коэффициентов - прямой ход метода прогонки
// 	double *alpha = new double[n - 1];
// 	double *beta = new double[n - 1];
	RealFunctionF64	alpha(n - 1);
	RealFunctionF64	beta(n - 1);
	double A(0), B(0), C(0), F(0), /*h_i, h_i1,*/ z(0);
	alpha[0] = beta[0] = 0.;
	for (size_t i = 1; i < n - 1; ++i)
	{
		double	h_i = x[i] - x[i - 1];
		double	h_i1 = x[i + 1] - x[i];
		A = h_i;
		C = 2. * (h_i + h_i1);
		B = h_i1;
		F = 6. * ((y[i + 1] - y[i]) / h_i1 - (y[i] - y[i - 1]) / h_i);
		z = (A * alpha[i - 1] + C);
		alpha[i] = -B / z;
		beta[i] = (F - A * beta[i - 1]) / z;
	}

	splines[n - 1].c = (F - A * beta[n - 2]) / (C + A * alpha[n - 2]);

	// Нахождение решения - обратный ход метода прогонки
	for (size_t i = n - 2; i > 0; --i)
		splines[i].c = alpha[i] * splines[i + 1].c + beta[i];

	// Освобождение памяти, занимаемой прогоночными коэффициентами
// 	delete[] beta;
// 	delete[] alpha;

	// По известным коэффициентам c[i] находим значения b[i] и d[i]
	for (size_t i = n - 1; i > 0; --i)
	{
		double h_i = x[i] - x[i - 1];
		splines[i].d = (splines[i].c - splines[i - 1].c) / h_i;
		splines[i].b = h_i * (2. * splines[i].c + splines[i - 1].c) / 6. + (y[i] - y[i - 1]) / h_i;
	}
}

double CubicSpline::Interpolate_y(double x) const
{
//	if (!splines)
//	if (splines.empty())
	if (splines.size()!=n)
		return std::numeric_limits<double>::quiet_NaN(); // Если сплайны ещё не построены - возвращаем NaN
//	spline_tuple *s;
	vector<spline_tuple>::const_iterator s;
	if (x <= splines[0].x) // Если x меньше точки сетки x[0] - пользуемся первым эл-том массива
		s = splines.begin() + 1;
	else if (x >= splines[n - 1].x) // Если x больше точки сетки x[n - 1] - пользуемся последним эл-том массива
		s = splines.begin() + n - 1;
	else // Иначе x лежит между граничными точками сетки - производим бинарный поиск нужного эл-та массива
	{
		std::size_t i = 0, j = n - 1;
		while (i + 1 < j)
		{
			std::size_t k = i + (j - i) / 2;
			if (x <= splines[k].x)
				j = k;
			else
				i = k;
		}
		s = splines.begin() + j;
	}
	double dx = (x - s->x);
	return s->a + (s->b + (s->c / 2. + s->d * dx / 6.) * dx) * dx; // Вычисляем значение сплайна в заданной точке.
}

// void CubicSpline::free_mem()
// {
// // 	delete[] splines;
// // 	splines = NULL;
// }



void LocalExtrema(RealFunctionF32 &burst, RealFunctionF32 &a, RealFunctionF32 &b, RealFunctionF32 &an, RealFunctionF32 &bn) 
{
	if (an.size() < burst.size() || bn.size() < burst.size())
	{
		an.resize(burst.size());
		bn.resize(burst.size());
	}
	a[0]=burst[0];
	b[0]=burst[0];
	size_t k(0), p(0);
// 	an[0]=0;
// 	bn[0]=0;
	// 	for (size_t i = 0; i<burst.size(); ++i)
	// 		burst[i]=i;
	for(size_t i = 1; i < burst.size()-1; ++i)
	{
		if (burst[i] >= burst[i-1] && burst[i] >= burst[i+1])
		{
			k+=1;
			a[i]=burst[i];
// 			b[i]=0;
			an[k]=i;
		}
		else
		{
			if (burst[i] <= burst[i-1] && burst[i] <= burst[i+1])
			{
				p+=1;
// 				a[i]=0;
				b[i]=burst[i];
				bn[p]=i;
			}
// 			else
// 			{
// 				a[i]=0;
// 				b[i]=0;
// 			}
		}
	}
// 	if (k == 0 || p == 0)
// 	{
// 		k+=1;
// 		an[k]=burst.size()/2;
// 		a[burst.size()/2]=burst[burst.size()/2];
// 		p+=1;
// 		bn[p]=burst.size()/2;
// 		b[burst.size()/2]=burst[burst.size()/2];
// 	}
	if (k == 0)
	{
		k += 1;
		an[k] = burst.size() / 2;
		a[burst.size() / 2] = burst[burst.size() / 2];
	}
	if (p == 0)
	{
		p += 1;
		bn[p] = burst.size() / 2;
		b[burst.size() / 2] = burst[burst.size() / 2];
	}
	k+=1;
	p+=1;
	if(k < burst.size())
	{
		an[k]=burst.size()-1;
		bn[p]=burst.size()-1;
		an.resize(k+1);
		bn.resize(p+1);
	}
	a[burst.size()-1]=burst[burst.size()-1];
	b[burst.size()-1]=burst[burst.size()-1];
}

//TODO более ясно описать функции Interpolation::Apply, что они делают

void LinearInterpolation::Apply(RealFunctionF32 &burst, RealFunctionF32 &a, RealFunctionF32 &b, RealFunctionF32 &an, RealFunctionF32 &bn) const
{
	size_t k(0), p(0);
	for(size_t i = 1; i < burst.size()-1; ++i)
	{
		if (burst[i] >= burst[i-1] && burst[i] >= burst[i+1])
		{
			while (i>bn[p] && p<(bn.size()-1)) p+=1;
			b[i]=b[bn[p-1]]+(b[bn[p]]-b[bn[p-1]])/(bn[p]-bn[p-1])*(i-bn[p-1]);
		}
		else
		{
			if (burst[i] <= burst[i-1] && burst[i] <= burst[i+1])
			{
				while (i>an[k]) k+=1;
				a[i]=a[an[k-1]]+(a[an[k]]-a[an[k-1]])/(an[k]-an[k-1])*(i-an[k-1]);
			}
			else
			{
				while (i>bn[p]) p+=1;
				b[i]=b[bn[p-1]]+(b[bn[p]]-b[bn[p-1]])/(bn[p]-bn[p-1])*(i-bn[p-1]);
				while (i>an[k]) k+=1;
				a[i]=a[an[k-1]]+(a[an[k]]-a[an[k-1]])/(an[k]-an[k-1])*(i-an[k-1]);
			}
		}
	}
}

void CSplineInterpCurve(RealFunctionF32 &y, RealFunctionF32 &x)
{
	CubicSpline interpolation;
	interpolation.Build_Spline(x, y);
	double a;
	for (size_t i = 0; i < y.size(); ++i)
	{
		a=interpolation.Interpolate_y(i);
		y[i]=a;
	}
}

//TODO: алгоритм сплайн-интерполяции работает не так
void CSplineInterpolation::Apply(RealFunctionF32 &/*burst*/, RealFunctionF32 &a, RealFunctionF32 &b, RealFunctionF32 &an, RealFunctionF32 &bn) const
{
	CSplineInterpCurve(a, an);
	CSplineInterpCurve(b, bn);
}

void EMD(RealFunctionF32 &burst, size_t flag, size_t number_of_iterations) 
{
	RealFunctionF32 a(burst.size(),0);
	RealFunctionF32 b(burst.size(),0);
	RealFunctionF32 an(burst.size(),0);
	RealFunctionF32 bn(burst.size(),0);
	for(size_t j = 0; j < number_of_iterations; ++j)
	{
		LocalExtrema(burst, a, b, an, bn);
		if (flag == 1)
		{
			LinearInterpolation interpolation;
			interpolation.Apply(burst, a, b, an, bn);
		}
		else
		{
			CSplineInterpolation interpolation;
			interpolation.Apply(burst, a, b, an, bn);
		}
		RealFunctionF32 m(burst.size());
		m[0]=a[0];
		m[burst.size()-1]=a[burst.size()-1];
		for(size_t i = 1; i < burst.size()-1; ++i)
		{
			m[i]=0.5*(a[i]+b[i]);
		}
		for(size_t i = 0; i < burst.size(); ++i)
		{
			burst[i]=burst[i]-m[i];
		}
	}
}



void WallFilterEMD::Apply(ComplexFunctionF32 &burst) const
{
	RealFunctionF32	re,im;
	burst.GetReal(re);
	burst.GetImag(im);
	EMD(re, flag, number_of_iterations);
	EMD(im, flag, number_of_iterations);
}


XRAD_END


