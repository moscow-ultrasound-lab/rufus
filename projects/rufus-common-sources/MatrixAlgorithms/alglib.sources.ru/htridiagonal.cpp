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
#include "htridiagonal.h"

/*************************************************************************
Приведение Эрмитовой матрицы, заданной верхним или нижним треугольником, к
трехдиагональной вещественной матрице унитарным преобразованием подобия.

Алгоритм находит такие матрицы Q (ортогональная) и  T  (трехдиагональная),
что Q'*A*Q = T

Входные параметры:
    A   -   преобразуемая матрица
            массив с нумерацией элементов [0..N-1, 0..N-1].
    N   -   размер матрицы A
    IsUpper-формат хранения. Если IsUpper=True, матрица A задана своим
            верхним треугольником, а нижний треугольник не используется  и
            не модифицируется алгоритмом, и наоборот, если  IsUpper=False.

Выходные параметры:
    A   -   матрицы T и Q в упакованной форме (см. ниже)
    Tau -   массив множителей, формирующих матрицы H(i)
            массив с нумерацией элементов [0..N-2]
    D   -   главная диагональ симметричной вещественной матрицы T.
            массив с нумерацией элементов [0..N-1]
    E   -   побочная диагональ симметричной вещественной матрицы T.
            массив с нумерацией элементов [0..N-2]


Если IsUpper=True, то матрица Q представляется, как произведение элементарных
отражений
    Q = H(n-2) . . . H(1) H(0).
где каждое H(i) имеет вид
    H(i) = I - tau * v * v'
где tau - комплексное число, хранится в Tau[I]; v - комплексный вектор,
причем v(i+1:n-1) = 0, v(i) = 1, v(0:i-1) хранится в A(0:i-1,i+1).


Если IsUpper=False, матрица Q представляется в виде
    Q = H(0) H(2) . . . H(n-2).
где каждое H(i) имеет вид
    H(i) = I - tau * v * v'
где tau - комплексное число, хранится в Tau[I]; v - комплексный вектор,
причем v(0:i) = 0, v(i+1) = 1, v(i+2:n-1) хранится в A(i+2:n-1,i).


ПРИМЕР:
 IsUpper=True, n=5:                     IsUpper=False, n=5:
     (  d   e   v1  v2  v3 )              (  d                  )
     (      d   e   v2  v3 )              (  e   d              )
     (          d   e   v3 )              (  v0  e   d          )
     (              d   e  )              (  v0  v1  e   d      )
     (                  d  )              (  v0  v1  v2  e   d  )

  -- LAPACK routine (version 3.0) --
     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
     Courant Institute, Argonne National Lab, and Rice University
     October 31, 1992
*************************************************************************/
void hmatrixtd(ap::complex_2d_array& a,
     int n,
     bool isupper,
     ap::complex_1d_array& tau,
     ap::real_1d_array& d,
     ap::real_1d_array& e)
{
    int i;
    ap::complex alpha;
    ap::complex taui;
    ap::complex v;
    ap::complex_1d_array t;
    ap::complex_1d_array t2;
    ap::complex_1d_array t3;
    int i_;
    int i1_;

    if( n<=0 )
    {
        return;
    }
    for(i = 0; i <= n-1; i++)
    {
//        ap::ap_error::make_assertion(a(i,i).y==0, "");
#pragma message ("this assertion produced a bug. ")
    }
    if( n>1 )
    {
        tau.setbounds(0, n-2);
        e.setbounds(0, n-2);
    }
    d.setbounds(0, n-1);
    t.setbounds(0, n-1);
    t2.setbounds(0, n-1);
    t3.setbounds(0, n-1);
    if( isupper )
    {
        
        //
        // Reduce the upper triangle of A
        //
        a(n-1,n-1) = a(n-1,n-1).re;
        for(i = n-2; i >= 0; i--)
        {
            
            //
            // Generate elementary reflector H = I+1 - tau * v * v'
            //
            alpha = a(i,i+1);
            t(1) = alpha;
            if( i>=1 )
            {
                i1_ = (0) - (2);
                for(i_=2; i_<=i+1;i_++)
                {
                    t(i_) = a(i_+i1_,i+1);
                }
            }
            complexgeneratereflection(t, i+1, taui);
            if( i>=1 )
            {
                i1_ = (2) - (0);
                for(i_=0; i_<=i-1;i_++)
                {
                    a(i_,i+1) = t(i_+i1_);
                }
            }
            alpha = t(1);
            e(i) = alpha.re;
            if( taui!=0 )
            {
                
                //
                // Apply H(I+1) from both sides to A
                //
                a(i,i+1) = 1;
                
                //
                // Compute  x := tau * A * v  storing x in TAU
                //
                i1_ = (0) - (1);
                for(i_=1; i_<=i+1;i_++)
                {
                    t(i_) = a(i_+i1_,i+1);
                }
                hermitianmatrixvectormultiply(a, isupper, 0, i, t, taui, t2);
                i1_ = (1) - (0);
                for(i_=0; i_<=i;i_++)
                {
                    tau(i_) = t2(i_+i1_);
                }
                
                //
                // Compute  w := x - 1/2 * tau * (x'*v) * v
                //
                v = 0.0;
                for(i_=0; i_<=i;i_++)
                {
                    v += ap::conj(tau(i_))*a(i_,i+1);
                }
                alpha = -0.5*taui*v;
                for(i_=0; i_<=i;i_++)
                {
                    tau(i_) = tau(i_) + alpha*a(i_,i+1);
                }
                
                //
                // Apply the transformation as a rank-2 update:
                //    A := A - v * w' - w * v'
                //
                i1_ = (0) - (1);
                for(i_=1; i_<=i+1;i_++)
                {
                    t(i_) = a(i_+i1_,i+1);
                }
                i1_ = (0) - (1);
                for(i_=1; i_<=i+1;i_++)
                {
                    t3(i_) = tau(i_+i1_);
                }
                hermitianrank2update(a, isupper, 0, i, t, t3, t2, ap::complex(-1));
            }
            else
            {
                a(i,i) = a(i,i).re;
            }
            a(i,i+1) = e(i);
            d(i+1) = a(i+1,i+1).re;
            tau(i) = taui;
        }
        d(0) = a(0,0).re;
    }
    else
    {
        
        //
        // Reduce the lower triangle of A
        //
        a(0,0) = a(0,0).re;
        for(i = 0; i <= n-2; i++)
        {
            
            //
            // Generate elementary reflector H = I - tau * v * v'
            //
            i1_ = (i+1) - (1);
            for(i_=1; i_<=n-i-1;i_++)
            {
                t(i_) = a(i_+i1_,i);
            }
            complexgeneratereflection(t, n-i-1, taui);
            i1_ = (1) - (i+1);
            for(i_=i+1; i_<=n-1;i_++)
            {
                a(i_,i) = t(i_+i1_);
            }
            e(i) = a(i+1,i).re;
            if( taui!=0 )
            {
                
                //
                // Apply H(i) from both sides to A(i+1:n,i+1:n)
                //
                a(i+1,i) = 1;
                
                //
                // Compute  x := tau * A * v  storing y in TAU
                //
                i1_ = (i+1) - (1);
                for(i_=1; i_<=n-i-1;i_++)
                {
                    t(i_) = a(i_+i1_,i);
                }
                hermitianmatrixvectormultiply(a, isupper, i+1, n-1, t, taui, t2);
                i1_ = (1) - (i);
                for(i_=i; i_<=n-2;i_++)
                {
                    tau(i_) = t2(i_+i1_);
                }
                
                //
                // Compute  w := x - 1/2 * tau * (x'*v) * v
                //
                i1_ = (i+1)-(i);
                v = 0.0;
                for(i_=i; i_<=n-2;i_++)
                {
                    v += ap::conj(tau(i_))*a(i_+i1_,i);
                }
                alpha = -0.5*taui*v;
                i1_ = (i+1) - (i);
                for(i_=i; i_<=n-2;i_++)
                {
                    tau(i_) = tau(i_) + alpha*a(i_+i1_,i);
                }
                
                //
                // Apply the transformation as a rank-2 update:
                // A := A - v * w' - w * v'
                //
                i1_ = (i+1) - (1);
                for(i_=1; i_<=n-i-1;i_++)
                {
                    t(i_) = a(i_+i1_,i);
                }
                i1_ = (i) - (1);
                for(i_=1; i_<=n-i-1;i_++)
                {
                    t2(i_) = tau(i_+i1_);
                }
                hermitianrank2update(a, isupper, i+1, n-1, t, t2, t3, ap::complex(-1));
            }
            else
            {
                a(i+1,i+1) = a(i+1,i+1).re;
            }
            a(i+1,i) = e(i);
            d(i) = a(i,i).re;
            tau(i) = taui;
        }
        d(n-1) = a(n-1,n-1).re;
    }
}


