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

#ifndef _creflections_h
#define _creflections_h

#include "libs/ap.h"

/*************************************************************************
Генерация элементарного комплексного преобразования отражения

Подпрограмма  генерирует  элементарное  комплексное отражение H порядка N,
такое, что для заданного X выполняется следующее равенство:

     ( X(1) )   ( Beta )
H' * (  ..  ) = (  0   ),   H'*H = I,   Beta - вещественное число
     ( X(n) )   (  0   )
    
где

              ( V(1) )
H = 1 - Tau * (  ..  ) * ( conj(V(1)), ..., conj(V(n)) )
              ( V(n) )

причем первая компонента вектора V равна единице.
              
Входные параметры:
    X       -   вектор. Массив с нумерацией элементов [1..N]
    N       -   порядок отражения
    
Выходные параметры:
    X       -   компоненты с 2 по N замещается вектором V. Первая
                компонента замещается параметром Beta.
    Tau     -   скалярная величина Tau.

Данная подпрограмма является модификацией подпрограмм CLARFG из библиотеки
LAPACK. Функциональность аналогичная, но отсутствует  корректная обработка
случаев, когда промежуточные результаты вычислений  переполняют  разрядную
сетку.

ИЗМЕНЕНИЯ И ИСПРАВЛЕНИЯ:
    24.12.2005 замена sign(Alpha) на аналогичный sign в фортране код

  -- LAPACK auxiliary routine (version 3.0) --
     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
     Courant Institute, Argonne National Lab, and Rice University
     September 30, 1994
*************************************************************************/
void complexgeneratereflection(ap::complex_1d_array& x,
     int n,
     ap::complex& tau);


/*************************************************************************
Применение элементарного отражения к прямоугольной матрице размером MxN

Алгоритм умножает слева матрицу на элементарное преобразование  отражения,
заданное    столбцом   V   и   скалярной   величиной   Tau  (см.  описание
GenerateReflection). Преобразованию подвергается не вся матрица, а  только
её часть (строки от M1 до M2, столбцы от N1 до N2). Элементы, не  попавшие
в указанную подматрицу, остаются без изменений.

Замечание: матрица умножается на матрицу H, а не  на  H'.  Если  требуется
умножить на матрицу H', вместо параметра Tau требуется передать Conj(Tau).

Входные параметры:
    C   -   матрица,  к  которой  применяется  преобразование.
    Tau -   скаляр, задающий преобразование.
    V   -   столбец, задающий преобразование. Массив с нумерацией элементов
            [1..M2-M1+1]
    M1,M2   -   диапазон строк, затрагиваемых преобразованием
    N1,N2   -   диапазон столбцов, затрагиваемых преобразованием
    WORK    -   рабочий массив с нумерацией элементов от N1 до N2

Выходные параметры:
    C   -   результат умножения входной матрицы C на матрицу преобразования,
            заданного Tau и V. Если N1>N2 или M1>M2, то C не меняется.

  -- LAPACK auxiliary routine (version 3.0) --
     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
     Courant Institute, Argonne National Lab, and Rice University
     September 30, 1994
*************************************************************************/
void complexapplyreflectionfromtheleft(ap::complex_2d_array& c,
     ap::complex tau,
     const ap::complex_1d_array& v,
     int m1,
     int m2,
     int n1,
     int n2,
     ap::complex_1d_array& work);


/*************************************************************************
Применение элементарного отражения к прямоугольной матрице размером MxN

Алгоритм умножает справа матрицу на элементарное преобразование отражения,
заданное    столбцом   V   и   скалярной   величиной   Tau  (см.  описание
GenerateReflection). Преобразованию подвергается не вся матрица, а  только
её часть (строки от M1 до M2, столбцы от N1 до N2). Элементы, не  попавшие
в указанную подматрицу, остаются без изменений.

Входные параметры:
    C   -   матрица,  к  которой  применяется  преобразование.
    Tau -   скаляр, задающий преобразование.
    V   -   столбец, задающий преобразование. Массив с нумерацией элементов
            [1..N2-N1+1]
    M1,M2   -   диапазон строк, затрагиваемых преобразованием
    N1,N2   -   диапазон столбцов, затрагиваемых преобразованием
    WORK    -   рабочий массив с нумерацией элементов от M1 до M2

Выходные параметры:
    C   -   результат умножения входной матрицы C на матрицу преобразования,
            заданного Tau и V. Если N1>N2 или M1>M2, то C не меняется.

  -- LAPACK auxiliary routine (version 3.0) --
     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
     Courant Institute, Argonne National Lab, and Rice University
     September 30, 1994
*************************************************************************/
void complexapplyreflectionfromtheright(ap::complex_2d_array& c,
     ap::complex tau,
     ap::complex_1d_array& v,
     int m1,
     int m2,
     int n1,
     int n2,
     ap::complex_1d_array& work);


#endif
