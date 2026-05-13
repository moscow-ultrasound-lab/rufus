/*************************************************************************
Copyright (c) 1992-2007 The University of Tennessee. All rights reserved.

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
#include "hessenberg.h"
#include "creflections.h"

/*************************************************************************
Приведение квадратной матрицы к верхней форме Хессенберга: Q'*A*Q = H,
где Q - ортогональная матрица, H - матрица Хессенберга.

Входные параметры:
    A   -   матрица A. Нумерация элементов: [0..N-1, 0..N-1]
    N   -   размер матрицы A

Выходные параметры:
    A   -   матрицы Q и R в компактной форме (см. ниже)
    Tau -   массив скалярных множителей, участвующих в формировании
            матрицы Q. Нумерация элементов [0.. N-1]

После завершения работы подпрограммы на главной диагонали матрицы A, нижней
побочной диагонали и выше главной диагонали располагаются элементы матрицы H.
В массиве Tau и под нижней побочной диагональю матрицы A располагаются
элементы, формирующие матрицу Q, следующим способом:

Матрица Q представляется, как произведение элементарных отражений

Q = H(0)*H(2)*...*H(n-2),

где каждое H(i) имеет вид

H(i) = 1 - tau * v * (v^T)

где tau скалярная величина, хранящаяся в Tau[I], а v - вещественный вектор
у которого v(0:i)=0, v(i+1)=1, v(i+2:n-1) хранится в элементах A(i+2:n-1,i).

  -- LAPACK routine (version 3.0) --
     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
     Courant Institute, Argonne National Lab, and Rice University
     October 31, 1992
*************************************************************************/
void rmatrixhessenberg(ap::real_2d_array& a, int n, ap::real_1d_array& tau)
{
    int i;
//    double aii;
    double v;
    ap::real_1d_array t;
    ap::real_1d_array work;

    ap::ap_error::make_assertion(n>=0, "RMatrixHessenberg: incorrect N!");
    
    //
    // Quick return if possible
    //
    if( n<=1 )
    {
        return;
    }
    tau.setbounds(0, n-2);
    t.setbounds(1, n);
    work.setbounds(0, n-1);
    for(i = 0; i <= n-2; i++)
    {
        
        //
        // Compute elementary reflector H(i) to annihilate A(i+2:ihi,i)
        //
        ap::vmove(t.getvector(1, n-i-1), a.getcolumn(i, i+1, n-1));
        generatereflection(t, n-i-1, v);
        ap::vmove(a.getcolumn(i, i+1, n-1), t.getvector(1, n-i-1));
        tau(i) = v;
        t(1) = 1;
        
        //
        // Apply H(i) to A(1:ihi,i+1:ihi) from the right
        //
        applyreflectionfromtheright(a, v, t, 0, n-1, i+1, n-1, work);
        
        //
        // Apply H(i) to A(i+1:ihi,i+1:n) from the left
        //
        applyreflectionfromtheleft(a, v, t, i+1, n-1, i+1, n-1, work);
    }
}


/*************************************************************************
"распаковка" матрицы Q, приводящей A к верхней форме Хессенберга

Входные параметры:
    A       -   результат работы подпрограммы RMatrixHessenberg
    N       -   размер матрицы A
    Tau     -   скалярные множители, формирующие Q.
                Результат работы RMatrixHessenberg

Выходные параметры:
    Q       -   матрица Q. Массив с нумерацией элементов [0..N-1, 0..N-1].

  -- ALGLIB --
     Copyright 2005 by Bochkanov Sergey
*************************************************************************/
void rmatrixhessenbergunpackq(const ap::real_2d_array& a,
     int n,
     const ap::real_1d_array& tau,
     ap::real_2d_array& q)
{
    int i;
    int j;
    ap::real_1d_array v;
    ap::real_1d_array work;

    if( n==0 )
    {
        return;
    }
    
    //
    // init
    //
    q.setbounds(0, n-1, 0, n-1);
    v.setbounds(0, n-1);
    work.setbounds(0, n-1);
    for(i = 0; i <= n-1; i++)
    {
        for(j = 0; j <= n-1; j++)
        {
            if( i==j )
            {
                q(i,j) = 1;
            }
            else
            {
                q(i,j) = 0;
            }
        }
    }
    
    //
    // unpack Q
    //
    for(i = 0; i <= n-2; i++)
    {
        
        //
        // Apply H(i)
        //
        ap::vmove(v.getvector(1, n-i-1), a.getcolumn(i, i+1, n-1));
        v(1) = 1;
        applyreflectionfromtheright(q, tau(i), v, 0, n-1, i+1, n-1, work);
    }
}


/*************************************************************************
"распаковка" матрицы H (результата приводения A к верхней форме Хессенберга)

Входные параметры:
    A       -   результат работы подпрограммы RMatrixHessenberg
    N       -   размер матрицы A

Выходные параметры:
    H       -   матрица H. Массив с нумерацией элементов [0..N-1, 0..N-1].

  -- ALGLIB --
     Copyright 2005 by Bochkanov Sergey
*************************************************************************/
void rmatrixhessenbergunpackh(const ap::real_2d_array& a,
     int n,
     ap::real_2d_array& h)
{
    int i;
    int j;
    ap::real_1d_array v;
    ap::real_1d_array work;
//    int ip1;
//    int nmi;

    if( n==0 )
    {
        return;
    }
    h.setbounds(0, n-1, 0, n-1);
    for(i = 0; i <= n-1; i++)
    {
        for(j = 0; j <= i-2; j++)
        {
            h(i,j) = 0;
        }
        j = ap::maxint(0, i-1);
        ap::vmove(&h(i, j), &a(i, j), ap::vlen(j,n-1));
    }
}