/*************************************************************************
"Распаковка"  матрицы  Q,  приводящей  Эрмитову  матрицу  к   вещественной
трехдиагональной форме.

Входные параметры:
    A   -   результат работы подпрограммы HMatrixTD
    N   -   размер матрицы
    IsUpper-формат хранения (параметр подпрограммы HMatrixTD)
    Tau -   результат работы подпрограммы HMatrixTD

Выходные параметры:
    Q   -   матрица преобразования.
            Массив с нумерацией элементов [0..N-1, 0..N-1]

  -- ALGLIB --
     Copyright 2005, 2007, 2008 by Bochkanov Sergey
*************************************************************************/
void hmatrixtdunpackq(const ap::complex_2d_array& a,
     const int& n,
     const bool& isupper,
     const ap::complex_1d_array& tau,
     ap::complex_2d_array& q)
{
    int i;
    int j;
    ap::complex_1d_array v;
    ap::complex_1d_array work;
    int i_;
    int i1_;

    if( n==0 )
    {
        return;
    }
    
    //
    // init
    //
    q.setbounds(0, n-1, 0, n-1);
    v.setbounds(1, n);
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
    if( isupper )
    {
        for(i = 0; i <= n-2; i++)
        {
            
            //
            // Apply H(i)
            //
            i1_ = (0) - (1);
            for(i_=1; i_<=i+1;i_++)
            {
                v(i_) = a(i_+i1_,i+1);
            }
            v(i+1) = 1;
            complexapplyreflectionfromtheleft(q, tau(i), v, 0, i, 0, n-1, work);
        }
    }
    else
    {
        for(i = n-2; i >= 0; i--)
        {
            
            //
            // Apply H(i)
            //
            i1_ = (i+1) - (1);
            for(i_=1; i_<=n-i-1;i_++)
            {
                v(i_) = a(i_+i1_,i);
            }
            v(1) = 1;
            complexapplyreflectionfromtheleft(q, tau(i), v, i+1, n-1, 0, n-1, work);
        }
    }
}


