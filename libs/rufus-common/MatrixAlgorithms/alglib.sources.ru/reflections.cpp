/*************************************************************************
Copyright (c) 1992-2007 The University of Tennessee.  All rights reserved.

Contributors:
    * Sergey Bochkanov (ALGLIB project). Translation from FORTRAN to
      pseudocode.

See subroutines comments for additional copyrights.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

- Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer listed
  in this license in the documentation and/or other materials
  provided with the distribution.

- Neither the name of the copyright holders nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*************************************************************************/

//#include "pre.h"
#include "reflections.h"

/*************************************************************************
Генерация элементарного преобразования отражения

Подпрограмма генерирует элементарное отражение H порядка N, такое, что для
заданного X выполняется следующее равенство:

    ( X(1) )   ( Beta )
H * (  ..  ) = (  0   )
    ( X(n) )   (  0   )
    
где
              ( V(1) )
H = 1 - Tau * (  ..  ) * ( V(1), ..., V(n) )
              ( V(n) )

причем первая компонента вектора V равна единице.
              
Входные параметры:
    X       -   вектор. Массив с нумерацией элементов [1..N]
    N       -   порядок отражения
    
Выходные параметры:
    X       -   компоненты с 2 по N замещается вектором V. Первая
                компонента замещается параметром Beta.
    Tau     -   скалярная величина Tau. Равно 0 (если вектор X - нулевой),
                в противном случае 1 <= Tau <= 2.

Данная подпрограмма является модификацией подпрограмм DLARFG из библиотеки
LAPACK. Функциональность аналогичная, но отсутствует  корректная обработка
случаев, когда промежуточные результаты вычислений  переполняют  разрядную
сетку.

ИЗМЕНЕНИЯ И ИСПРАВЛЕНИЯ:
    24.12.2005 замена sign(Alpha) на аналогичный sign в фортране код

  -- LAPACK auxiliary routine (version 3.0) --
     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
     Courant Institute, Argonne National Lab, and Rice University
     September 30, 1994
*************************************************************************/
void generatereflection(ap::real_1d_array& x, int n, double& tau)
{
    int j;
    double alpha;
    double xnorm;
    double v;
    double beta;
    double mx;

    
    //
    // Executable Statements ..
    //
    if( n<=1 )
    {
        tau = 0;
        return;
    }
    
    //
    // XNORM = DNRM2( N-1, X, INCX )
    //
    alpha = x(1);
    mx = 0;
    for(j = 2; j <= n; j++)
    {
        mx = ap::maxreal(::fabs(x(j)), mx);
    }
    xnorm = 0;
    if( mx!=0 )
    {
        for(j = 2; j <= n; j++)
        {
            xnorm = xnorm+ap::sqr(x(j)/mx);
        }
        xnorm = sqrt(xnorm)*mx;
    }
    if( xnorm==0 )
    {
        
        //
        // H  =  I
        //
        tau = 0;
        return;
    }
    
    //
    // general case
    //
    mx = ap::maxreal(::fabs(alpha), ::fabs(xnorm));
    beta = -mx*sqrt(ap::sqr(alpha/mx)+ap::sqr(xnorm/mx));
    if( alpha<0 )
    {
        beta = -beta;
    }
    tau = (beta-alpha)/beta;
    v = 1/(alpha-beta);
    ap::vmul(&x(2), ap::vlen(2,n), v);
    x(1) = beta;
}


void generatereflection_c(ap::complex_1d_array& in_t, int n, double& tau)
{
	ap::real_1d_array t;
	t.setbounds(1,n);
	for(int i = 0; i < n; i++)
	{
		t(i+1) = in_t(i+1).re;
	}

	int j;
// 	double alpha;
	double alpha;
	double xnorm;
	double v;
	double beta;
	double mx;


	//
	// Executable Statements ..
	//
	if( n<=1 )
	{
		tau = 0;
		return;
	}

	//
	// XNORM = DNRM2( N-1, X, INCX )
	//
	alpha = t(1);
	mx = 0;
	for(j = 2; j <= n; j++)
	{
		mx = ap::maxreal(::fabs(t(j)), mx);
	}
	xnorm = 0;
	if( mx!=0 )
	{
		for(j = 2; j <= n; j++)
		{
			xnorm = xnorm+ap::sqr(t(j)/mx);
		}
		xnorm = sqrt(xnorm)*mx;
	}
	if( xnorm==0 )
	{

		//
		// H  =  I
		//
		tau = 0;
		return;
	}

	//
	// general case
	//
	mx = ap::maxreal(::fabs(alpha), ::fabs(xnorm));
	beta = -mx*sqrt(ap::sqr(alpha/mx)+ap::sqr(xnorm/mx));
	if( alpha<0 )
	{
		beta = -beta;
	}
	tau = (beta-alpha)/beta;
	v = 1/(alpha-beta);
	ap::vmul(&t(2), ap::vlen(2,n), v);
	t(1) = beta;

	for(int i = 0; i < n; i++)
	{
		in_t(i+1).re = t(i+1);
	}
}

