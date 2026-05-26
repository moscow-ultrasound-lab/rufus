/*************************************************************************
Copyright (c) 2005-2007, Sergey Bochkanov (ALGLIB project).

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
#include "hevd.h"

/*************************************************************************
Поиск собственных чисел и векторов эрмитовой матрицы

Алгоритм ищет собственные пары эрмитовой матрицы приводя её к вещественной
трехдиагональной форме и используя QL/QR алгоритм.

Входные параметры:
    A   -       эрмитова матрица, заданная верхним или нижним
                треугольником. Массив с нумерацией элементов [0..N-1, 0..N-1]
    N   -       размер матрицы
    ZNeeded-    флаг, сообщающий, требуются ли собственные векторы.
                Если ZNeeded:
                 * равно 0, то собственные векторы не возвращаются
                 * равно 1, то собственные векторы возвращаются
    IsUpper-    формат хранения

Выходные параметры:
    D   -       собственные числа в порядке возрастания.
                Массив с нумерацией элементов [0..N-1].
    Z   -       если ZNeeded:
                 * равно 0, не модифицируется
                 * равно 1, содержит собственные векторы
                Массив с нумерацией элементов [0..N-1, 0..N-1], при этом
                собственные векторы хранятся в столбцах матрицы.

Результат:
    True, если алгоритм сошелся
    False, если алгоритм не сошелся (редчайший случай)

Замечание:
    собственные векторы эрмитовой  матрицы  определяются  с  точностью  до
    умножения на комплексное L, такое что |L| = 1.

  -- ALGLIB --
     Copyright 2005, 23 March 2007 by Bochkanov Sergey
*************************************************************************/
bool hmatrixevd(ap::complex_2d_array a,
     int n,
     int zneeded,
     bool isupper,
     ap::real_1d_array& d,
     ap::complex_2d_array& z)
{
    bool result;
    ap::complex_1d_array tau;
    ap::real_1d_array e;
    ap::real_1d_array work;
    ap::real_2d_array t;
    ap::complex_2d_array q;
    int i;
    int k;
    double v;

    ap::ap_error::make_assertion(zneeded==0||zneeded==1, "HermitianEVD: incorrect ZNeeded");
    
    //
    // Reduce to tridiagonal form
    //
    hmatrixtd(a, n, isupper, tau, d, e);
    if( zneeded==1 )
    {
        hmatrixtdunpackq(a, n, isupper, tau, q);
        zneeded = 2;
    }
    
    //
    // TDEVD
    //
    result = smatrixtdevd(d, e, n, zneeded, t);
    
    //
    // Eigenvectors are needed
    // Calculate Z = Q*T = Re(Q)*T + i*Im(Q)*T
    //
    if( result&&zneeded!=0 )
    {
        work.setbounds(0, n-1);
        z.setbounds(0, n-1, 0, n-1);
        for(i = 0; i <= n-1; i++)
        {
            
            //
            // Calculate real part
            //
            for(k = 0; k <= n-1; k++)
            {
                work(k) = 0;
            }
            for(k = 0; k <= n-1; k++)
            {
                v = q(i,k).re;
                ap::vadd(&work(0), &t(k, 0), ap::vlen(0,n-1), v);
            }
            for(k = 0; k <= n-1; k++)
            {
                z(i,k).re = work(k);
            }
            
            //
            // Calculate imaginary part
            //
            for(k = 0; k <= n-1; k++)
            {
                work(k) = 0;
            }
            for(k = 0; k <= n-1; k++)
            {
                v = q(i,k).im;
                ap::vadd(&work(0), &t(k, 0), ap::vlen(0,n-1), v);
            }
            for(k = 0; k <= n-1; k++)
            {
                z(i,k).im = work(k);
            }
        }
    }
    return result;
}


/*************************************************************************
Obsolete 1-based subroutine

  -- ALGLIB --
     Copyright 2005, 23 March 2007 by Bochkanov Sergey
*************************************************************************/
bool hermitianevd(ap::complex_2d_array a,
     int n,
     int zneeded,
     bool isupper,
     ap::real_1d_array& d,
     ap::complex_2d_array& z)
{
    bool result;
    ap::complex_1d_array tau;
    ap::real_1d_array e;
    ap::real_1d_array work;
    ap::real_2d_array t;
    ap::complex_2d_array q;
    int i;
    int k;
    double v;

    ap::ap_error::make_assertion(zneeded==0||zneeded==1, "HermitianEVD: incorrect ZNeeded");
    
    //
    // Reduce to tridiagonal form
    //
    hermitiantotridiagonal(a, n, isupper, tau, d, e);
    if( zneeded==1 )
    {
        unpackqfromhermitiantridiagonal(a, n, isupper, tau, q);
        zneeded = 2;
    }
    
    //
    // TDEVD
    //
    result = tridiagonalevd(d, e, n, zneeded, t);
    
    //
    // Eigenvectors are needed
    // Calculate Z = Q*T = Re(Q)*T + i*Im(Q)*T
    //
    if( result&&zneeded!=0 )
    {
        work.setbounds(1, n);
        z.setbounds(1, n, 1, n);
        for(i = 1; i <= n; i++)
        {
            
            //
            // Calculate real part
            //
            for(k = 1; k <= n; k++)
            {
                work(k) = 0;
            }
            for(k = 1; k <= n; k++)
            {
                v = q(i,k).re;
                ap::vadd(&work(1), &t(k, 1), ap::vlen(1,n), v);
            }
            for(k = 1; k <= n; k++)
            {
                z(i,k).re = work(k);
            }
            
            //
            // Calculate imaginary part
            //
            for(k = 1; k <= n; k++)
            {
                work(k) = 0;
            }
            for(k = 1; k <= n; k++)
            {
                v = q(i,k).im;
                ap::vadd(&work(1), &t(k, 1), ap::vlen(1,n), v);
            }
            for(k = 1; k <= n; k++)
            {
                z(i,k).im = work(k);
            }
        }
    }
    return result;
}



