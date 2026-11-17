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

#ifndef _tdevd_h
#define _tdevd_h

#include "libs/ap.h"

#include "blas.h"
#include "rotations.h"


/*************************************************************************
Поиск собственных чисел и векторов трехдиагональной симметричной матрицы

Алгоритм ищет  собственные  пары  трехдиагональной  симметричной  матрицы,
используя неявный QL/QR алгоритм.

Входные параметры:
    D   -       главная диагональ трехдиагональной матрицы.
                Массив с нумерацией элементов [0..N-1].
    E   -       побочная диагональ трехдиагональной матрицы.
                Массив с нумерацией элементов [0..N-2].
    N   -       размер матрицы
    ZNeeded-    флаг, сообщающий, требуются ли собственные векторы.
                Если ZNeeded:
                 * равно 0, то собственные векторы не требуются
                 * равно 1, то собственные векторы трехдиагональной матрицы
                   умножаются на квадратную матрицу Z. Используется,  если
                   трехдиагональная матрица получена преобразованием
                   подобия симметричной матрицы
                 * равно 2, то собственные векторы трехдиагональной матрицы
                   замещают квадратную матрицу Z.
                 * равно 3, то в матрице Z возвращаются первая строка
                   матрицы собственных векторов трехдиагональной матрицы.
    Z   -       если ZNeeded=1, то содержит квадратную матрицу, на которую
                умножаются собственные векторы.
                Массив с нумерацией элементов [0..N-1, 0..N-1]

Выходные параметры:
    D   -       собственные числа в порядке возрастания.
                Массив с нумерацией элементов [0..N-1].
    Z   -       если ZNeeded:
                 * равно 0, не модифицируется
                 * равно 1, содержит произведение исходной матрицы
                  (слева) и матрицы собственных векторов (справа)
                 * равно 2, содержит матрицу собственных векторов.
                 * равно 3, содержит первую строку матрицы собственных
                   векторов.
                При ZNeeded<3 - массив с нумерацией элементов [0..N-1, 0..N-1],
                при этом собственные векторы хранятся в столбцах матрицы.
                При ZNeeded=3 - массив с нумерацией элементов [0..0, 0..N-1]

Результат:
    True, если алгоритм сошелся
    False, если алгоритм не сошелся (редчайший случай)

  -- LAPACK routine (version 3.0) --
     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
     Courant Institute, Argonne National Lab, and Rice University
     September 30, 1994
*************************************************************************/
bool smatrixtdevd(ap::real_1d_array& d,
     ap::real_1d_array e,
     int n,
     int zneeded,
     ap::real_2d_array& z);


/*************************************************************************
Obsolete 1-based subroutine.
*************************************************************************/
bool tridiagonalevd(ap::real_1d_array& d,
     ap::real_1d_array e,
     int n,
     int zneeded,
     ap::real_2d_array& z);


#endif
