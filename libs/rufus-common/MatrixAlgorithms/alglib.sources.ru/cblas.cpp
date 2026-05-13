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

#include "cblas.h"

/*************************************************************************
Операция y := alpha*op(A)*x + beta*y
где op(X) равно X, X', conj(X), conj(X').

Подпрограмма использует рабочий массив T с нумерацией элементов [IX1..IX2]

Аналогично CGEMV из стандартной BLAS
*************************************************************************/
void complexmatrixvectormultiply(const ap::complex_2d_array& a,
     int i1,
     int i2,
     int j1,
     int j2,
     bool transa,
     bool conja,
     const ap::complex_1d_array& x,
     int ix1,
     int ix2,
     ap::complex alpha,
     ap::complex_1d_array& y,
     int iy1,
     int iy2,
     ap::complex beta,
     ap::complex_1d_array& t)
{
    int i;
    ap::complex v;
    int i_;
    int i1_;

    if( !transa )
    {
        
        //
        // y := alpha*A*x + beta*y
        //
        // or
        //
        // y := alpha*conj(A)*x + beta*y
        //
        if( i1>i2||j1>j2 )
        {
            return;
        }
        ap::ap_error::make_assertion(j2-j1==ix2-ix1, "ComplexMatrixVectorMultiply: A and X dont match!");
        ap::ap_error::make_assertion(i2-i1==iy2-iy1, "ComplexMatrixVectorMultiply: A and Y dont match!");
        
        //
        // beta*y
        //
        if( beta==0 )
        {
            for(i = iy1; i <= iy2; i++)
            {
                y(i) = 0;
            }
        }
        else
        {
            for(i_=iy1; i_<=iy2;i_++)
            {
                y(i_) = beta*y(i_);
            }
        }
        
        //
        // conj?
        //
        if( conja )
        {
            for(i_=ix1; i_<=ix2;i_++)
            {
                t(i_) = ap::conj(x(i_));
            }
            alpha = ap::conj(alpha);
            for(i_=iy1; i_<=iy2;i_++)
            {
                y(i_) = ap::conj(y(i_));
            }
        }
        else
        {
            for(i_=ix1; i_<=ix2;i_++)
            {
                t(i_) = x(i_);
            }
        }
        
        //
        // alpha*A*x
        //
        for(i = i1; i <= i2; i++)
        {
            i1_ = (ix1)-(j1);
            v = 0.0;
            for(i_=j1; i_<=j2;i_++)
            {
                v += a(i,i_)*x(i_+i1_);
            }
            y(iy1+i-i1) = y(iy1+i-i1)+alpha*v;
        }
        
        //
        // conj?
        //
        if( conja )
        {
            for(i_=iy1; i_<=iy2;i_++)
            {
                y(i_) = ap::conj(y(i_));
            }
        }
    }
    else
    {
        
        //
        // y := alpha*A'*x + beta*y;
        //
        // or
        //
        // y := alpha*conj(A')*x + beta*y;
        //
        if( i1>i2||j1>j2 )
        {
            return;
        }
        ap::ap_error::make_assertion(i2-i1==ix2-ix1, "ComplexMatrixVectorMultiply: A and X dont match!");
        ap::ap_error::make_assertion(j2-j1==iy2-iy1, "ComplexMatrixVectorMultiply: A and Y dont match!");
        
        //
        // beta*y
        //
        if( beta==0 )
        {
            for(i = iy1; i <= iy2; i++)
            {
                y(i) = 0;
            }
        }
        else
        {
            for(i_=iy1; i_<=iy2;i_++)
            {
                y(i_) = beta*y(i_);
            }
        }
        
        //
        // conj?
        //
        if( conja )
        {
            for(i_=ix1; i_<=ix2;i_++)
            {
                t(i_) = ap::conj(x(i_));
            }
            alpha = ap::conj(alpha);
            for(i_=iy1; i_<=iy2;i_++)
            {
                y(i_) = ap::conj(y(i_));
            }
        }
        else
        {
            for(i_=ix1; i_<=ix2;i_++)
            {
                t(i_) = x(i_);
            }
        }
        
        //
        // alpha*A'*x
        //
        for(i = i1; i <= i2; i++)
        {
            v = alpha*x(ix1+i-i1);
            i1_ = (j1) - (iy1);
            for(i_=iy1; i_<=iy2;i_++)
            {
                y(i_) = y(i_) + v*a(i,i_+i1_);
            }
        }
        
        //
        // conj?
        //
        if( conja )
        {
            for(i_=iy1; i_<=iy2;i_++)
            {
                y(i_) = ap::conj(y(i_));
            }
        }
    }
}