/*************************************************************************
Obsolete 1-based subroutine

  -- LAPACK routine (version 3.0) --
     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
     Courant Institute, Argonne National Lab, and Rice University
     October 31, 1992
*************************************************************************/
void hermitiantotridiagonal(ap::complex_2d_array& a,
     int n,
     bool isupper,
     ap::complex_1d_array& tau,
     ap::real_1d_array& d,
     ap::real_1d_array& e)
{
    int i;
    ap::complex alpha;
    ap::complex taui;
    ap::complex v;
    ap::complex_1d_array t;
    ap::complex_1d_array t2;
    ap::complex_1d_array t3;
    int i_;
    int i1_;

    if( n<=0 )
    {
        return;
    }
    for(i = 1; i <= n; i++)
    {
        ap::ap_error::make_assertion(a(i,i).im==0, "");
    }
    tau.setbounds(1, ap::maxint(1, n-1));
    d.setbounds(1, n);
    e.setbounds(1, ap::maxint(1, n-1));
    t.setbounds(1, n);
    t2.setbounds(1, n);
    t3.setbounds(1, n);
    if( isupper )
    {
        
        //
        // Reduce the upper triangle of A
        //
        a(n,n) = a(n,n).re;
        for(i = n-1; i >= 1; i--)
        {
            
            //
            // Generate elementary reflector H(i) = I - tau * v * v'
            // to annihilate A(1:i-1,i+1)
            //
            alpha = a(i,i+1);
            t(1) = alpha;
            if( i>=2 )
            {
                i1_ = (1) - (2);
                for(i_=2; i_<=i;i_++)
                {
                    t(i_) = a(i_+i1_,i+1);
                }
            }
            complexgeneratereflection(t, i, taui);
            if( i>=2 )
            {
                i1_ = (2) - (1);
                for(i_=1; i_<=i-1;i_++)
                {
                    a(i_,i+1) = t(i_+i1_);
                }
            }
            alpha = t(1);
            e(i) = alpha.re;
            if( taui!=0 )
            {
                
                //
                // Apply H(i) from both sides to A(1:i,1:i)
                //
                a(i,i+1) = 1;
                
                //
                // Compute  x := tau * A * v  storing x in TAU(1:i)
                //
                for(i_=1; i_<=i;i_++)
                {
                    t(i_) = a(i_,i+1);
                }
                hermitianmatrixvectormultiply(a, isupper, 1, i, t, taui, tau);
                
                //
                // Compute  w := x - 1/2 * tau * (x'*v) * v
                //
                v = 0.0;
                for(i_=1; i_<=i;i_++)
                {
                    v += ap::conj(tau(i_))*a(i_,i+1);
                }
                alpha = -0.5*taui*v;
                for(i_=1; i_<=i;i_++)
                {
                    tau(i_) = tau(i_) + alpha*a(i_,i+1);
                }
                
                //
                // Apply the transformation as a rank-2 update:
                //    A := A - v * w' - w * v'
                //
                for(i_=1; i_<=i;i_++)
                {
                    t(i_) = a(i_,i+1);
                }
                hermitianrank2update(a, isupper, 1, i, t, tau, t2, ap::complex(-1));
            }
            else
            {
                a(i,i) = a(i,i).re;
            }
            a(i,i+1) = e(i);
            d(i+1) = a(i+1,i+1).re;
            tau(i) = taui;
        }
        d(1) = a(1,1).re;
    }
    else
    {
        
        //
        // Reduce the lower triangle of A
        //
        a(1,1) = a(1,1).re;
        for(i = 1; i <= n-1; i++)
        {
            
            //
            // Generate elementary reflector H(i) = I - tau * v * v'
            // to annihilate A(i+2:n,i)
            //
            i1_ = (i+1) - (1);
            for(i_=1; i_<=n-i;i_++)
            {
                t(i_) = a(i_+i1_,i);
            }
            complexgeneratereflection(t, n-i, taui);
            i1_ = (1) - (i+1);
            for(i_=i+1; i_<=n;i_++)
            {
                a(i_,i) = t(i_+i1_);
            }
            e(i) = a(i+1,i).re;
            if( taui!=0 )
            {
                
                //
                // Apply H(i) from both sides to A(i+1:n,i+1:n)
                //
                a(i+1,i) = 1;
                
                //
                // Compute  x := tau * A * v  storing y in TAU(i:n-1)
                //
                i1_ = (i+1) - (1);
                for(i_=1; i_<=n-i;i_++)
                {
                    t(i_) = a(i_+i1_,i);
                }
                hermitianmatrixvectormultiply(a, isupper, i+1, n, t, taui, t2);
                i1_ = (1) - (i);
                for(i_=i; i_<=n-1;i_++)
                {
                    tau(i_) = t2(i_+i1_);
                }
                
                //
                // Compute  w := x - 1/2 * tau * (x'*v) * v
                //
                i1_ = (i+1)-(i);
                v = 0.0;
                for(i_=i; i_<=n-1;i_++)
                {
                    v += ap::conj(tau(i_))*a(i_+i1_,i);
                }
                alpha = -0.5*taui*v;
                i1_ = (i+1) - (i);
                for(i_=i; i_<=n-1;i_++)
                {
                    tau(i_) = tau(i_) + alpha*a(i_+i1_,i);
                }
                
                //
                // Apply the transformation as a rank-2 update:
                // A := A - v * w' - w * v'
                //
                i1_ = (i+1) - (1);
                for(i_=1; i_<=n-i;i_++)
                {
                    t(i_) = a(i_+i1_,i);
                }
                i1_ = (i) - (1);
                for(i_=1; i_<=n-i;i_++)
                {
                    t2(i_) = tau(i_+i1_);
                }
                hermitianrank2update(a, isupper, i+1, n, t, t2, t3, ap::complex(-1));
            }
            else
            {
                a(i+1,i+1) = a(i+1,i+1).re;
            }
            a(i+1,i) = e(i);
            d(i) = a(i,i).re;
            tau(i) = taui;
        }
        d(n) = a(n,n).re;
    }
}


