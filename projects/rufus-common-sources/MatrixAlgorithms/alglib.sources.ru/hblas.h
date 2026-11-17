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

#ifndef _hblas_h
#define _hblas_h

#include "libs/ap.h"

/*************************************************************************
Умножение подматрицы эрмитовой матрицы на вектор.

Алгоритм умножает подматрицу эрмитовой матрицы  A,  заданной  верхним  или
нижним треугольником, на вектор (справа):

y = alpha*A*x

Входные параметры:
    A   -   матрица. Массив с нумерацией элементов как минимум  от  I1  до
            I2 по обеим размерностям.
    IsUpper-формат хранения (верхний или нижний треугольник).
    I1  -   первые столбец и строка начподматрицы
    I2  -   последние столбец и строка начподматрицы
    X   -   вектор. Массив с нумерацией элементов [1..I2-I1+1]
    Alpha-  скалярный множитель
    Y   -   заранее размещенный в памяти массив
            с нумерацией элементов [1..I2-I1+1].

Выходные параметры:
    Y   -   результат. Массив с нумерацией элементов [1..I2-I1+1]
*************************************************************************/
void hermitianmatrixvectormultiply(const ap::complex_2d_array& a,
     bool isupper,
     int i1,
     int i2,
     const ap::complex_1d_array& x,
     ap::complex alpha,
     ap::complex_1d_array& y);


/*************************************************************************
Симметричное обновление ранга 2

Алгоритм осуществляет следующую операцию с симметричной матрецей A:

A = alpha*x*conj(y') + conj(alpha)*y*conj(x') + A

Входные параметры:
    A   -   матрица. Массив с нумерацией элементов как минимум  от  I1  до
            I2 по обеим размерностям.
    IsUpper-формат хранения (верхний или нижний треугольник).
    I1  -   первые столбец и строка подматрицы
    I2  -   последние столбец и строка подматрицы
    X   -   вектор. Массив с нумерацией элементов [1..I2-I1+1]
    Y   -   вектор. Массив с нумерацией элементов [1..I2-I1+1]
    T   -   временный массив с нумерацией элементов [1..I2-I1+1], заранее
            размещенный в памяти. модифицируется подпрограммой.
    Alpha-  скалярный множитель

Выходные параметры:
    A   -   модифициованная матрица
*************************************************************************/
void hermitianrank2update(ap::complex_2d_array& a,
     bool isupper,
     int i1,
     int i2,
     const ap::complex_1d_array& x,
     const ap::complex_1d_array& y,
     ap::complex_1d_array& t,
     ap::complex alpha);


#endif