/*************************************************************************
Применение элементарного отражения к прямоугольной матрице размером MxN

Алгоритм умножает слева матрицу на элементарное преобразование  отражения,
заданное    столбцом   V   и   скалярной   величиной   Tau  (см.  описание
GenerateReflection). Преобразованию подвергается не вся матрица, а  только
её часть (строки от M1 до M2, столбцы от N1 до N2). Элементы, не  попавшие
в указанную подматрицу, остаются без изменений.

Входные параметры:
    C   -   матрица,  к  которой  применяется  преобразование.
    Tau -   скаляр, задающий преобразование.
    V   -   столбец, задающий преобразование. Массив с нумерацией элементов
            [1..M2-M1+1]
    M1,M2   -   диапазон строк, затрагиваемых преобразованием
    N1,N2   -   диапазон столбцов, затрагиваемых преобразованием
    WORK    -   рабочий массив с нумерацией элементов от N1 до N2

Выходные параметры:
    C   -   результат умножения входной матрицы C на матрицу преобразования,
            заданного Tau и V. Если N1>N2 или M1>M2, то C не меняется.

  -- LAPACK auxiliary routine (version 3.0) --
     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
     Courant Institute, Argonne National Lab, and Rice University
     September 30, 1994
*************************************************************************/
void applyreflectionfromtheleft(ap::real_2d_array& c,
     double tau,
     const ap::real_1d_array& v,
     int m1,
     int m2,
     int n1,
     int n2,
     ap::real_1d_array& work)
{
    double t;
    int i;
    int vm;

    if( tau==0||n1>n2||m1>m2 )
    {
        return;
    }
    
    //
    // w := C' * v
    //
    vm = m2-m1+1;
    for(i = n1; i <= n2; i++)
    {
        work(i) = 0;
    }
    for(i = m1; i <= m2; i++)
    {
        t = v(i+1-m1);
        ap::vadd(&work(n1), &c(i, n1), ap::vlen(n1,n2), t);
    }
    
    //
    // C := C - tau * v * w'
    //
    for(i = m1; i <= m2; i++)
    {
        t = v(i-m1+1)*tau;
        ap::vsub(&c(i, n1), &work(n1), ap::vlen(n1,n2), t);
    }
}

void applyreflectionfromtheleft_c(ap::complex_2d_array& a,
								xrad::complexF64 v,
								const ap::complex_1d_array& t,
								int m1,
								int m2,
								int n1,
								int n2,
								ap::complex_1d_array& work)
{
// 	int n(5);
// 	ap::real_2d_array a;
// 	a.setbounds(1,n,1,n);
// 	for(int i = 0; i < n; i++)
// 	{
// 		for(int j = 0; j < n; j++)
// 		{
// 			a(i+1,j+1) = in_a(i+1,j+1).re;
// 		}
// 	}
// 	ap::real_1d_array t;
// 	t.setbounds(1,n);
// 	for(int i = 0; i < n; i++)
// 	{
// 		t(i+1) = in_t(i+1).re;
// 	}
// 	double v(in_v.re);

// 	double tt;
	xrad::complexF64 tt;
	int i;
	int vm;

	if( v==0||n1>n2||m1>m2 )
	{
		return;
	}

	//
	// w := C' * t
	//
	vm = m2-m1+1;
	for(i = n1; i <= n2; i++)
	{
		work(i) = 0;
	}
	for(i = m1; i <= m2; i++)
	{
		tt = t(i+1-m1);
		ap::vadd(&work(n1), &a(i, n1), ap::vlen(n1,n2), tt);
	}

	//
	// C := C - v * t * w'
	//
	for(i = m1; i <= m2; i++)
	{
		tt = t(i-m1+1)*v;
		ap::vsub(&a(i, n1), &work(n1), ap::vlen(n1,n2), tt);
	}

// 	for(int i = 0; i < n; i++)
// 	{
// 		for(int j = 0; j < n; j++)
// 		{
// 			in_a(i+1,j+1).re = a(i+1,j+1);
// 		}
// 	}

}