/*************************************************************************
Obsolete 1-based subroutine

  -- ALGLIB --
     Copyright 2005, 2007 by Bochkanov Sergey
*************************************************************************/
void unpackqfromhermitiantridiagonal(const ap::complex_2d_array& a,
     const int& n,
     const bool& isupper,
     const ap::complex_1d_array& tau,
     ap::complex_2d_array& q)
{
    int i;
    int j;
    ap::complex_1d_array v;
    ap::complex_1d_array work;
    int i_;
    int i1_;

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
    if( isupper )
    {
        for(i = 1; i <= n-1; i++)
        {
            
            //
            // Apply H(i)
            //
            for(i_=1; i_<=i;i_++)
            {
                v(i_) = a(i_,i+1);
            }
            v(i) = 1;
            complexapplyreflectionfromtheleft(q, tau(i), v, 1, i, 1, n, work);
        }
    }
    else
    {
        for(i = n-1; i >= 1; i--)
        {
            
            //
            // Apply H(i)
            //
            i1_ = (i+1) - (1);
            for(i_=1; i_<=n-i;i_++)
            {
                v(i_) = a(i_+i1_,i);
            }
            v(1) = 1;
            complexapplyreflectionfromtheleft(q, tau(i), v, i+1, n, 1, n, work);
        }
    }
}



