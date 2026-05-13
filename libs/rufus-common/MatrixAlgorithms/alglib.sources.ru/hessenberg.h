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

#ifndef _hessenberg_h
#define _hessenberg_h

#include "libs/ap.h"

#include "reflections.h"


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
void rmatrixhessenberg(ap::real_2d_array& a, int n, ap::real_1d_array& tau);


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
     ap::real_2d_array& q);


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
     ap::real_2d_array& h);


/*************************************************************************
Obsolete 1-based subroutine.
See RMatrixHessenberg for 0-based replacement.
*************************************************************************/
void toupperhessenberg(ap::real_2d_array& a, int n, ap::real_1d_array& tau);
void toupperhessenberg_c(ap::complex_2d_array& a, int n, ap::complex_1d_array& tau);

/*************************************************************************
Obsolete 1-based subroutine.
See RMatrixHessenbergUnpackQ for 0-based replacement.
*************************************************************************/
void unpackqfromupperhessenberg(const ap::real_2d_array& a,
     int n,
     const ap::real_1d_array& tau,
     ap::real_2d_array& q);


/*************************************************************************
Obsolete 1-based subroutine.
See RMatrixHessenbergUnpackH for 0-based replacement.
*************************************************************************/
void unpackhfromupperhessenberg(const ap::real_2d_array& a,
     int n,
     const ap::real_1d_array& tau,
     ap::real_2d_array& h);


#endif