/*************************************************************************
Применение элементарного отражения к прямоугольной матрице размером MxN

Алгоритм умножает справа матрицу на элементарное преобразование отражения,
заданное    столбцом   V   и   скалярной   величиной   Tau  (см.  описание
GenerateReflection). Преобразованию подвергается не вся матрица, а  только
её часть (строки от M1 до M2, столбцы от N1 до N2). Элементы, не  попавшие
в указанную подматрицу, остаются без изменений.

Входные параметры:
    C   -   матрица,  к  которой  применяется  преобразование.
    Tau -   скаляр, задающий преобразование.
    V   -   столбец, задающий преобразование. Массив с нумерацией элементов
            [1..N2-N1+1]
    M1,M2   -   диапазон строк, затрагиваемых преобразованием
    N1,N2   -   диапазон столбцов, затрагиваемых преобразованием
    WORK    -   рабочий массив с нумерацией элементов от M1 до M2

Выходные параметры:
    C   -   результат умножения входной матрицы C на матрицу преобразования,
            заданного Tau и V. Если N1>N2 или M1>M2, то C не меняется.

  -- LAPACK auxiliary routine (version 3.0) --
     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
     Courant Institute, Argonne National Lab, and Rice University
     September 30, 1994
*************************************************************************/
void applyreflectionfromtheright(ap::real_2d_array& c,
     double tau,
     const ap::real_1d_array& v,
     int m1,
     int m2,
     int n1,
     int n2,
     ap::real_1d_array& work)
{
    double t;
    int i;
    int vm;

    if( tau==0||n1>n2||m1>m2 )
    {
        return;
    }
    
    //
    // w := C * v
    //
    vm = n2-n1+1;
    for(i = m1; i <= m2; i++)
    {
        t = ap::vdotproduct(&c(i, n1), &v(1), ap::vlen(n1,n2));
        work(i) = t;
    }
    
    //
    // C := C - w * v'
    //
    for(i = m1; i <= m2; i++)
    {
        t = work(i)*tau;
        ap::vsub(&c(i, n1), &v(1), ap::vlen(n1,n2), t);
    }
}


void applyreflectionfromtheright_c(ap::complex_2d_array& a,
								 xrad::complexF64 v,
								 const ap::complex_1d_array& t,
								 int m1,
								 int m2,
								 int n1,
								 int n2,
								 ap::complex_1d_array& work)
{
// 	int n(5);
// 	ap::real_2d_array a;
// 	a.setbounds(1,n,1,n);
// 	for(int i = 0; i < n; i++)
// 	{
// 		for(int j = 0; j < n; j++)
// 		{
// 			a(i+1,j+1) = in_a(i+1,j+1).re;
// 		}
// 	}
// 	ap::real_1d_array t;
// 	t.setbounds(1,n);
// 	for(int i = 0; i < n; i++)
// 	{
// 		t(i+1) = in_t(i+1).re;
// 	}
// 	double v(in_v.re);



// 	double tt;
	xrad::complexF64 tt;
	int i;
	int vm;

	if( v==0||n1>n2||m1>m2 )
	{
		return;
	}

	//
	// w := C * t
	//
	vm = n2-n1+1;
	for(int ii = m1; ii <= m2; ii++)
	{
		tt = ap::vdotproduct(&a(ii, n1), &t(1), ap::vlen(n1,n2));
		work(ii) = tt;
	}

	//
	// C := C - w * t'
	//
	for(i = m1; i <= m2; i++)
	{
		tt = work(i)*v;
		ap::vsub(&a(i, n1), &t(1), ap::vlen(n1,n2), tt);
	}


// 	for(int i = 0; i < n; i++)
// 	{
// 		for(int j = 0; j < n; j++)
// 		{
// 			in_a(i+1,j+1).re = a(i+1,j+1);
// 		}
// 	}

}



