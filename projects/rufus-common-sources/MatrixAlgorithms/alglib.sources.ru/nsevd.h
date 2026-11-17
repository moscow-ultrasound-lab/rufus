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

#ifndef _nsevd_h
#define _nsevd_h

#include "libs/ap.h"

#include "blas.h"
#include "reflections.h"
#include "rotations.h"
#include "hsschur.h"
#include "hessenberg.h"


/*************************************************************************
Поиск собственных чисел и векторов матрицы общего вида

Алгоритм ищет собственные пары  матрицы общего вида, используя QR-алгоритм
с множественными сдвигами. Алгоритм позволяет найти  собственные  числа  и
собственные векторы, как правые, так и левые.

При этом правый  собственный вектор - это такой вектор x, что A*x = w*x, а
левый собственный вектор - это такой вектор y,  что y'*A = w*y' (здесь  y'
обозначает комплексно-сопряженное транспонирование вектора).

Входные параметры:
    A       -   матрица. Массив с нумерацией элементов [0..N-1, 0..N-1]
    N       -   размер матрицы
    VNeeded -   флаг, сообщающий, требуются ли собственные векторы.
                Если VNeeded:
                 * равно 0, то собственные векторы не возвращаются
                 * равно 1, то возвращаются правые собственные векторы
                 * равно 2, то возвращаются левые собственные векторы
                 * равно 3, то возвращаются правые и левые собственные векторы

Выходные параметры:
    WR      -   Вещественные части собственных значений.
                Массив с нумерацией элементов [0..N-1]
    WI      -   Мнимые части собственных значений.
                Массив с нумерацией элементов [0..N-1]
    VL, VR  -   Массивы левых и правых собственных векторов (если запрошены
                собственные векторы).
                Если WI[i] равно 0, то i-ое собственное число - вещественное,
                и ему соответствует i-ый столбец матриц VL/VR.
                Если WI[i]>0, то мы имеем пару комплексно сопряженных чисел с
                положительной и отрицательной мнимыми частями:
                    первое собственное число WR[i]   + sqrt(-1)*WI[i]
                    второе собственное число WR[i+1] + sqrt(-1)*WI[i+1]
                    WI[i]>0
                    WI[i+1] = -WI[i] < 0
                При этом собственный вектор, соответствующий первому
                собственному числу, занимает i-ый и i+1-ый столбцы матриц
                VL/VR (в i-ом столбце вещественная часть, в i+1-ом мнимая),
                а вектор, соответствующий второму числу, комплексно сопряжен
                первому вектору.
                Массивы с нумерацией элементов [0..N-1, 0..N-1].

Результат:
    True, если алгоритм сошелся
    False, если алгоритм не сошелся

Замечание 1:
        У некоторых пользователей подпрограммы возникает вопрос - а что
    будет, если WI[N-1]>0? Согласно описанию алгоритма, в WI[N] должно
    храниться собственное значение, комплексно сопряженное N-ому, но
    массив имеет размер N, а не N+1.
        Ответ - подобная ситуация не может возникнуть, т.к. алгоритм
    всегда находит собственные значения парами, т.е. если WI[I]>0, то
    тогда I строго меньше N-1.

Замечание 2:
    быстродействие алгоритма существенно зависит от внутреннего параметра
    NS подпрограммы InternalSchurDecomposition, определяющего число сдвигов
    в QR-алгоритме (аналог ширины блока в блочно-матричных алгоритмах
    линейной алгебры). В случае, если вы желаете добиться максимального
    быстродействия на вашей машине, рекомендуется ручная настройка этого
    параметра.

См. также подпрограмму InternalTREVC

Алгоритм основан на пакете LAPACK 3.0
*************************************************************************/
bool rmatrixevd(ap::real_2d_array a,
     int n,
     int vneeded,
     ap::real_1d_array& wr,
     ap::real_1d_array& wi,
     ap::real_2d_array& vl,
     ap::real_2d_array& vr);

bool rmatrixevd_c(ap::complex_2d_array a,
				int n,
				int vneeded,
				ap::real_1d_array& wr,
				ap::real_1d_array& wi,
				ap::real_2d_array& vl,
				ap::real_2d_array& vr);


/*************************************************************************
Obsolete 1-based subroutine
*************************************************************************/
bool nonsymmetricevd(ap::real_2d_array a,
     int n,
     int vneeded,
     ap::real_1d_array& wr,
     ap::real_1d_array& wi,
     ap::real_2d_array& vl,
     ap::real_2d_array& vr);

bool nonsymmetricevd_c(ap::complex_2d_array a,
					 int n,
					 int vneeded,
					 ap::real_1d_array& wr,
					 ap::real_1d_array& wi,
					 ap::real_2d_array& vl,
					 ap::real_2d_array& vr);


#endif