/*************************************************************************
Obsolete 1-based subroutine.
See RMatrixHessenberg for 0-based replacement.
*************************************************************************/
void toupperhessenberg(ap::real_2d_array& a, int n, ap::real_1d_array& tau)
{
    int i;
    int ip1;
    int nmi;
//    double aii;
    double v;
    ap::real_1d_array t;
    ap::real_1d_array work;

    ap::ap_error::make_assertion(n>=0, "ToUpperHessenberg: incorrect N!");
    
    //
    // Quick return if possible
    //
    if( n<=1 )
    {
        return;
    }
    tau.setbounds(1, n-1);
    t.setbounds(1, n);
    work.setbounds(1, n);
    for(i = 1; i <= n-1; i++)
    {
        
        //
        // Compute elementary reflector H(i) to annihilate A(i+2:ihi,i)
        //
        ip1 = i+1;
        nmi = n-i;
        ap::vmove(t.getvector(1, nmi), a.getcolumn(i, ip1, n));
        generatereflection(t, nmi, v);
        ap::vmove(a.getcolumn(i, ip1, n), t.getvector(1, nmi));
        tau(i) = v;
        t(1) = 1;
        
        //
        // Apply H(i) to A(1:ihi,i+1:ihi) from the right
        //
        applyreflectionfromtheright(a, v, t, 1, n, i+1, n, work);
        
        //
        // Apply H(i) to A(i+1:ihi,i+1:n) from the left
        //
        applyreflectionfromtheleft(a, v, t, i+1, n, i+1, n, work);
    }
}


void toupperhessenberg_c(ap::complex_2d_array& a, int n, ap::complex_1d_array& tau)
{
	int i;
	int ip1;
	int nmi;
	xrad::complexF64 v;
	ap::complex_1d_array t;
	ap::complex_1d_array work;
	ap::ap_error::make_assertion(n>=0, "ToUpperHessenberg: incorrect N!");
	if( n<=1 )
	{
		return;
	}
	tau.setbounds(1, n-1);
	t.setbounds(1, n);
	work.setbounds(1, n);
	for(i = 1; i <= n-1; i++)
	{
		ip1 = i+1;
		nmi = n-i;
		ap::vmove(t.getvector(1, nmi), a.getcolumn(i, ip1, n));
		complexgeneratereflection(t, nmi, v);
		ap::vmove(a.getcolumn(i, ip1, n), t.getvector(1, nmi));
		tau(i) = v;
		t(1) = 1;
		applyreflectionfromtheright_c(a, v, t, 1, n, i+1, n, work);
		applyreflectionfromtheleft_c(a, v, t, i+1, n, i+1, n, work);
	}
}


/*************************************************************************
Obsolete 1-based subroutine.
See RMatrixHessenbergUnpackQ for 0-based replacement.
*************************************************************************/
void unpackqfromupperhessenberg(const ap::real_2d_array& a,
     int n,
     const ap::real_1d_array& tau,
     ap::real_2d_array& q)
{
    int i;
    int j;
    ap::real_1d_array v;
    ap::real_1d_array work;
    int ip1;
    int nmi;

    if( n==0 )
    {
        return;
    }
    
    //
    // init
    //
    q.setbounds(1, n, 1, n);
    v.setbounds(1, n);
    work.setbounds(1, n);
    for(i = 1; i <= n; i++)
    {
        for(j = 1; j <= n; j++)
        {
            if( i==j )
            {
                q(i,j) = 1;
            }
            else
            {
                q(i,j) = 0;
            }
        }
    }
    
    //
    // unpack Q
    //
    for(i = 1; i <= n-1; i++)
    {
        
        //
        // Apply H(i)
        //
        ip1 = i+1;
        nmi = n-i;
        ap::vmove(v.getvector(1, nmi), a.getcolumn(i, ip1, n));
        v(1) = 1;
        applyreflectionfromtheright(q, tau(i), v, 1, n, i+1, n, work);
    }
}

#if 0
//kns
/*************************************************************************
Obsolete 1-based subroutine.
See RMatrixHessenbergUnpackH for 0-based replacement.
*************************************************************************/
void unpackhfromupperhessenberg(const ap::real_2d_array& a,
     int n,
     const ap::real_1d_array& /*tau*/,
     ap::real_2d_array& h)
{
//#pragma unused (tau)
    int i;
    int j;
    ap::real_1d_array v;
    ap::real_1d_array work;
//    int ip1;
//    int nmi;

    if( n==0 )
    {
        return;
    }
    h.setbounds(1, n, 1, n);
    for(i = 1; i <= n; i++)
    {
        for(j = 1; j <= i-2; j++)
        {
            h(i,j) = 0;
        }
        j = ap::maxint(1, i-1);
        ap::vmove(&h(i, j), &a(i, j), ap::vlen(j,n));
    }
}
#endif //0


