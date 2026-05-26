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

#ifndef _hsschur_h
#define _hsschur_h

#include "libs/ap.h"

#include "blas.h"
#include "reflections.h"
#include "rotations.h"


/*************************************************************************
Подпрограмма, осуществляющая разложение Шура матрицы в верхней форме
Хессенберга при помощи QR-алгоритма с множественными сдвигами.

Передаваемая матрица H представляется в виде S'*H*S = T, где H - матрица
в верхней форме Хессенберга, S - ортогональная матрица (векторы Шура),
T - верхняя квази-треугольная матрица (с блоками 1x1 и 2x2 на главной
диагонали).

Входные параметры:
    H       -   разлагаемая матрица
                Массив с нумерацией элементов [1..N, 1..N]
    N       -   размерность задачи, N>=0


Выходные параметры:
    H       -   содержит матрицу T. Массив с нумерацией
                элементов [1..N, 1..N]. Все элементы, располагающиеся
                ниже блоков на главной диагонали матрицы, равны 0.
    S       -   содержит векторы Шура
                массив с нумерацией элементов [1..N, 1..N]

Замечание 1:
    структура блоков матрицы T может быть легко определена путем изучения
    поддиагонали главной диагонали матрицы - поскольку все элементы,
    лежащие ниже блоков, равны 0, то элементы a[i+1,i], равные 0,
    могут служить, как индикатор границы блока.
    
Замечание 2:
    быстродействие алгоритма существенно зависит от внутреннего параметра
    NS подпрограммы InternalSchurDecomposition, определяющего число сдвигов
    в QR-алгоритме (аналог ширины блока в блочно-матричных алгоритмах
    линейной алгебры). В случае, если вы желаете добиться максимального
    быстродействия на вашей машине, рекомендуется ручная настройка этого
    параметра.

Результат:
    True,  если алгоритм сошелся и параметры H и S содержат результат
    False, если алгоритм не сошелся

Алгоритм построен на основе подпрограммы DHSEQR из пакета LAPACK 3.0
*************************************************************************/
bool upperhessenbergschurdecomposition(ap::real_2d_array& h,
     int n,
     ap::real_2d_array& s);


/*************************************************************************
Внутренняя подпрограмма, осуществляющая разложение Шура и вычисление
собственных значений матрицы в верхней форме Хессенберга при помощи
QR-алгоритма с множественными сдвигами.

Передаваемая матрица H представляется в виде Z'*H*Z = T, где H - матрица
в верхней форме Хессенберга, Z - ортогональная матрица (векторы Шура),
T - верхняя квази-треугольная матрица (с блоками 1x1 и 2x2 на главной
диагонали).

Входные параметры:
    H       -   разлагаемая матрица в верхней форме Хессенберга.
                Массив с нумерацией элементов [1..N, 1..N]
    N       -   размерность задачи, N>=0
    TNeeded -   флаг, сообщающий, требуется ли матрица T. Если TNeeded:
                 * равно 0, матрица T не возвращается
                 * равно 1, возвращается матрица T
    ZNeeded -   флаг, сообщающий, требуются ли векторы Шура. Если ZNeeded:
                 * равно 0, матрица Z не возвращается
                 * равно 1, векторы Шура умножаются на матрицу Z
                 * равно 2, векторы Шура замещают матрицу Z
    Z       -   если ZNeeded равно 1, содержит матрицу, на которую
                умножаются векторы Шура. В ином случае её содержимое
                не играет роли.
                Массив с нумерацией элементов [1..N, 1..N].


Выходные параметры:
    H       -   если TNeeded=1, содержит матрицу T (массив с нумерацией
                элементов [1..N, 1..N]), в противном случае содержимое
                этой матрицы не определено (уничтожается в ходе работы
                алгоритма).
    WR, WI  -   содержат собственные числа матрицы H (вещественная часть
                I-ого значения в WR[I], мнимая часть в WI[I]). При этом
                комплексно сопряженные пары значений идут в массиве подряд,
                сначала значение с положительной мнимой частью, хранящееся
                в (WR[I], WI[I]), затем значение с отрицательной мнимой
                частью, хранящееся в (WR[I+1], WI[I+1]). Вещественным
                значениям соответствует пара (WR[I], WI[I]) с WI[I]=0.
                Если TNeeded=1, то значения идут в том же порядке, что
                и блоки на диагонали матрицы T.
                Массивы с нумерацией элементов [1..N]
    Z       -   векторы Шура или их произведение с исходной матрицей Z
                в зависимости от ZNeeded
    INFO    -   флаг завершения:
                 * равно 0, если алгоритм успешно сошелся, при этом
                   параметры WR, WI, H и Z содержат информацию (в
                   соответствии с переданными флагами ZNeeded и TNeeded).
                 * равно I, если алгоритм смог вычислить только N-I
                   собственных значений (с I+1-ого по N-ое), при этом WR и
                   WI содержат те собственные значения, которые были
                   найдены, в позициях с I+1-ой по N-ую, а содержимое H
                   и Z не определено.

Замечание 1:
        У некоторых пользователей подпрограммы возникает вопрос - а что
    будет, если WI[N]>0? Согласно описанию алгоритма, в WI[N+1] должно
    храниться собственное значение, комплексно сопряженное N-ому, но
    массив имеет размер N, а не N+1.
        Ответ - подобная ситуация не может возникнуть, т.к. алгоритм
    всегда находит собственные значения парами, т.е. если WI[I]>0, то
    тогда I строго меньше N.

Замечание 2:
    быстродействие алгоритма существенно зависит от внутреннего параметра
    NS, определяющего число сдвигов в QR-алгоритме (аналог ширины блока
    в блочно-матричных алгоритмах линейной алгебры). В случае, если вы
    желаете добиться максимального быстродействия на вашей машине,
    рекомендуется ручная настройка этого параметра.
    Обычно оптимальное значение находится в диапазоне от 4 до 16.

Внутренние параметры:
    NS      -   число сдвигов в QR-алгоритме.
    MAXB    -   размер матрицы, при котором происходит переключение между
                алгоритмом с двойным сдвигом и алгоритмом с множественными
                сдвигами.

  -- LAPACK routine (version 3.0) --
     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
     Courant Institute, Argonne National Lab, and Rice University
     June 30, 1999
*************************************************************************/
void internalschurdecomposition(ap::real_2d_array& h,
     int n,
     int tneeded,
     int zneeded,
     ap::real_1d_array& wr,
     ap::real_1d_array& wi,
     ap::real_2d_array& z,
     int& info);

void internalschurdecomposition_c(ap::complex_2d_array& h,
								int n,
								int tneeded,
								int zneeded,
								ap::real_1d_array& wr,
								ap::real_1d_array& wi,
								ap::real_2d_array& z,
								int& info);

#endif
