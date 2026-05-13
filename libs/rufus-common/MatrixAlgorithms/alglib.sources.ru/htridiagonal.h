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

#ifndef _htridiagonal_h
#define _htridiagonal_h

#include "libs/ap.h"

#include "cblas.h"
#include "creflections.h"
#include "hblas.h"


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
     ap::real_1d_array& e);


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
     ap::complex_2d_array& q);


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
     ap::real_1d_array& e);


/*************************************************************************
Obsolete 1-based subroutine

  -- ALGLIB --
     Copyright 2005, 2007 by Bochkanov Sergey
*************************************************************************/
void unpackqfromhermitiantridiagonal(const ap::complex_2d_array& a,
     const int& n,
     const bool& isupper,
     const ap::complex_1d_array& tau,
     ap::complex_2d_array& q);


#endif
