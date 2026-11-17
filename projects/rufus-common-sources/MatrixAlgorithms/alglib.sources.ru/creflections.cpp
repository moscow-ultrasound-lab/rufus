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
#include "creflections.h"

/*************************************************************************
Генерация элементарного комплексного преобразования отражения

Подпрограмма  генерирует  элементарное  комплексное отражение H порядка N,
такое, что для заданного X выполняется следующее равенство:

     ( X(1) )   ( Beta )
H' * (  ..  ) = (  0   ),   H'*H = I,   Beta - вещественное число
     ( X(n) )   (  0   )
    
где

              ( V(1) )
H = 1 - Tau * (  ..  ) * ( conj(V(1)), ..., conj(V(n)) )
              ( V(n) )

причем первая компонента вектора V равна единице.
              
Входные параметры:
    X       -   вектор. Массив с нумерацией элементов [1..N]
    N       -   порядок отражения
    
Выходные параметры:
    X       -   компоненты с 2 по N замещается вектором V. Первая
                компонента замещается параметром Beta.
    Tau     -   скалярная величина Tau.

Данная подпрограмма является модификацией подпрограмм CLARFG из библиотеки
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
void complexgeneratereflection(ap::complex_1d_array& x,
     int n,
     ap::complex& tau)
{
    int j;
//    int knt;
    ap::complex alpha;
    double alphi;
    double alphr;
    double beta;
    double xnorm;
    double mx;
    ap::complex t;
    int i_;

    if( n<=0 )
    {
        tau = 0;
        return;
    }
    alpha = x(1);
    mx = 0;
    for(j = 2; j <= n; j++)
    {
        mx = ap::maxreal(ap::abscomplex(x(j)), mx);
    }
    xnorm = 0;
    if( mx!=0 )
    {
        for(j = 2; j <= n; j++)
        {
            t = x(j)/mx;
            xnorm = xnorm+(t*ap::conj(t)).re;
        }
        xnorm = sqrt(xnorm)*mx;
    }
    alphr = alpha.re;
    alphi = alpha.im;
    if( xnorm==0&&alphi==0 )
    {
        tau = 0;
        return;
    }
    mx = ap::maxreal(::fabs(alphr), ::fabs(alphi));
    mx = ap::maxreal(mx, ::fabs(xnorm));
    beta = -mx*sqrt(ap::sqr(alphr/mx)+ap::sqr(alphi/mx)+ap::sqr(xnorm/mx));
    if( alphr<0 )
    {
        beta = -beta;
    }
    tau.re = (beta-alphr)/beta;
    tau.im = -alphi/beta;
    alpha = 1/(alpha-beta);
    if( n>1 )
    {
        for(i_=2; i_<=n;i_++)
        {
            x(i_) = alpha*x(i_);
        }
    }
    alpha = beta;
    x(1) = alpha;
}


/*************************************************************************
Применение элементарного отражения к прямоугольной матрице размером MxN

Алгоритм умножает слева матрицу на элементарное преобразование  отражения,
заданное    столбцом   V   и   скалярной   величиной   Tau  (см.  описание
GenerateReflection). Преобразованию подвергается не вся матрица, а  только
её часть (строки от M1 до M2, столбцы от N1 до N2). Элементы, не  попавшие
в указанную подматрицу, остаются без изменений.

Замечание: матрица умножается на матрицу H, а не  на  H'.  Если  требуется
умножить на матрицу H', вместо параметра Tau требуется передать Conj(Tau).

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
void complexapplyreflectionfromtheleft(ap::complex_2d_array& c,
     ap::complex tau,
     const ap::complex_1d_array& v,
     int m1,
     int m2,
     int n1,
     int n2,
     ap::complex_1d_array& work)
{
    ap::complex t;
    int i;
    int vm;
    int i_;

    if( tau==0||n1>n2||m1>m2 )
    {
        return;
    }
    
    //
    // w := C^T * conj(v)
    //
    vm = m2-m1+1;
    for(i = n1; i <= n2; i++)
    {
        work(i) = 0;
    }
    for(i = m1; i <= m2; i++)
    {
        t = ap::conj(v(i+1-m1));
        for(i_=n1; i_<=n2;i_++)
        {
            work(i_) = work(i_) + t*c(i,i_);
        }
    }
    
    //
    // C := C - tau * v * w^T
    //
    for(i = m1; i <= m2; i++)
    {
        t = v(i-m1+1)*tau;
        for(i_=n1; i_<=n2;i_++)
        {
            c(i,i_) = c(i,i_) - t*work(i_);
        }
    }
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
void complexapplyreflectionfromtheright(ap::complex_2d_array& c,
     ap::complex tau,
     ap::complex_1d_array& v,
     int m1,
     int m2,
     int n1,
     int n2,
     ap::complex_1d_array& work)
{
    ap::complex t;
    int i;
    int vm;
    int i_;
    int i1_;

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
        i1_ = (1)-(n1);
        t = 0.0;
        for(i_=n1; i_<=n2;i_++)
        {
            t += c(i,i_)*v(i_+i1_);
        }
        work(i) = t;
    }
    
    //
    // C := C - w * conj(v^T)
    //
    for(i_=1; i_<=vm;i_++)
    {
        v(i_) = ap::conj(v(i_));
    }
    for(i = m1; i <= m2; i++)
    {
        t = work(i)*tau;
        i1_ = (1) - (n1);
        for(i_=n1; i_<=n2;i_++)
        {
            c(i,i_) = c(i,i_) - t*v(i_+i1_);
        }
    }
    for(i_=1; i_<=vm;i_++)
    {
        v(i_) = ap::conj(v(i_));
    }
}



