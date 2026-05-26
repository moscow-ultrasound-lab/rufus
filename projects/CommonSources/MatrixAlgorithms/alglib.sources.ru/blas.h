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

#ifndef _blas_h
#define _blas_h

#include "libs/ap.h"

/*************************************************************************
Евклидова норма вектора, заданного компонентами с I1-ой по I2-ую массива X
"Безопасный" алгоритм, гарантированно не приводящий к overflow или underflow.

 -- BLAS
    This version written on 25-October-1982.
    Modified on 14-October-1993 to inline the call to DLASSQ.
    Sven Hammarling, Nag Ltd.
*************************************************************************/
double vectornorm2(const ap::real_1d_array& x, int i1, int i2);


/*************************************************************************
Индекс элемента с максимальным модулем, расположенного в массиве X с I1-ой
по I2-ую позиции
*************************************************************************/
int vectoridxabsmax(const ap::real_1d_array& x, int i1, int i2);
int vectoridxabsmax_c(const ap::complex_1d_array& x, int i1, int i2);

/*************************************************************************
Индекс элемента с максимальным модулем, расположенного в матрице X с I1-ой
по I2-ую строки J-ого столбца
*************************************************************************/
int columnidxabsmax(const ap::real_2d_array& x, int i1, int i2, int j);


/*************************************************************************
Индекс элемента с максимальным модулем, расположенного в массиве X с J1-ого
по J2-й столбец I-ой строки
*************************************************************************/
int rowidxabsmax(const ap::real_2d_array& x, int j1, int j2, int i);


/*************************************************************************
1-норма матрицы Хессенберга

Параметры:
    A       -   матрица
    I1, I2  -   диапазон строк, в которых находится подматрица
    J1, J2  -   диапазон столбцов, в которых находится подматрица
    WORK    -   рабочий массив, нумерация элементов как минимум от J1 до J2
*************************************************************************/
double upperhessenberg1norm(const ap::real_2d_array& a,
     int i1,
     int i2,
     int j1,
     int j2,
     ap::real_1d_array& work);

double upperhessenberg1norm_c(const ap::complex_2d_array& a,
							int i1,
							int i2,
							int j1,
							int j2,
							ap::real_1d_array& work);


/*************************************************************************
копирование матрицы

Параметры:
    A           -   матрица-источник
    IS1, IS2    -   диапазон строк, в которых находится подматрица-источник
    JS1, JS2    -   диапазон столбцов, в которых находится подматрица-источник
    B           -   матрица-приемник
    ID1, ID2    -   диапазон строк, в которых находится подматрица-приемник
    JD1, JD2    -   диапазон столбцов, в которых находится подматрица-приемник
*************************************************************************/
void copymatrix(const ap::real_2d_array& a,
     int is1,
     int is2,
     int js1,
     int js2,
     ap::real_2d_array& b,
     int id1,
     int id2,
     int jd1,
     int jd2);

void copymatrix_c(const ap::complex_2d_array& a,
				int is1,
				int is2,
				int js1,
				int js2,
				ap::complex_2d_array& b,
				int id1,
				int id2,
				int jd1,
				int jd2);


/*************************************************************************
Подпрограмма транспонирования "на месте"

Транспонирует квадратную подматрицу A[I1:I2,J1:J2] с использованием
рабочего массива WORK с нумерацией элементов как минимум от 1 до I2-I1.
*************************************************************************/
void inplacetranspose(ap::real_2d_array& a,
     int i1,
     int i2,
     int j1,
     int j2,
     ap::real_1d_array& work);


/*************************************************************************
копирование матрицы с транспонированием

Параметры:
    A           -   матрица-источник
    IS1, IS2    -   диапазон строк, в которых находится подматрица-источник
    JS1, JS2    -   диапазон столбцов, в которых находится подматрица-источник
    B           -   матрица-приемник
    ID1, ID2    -   диапазон строк, в которых находится подматрица-приемник
    JD1, JD2    -   диапазон столбцов, в которых находится подматрица-приемник
*************************************************************************/
void copyandtranspose(const ap::real_2d_array& a,
     int is1,
     int is2,
     int js1,
     int js2,
     ap::real_2d_array& b,
     int id1,
     int id2,
     int jd1,
     int jd2);


/*************************************************************************
Операция y := alpha*A*x + beta*y или y := alpha*A'*x + beta*y

Аналогично DGEMV из стандартной BLAS
*************************************************************************/
void matrixvectormultiply(const ap::real_2d_array& a,
     int i1,
     int i2,
     int j1,
     int j2,
     bool trans,
     const ap::real_1d_array& x,
     int ix1,
     int ix2,
     double alpha,
     ap::real_1d_array& y,
     int iy1,
     int iy2,
     double beta);

void matrixvectormultiply_c(const ap::complex_2d_array& a,
						  int i1,
						  int i2,
						  int j1,
						  int j2,
						  bool trans,
						  const ap::complex_1d_array& x,
						  int ix1,
						  int ix2,
						  double alpha,
						  ap::complex_1d_array& y,
						  int iy1,
						  int iy2,
						  double beta);


double pythag2(double x, double y);


/*************************************************************************
Произведение матриц: C = alpha*op1(A)*op2(B) + beta*C, где op(X)  равно  X
или равно X^T, в зависимости от параметров, переданных подпрограмме.

Матрица A задается частью массива A[AI1..AI2,AJ1..AJ2]. Аналогично, матрица
B задается частью массива B[BI1..BI2,BJ1..BJ2]. Результат произведения
помещается в массив C[CI1..CI2,CJ1..CJ2].

Также подпрограмма принимает рабочий массив WORK с нумерацией элементов от
1 до Max(ColumnsCount(А), ColumnsCount(B), RowsCount(A), RowsCount(B)).
*************************************************************************/
void matrixmatrixmultiply(const ap::real_2d_array& a,
     int ai1,
     int ai2,
     int aj1,
     int aj2,
     bool transa,
     const ap::real_2d_array& b,
     int bi1,
     int bi2,
     int bj1,
     int bj2,
     bool transb,
     double alpha,
     ap::real_2d_array& c,
     int ci1,
     int ci2,
     int cj1,
     int cj2,
     double beta,
     ap::real_1d_array& work);


#endif
