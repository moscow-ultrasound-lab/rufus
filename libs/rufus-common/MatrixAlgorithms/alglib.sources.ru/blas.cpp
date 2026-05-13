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

// #include "pre.h"
#include "blas.h"

/*************************************************************************
Евклидова норма вектора, заданного компонентами с I1-ой по I2-ую массива X
"Безопасный" алгоритм, гарантированно не приводящий к overflow или underflow.

 -- BLAS
    This version written on 25-October-1982.
    Modified on 14-October-1993 to inline the call to DLASSQ.
    Sven Hammarling, Nag Ltd.
*************************************************************************/
double vectornorm2(const ap::real_1d_array& x, int i1, int i2)
{
    double result;
    int n;
    int ix;
    double absxi;
    double scl;
    double ssq;

    n = i2-i1+1;
    if( n<1 )
    {
        result = 0;
        return result;
    }
    if( n==1 )
    {
        result = ::fabs(x(i1));
        return result;
    }
    scl = 0;
    ssq = 1;
    for(ix = i1; ix <= i2; ix++)
    {
        if( x(ix)!=0 )
        {
            absxi = ::fabs(x(ix));
            if( scl<absxi )
            {
                ssq = 1+ssq*ap::sqr(scl/absxi);
                scl = absxi;
            }
            else
            {
                ssq = ssq+ap::sqr(absxi/scl);
            }
        }
    }
    result = scl*sqrt(ssq);
    return result;
}


/*************************************************************************
Индекс элемента с максимальным модулем, расположенного в массиве X с I1-ой
по I2-ую позиции
*************************************************************************/
int vectoridxabsmax(const ap::real_1d_array& x, int i1, int i2)
{
    int result;
    int i;
    double a;

    result = i1;
    a = ::fabs(x(result));
    for(i = i1+1; i <= i2; i++)
    {
        if( ::fabs(x(i))>::fabs(x(result)) )
        {
            result = i;
        }
    }
    return result;
}

#if 0
int vectoridxabsmax_c(const ap::complex_1d_array& x, int i1, int i2)
{
	int result;
	int i;
	double a;

	result = i1;
	a = ::cabs(x(result));
	for(i = i1+1; i <= i2; i++)
	{
		if( ::cabs(x(i))>::cabs(x(result)) )
		{
			result = i;
		}
	}
	return result;
}
#endif

/*************************************************************************
Индекс элемента с максимальным модулем, расположенного в матрице X с I1-ой
по I2-ую строки J-ого столбца
*************************************************************************/
int columnidxabsmax(const ap::real_2d_array& x, int i1, int i2, int j)
{
    int result;
    int i;
    double a;

    result = i1;
    a = ::fabs(x(result,j));
    for(i = i1+1; i <= i2; i++)
    {
        if( ::fabs(x(i,j))>::fabs(x(result,j)) )
        {
            result = i;
        }
    }
    return result;
}


/*************************************************************************
Индекс элемента с максимальным модулем, расположенного в массиве X с J1-ого
по J2-й столбец I-ой строки
*************************************************************************/
int rowidxabsmax(const ap::real_2d_array& x, int j1, int j2, int i)
{
    int result;
    int j;
    double a;

    result = j1;
    a = ::fabs(x(i,result));
    for(j = j1+1; j <= j2; j++)
    {
        if( ::fabs(x(i,j))>::fabs(x(i,result)) )
        {
            result = j;
        }
    }
    return result;
}


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
     ap::real_1d_array& work)
{
    double result;
    int i;
    int j;

    ap::ap_error::make_assertion(i2-i1==j2-j1, "UpperHessenberg1Norm: I2-I1<>J2-J1!");
    for(j = j1; j <= j2; j++)
    {
        work(j) = 0;
    }
    for(i = i1; i <= i2; i++)
    {
        for(j = ap::maxint(j1, j1+i-i1-1); j <= j2; j++)
        {
            work(j) = work(j)+::fabs(a(i,j));
        }
    }
    result = 0;
    for(j = j1; j <= j2; j++)
    {
        result = ap::maxreal(result, work(j));
    }
    return result;
}

#if 0

double upperhessenberg1norm_c(const ap::complex_2d_array& in_a,
							int i1,
							int i2,
							int j1,
							int j2,
							ap::real_1d_array& work)
{
// 	int n(5);
// 	ap::real_2d_array a;
// 	a.setbounds(1,n,1,n);
// 	for(int i = 0; i < n; i++)
// 	{
// 		for(int j = 0; j < n; j++)
// 		{
// 			a(i+1,j+1) = in_a(i+1,j+1).re;
// 		}
// 	}


	double result;
	int i;
	int j;

	ap::ap_error::make_assertion(i2-i1==j2-j1, "UpperHessenberg1Norm: I2-I1<>J2-J1!");
	for(j = j1; j <= j2; j++)
	{
		work(j) = 0;
	}
	for(i = i1; i <= i2; i++)
	{
		for(j = ap::maxint(j1, j1+i-i1-1); j <= j2; j++)
		{
			work(j) = work(j)+::cabs(in_a(i,j));
		}
	}
	result = 0;
	for(j = j1; j <= j2; j++)
	{
		result = ap::maxreal(result, work(j));
	}
	return result;
}
#endif


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
     int jd2)
{
    int isrc;
    int idst;

    if( is1>is2||js1>js2 )
    {
        return;
    }
    ap::ap_error::make_assertion(is2-is1==id2-id1, "CopyMatrix: different sizes!");
    ap::ap_error::make_assertion(js2-js1==jd2-jd1, "CopyMatrix: different sizes!");
    for(isrc = is1; isrc <= is2; isrc++)
    {
        idst = isrc-is1+id1;
        ap::vmove(&b(idst, jd1), &a(isrc, js1), ap::vlen(jd1,jd2));
    }
}

void copymatrix_c(const ap::complex_2d_array& a,
				int is1,
				int is2,
				int js1,
				int js2,
				ap::complex_2d_array& b,
				int id1,
				int id2,
				int jd1,
				int jd2)
{
	int isrc;
	int idst;

	if( is1>is2||js1>js2 )
	{
		return;
	}
	ap::ap_error::make_assertion(is2-is1==id2-id1, "CopyMatrix: different sizes!");
	ap::ap_error::make_assertion(js2-js1==jd2-jd1, "CopyMatrix: different sizes!");
	for(isrc = is1; isrc <= is2; isrc++)
	{
		idst = isrc-is1+id1;
		ap::vmove(&b(idst, jd1), &a(isrc, js1), ap::vlen(jd1,jd2));
	}
}



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
     ap::real_1d_array& work)
{
    int i;
    int j;
    int ips;
    int jps;
    int l;

    if( i1>i2||j1>j2 )
    {
        return;
    }
    ap::ap_error::make_assertion(i1-i2==j1-j2, "InplaceTranspose error: incorrect array size!");
    for(i = i1; i <= i2-1; i++)
    {
        j = j1+i-i1;
        ips = i+1;
        jps = j1+ips-i1;
        l = i2-i;
        ap::vmove(work.getvector(1, l), a.getcolumn(j, ips, i2));
        ap::vmove(a.getcolumn(j, ips, i2), a.getrow(i, jps, j2));
        ap::vmove(&a(i, jps), &work(1), ap::vlen(jps,j2));
    }
}


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
     int jd2)
{
    int isrc;
    int jdst;

    if( is1>is2||js1>js2 )
    {
        return;
    }
    ap::ap_error::make_assertion(is2-is1==jd2-jd1, "CopyAndTranspose: different sizes!");
    ap::ap_error::make_assertion(js2-js1==id2-id1, "CopyAndTranspose: different sizes!");
    for(isrc = is1; isrc <= is2; isrc++)
    {
        jdst = isrc-is1+jd1;
        ap::vmove(b.getcolumn(jdst, id1, id2), a.getrow(isrc, js1, js2));
    }
}


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
     double beta)
{
    int i;
    double v;

    if( !trans )
    {
        
        //
        // y := alpha*A*x + beta*y;
        //
        if( i1>i2||j1>j2 )
        {
            return;
        }
        ap::ap_error::make_assertion(j2-j1==ix2-ix1, "MatrixVectorMultiply: A and X dont match!");
        ap::ap_error::make_assertion(i2-i1==iy2-iy1, "MatrixVectorMultiply: A and Y dont match!");
        
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
            ap::vmul(&y(iy1), ap::vlen(iy1,iy2), beta);
        }
        
        //
        // alpha*A*x
        //
        for(i = i1; i <= i2; i++)
        {
            v = ap::vdotproduct(&a(i, j1), &x(ix1), ap::vlen(j1,j2));
            y(iy1+i-i1) = y(iy1+i-i1)+alpha*v;
        }
    }
    else
    {
        
        //
        // y := alpha*A'*x + beta*y;
        //
        if( i1>i2||j1>j2 )
        {
            return;
        }
        ap::ap_error::make_assertion(i2-i1==ix2-ix1, "MatrixVectorMultiply: A and X dont match!");
        ap::ap_error::make_assertion(j2-j1==iy2-iy1, "MatrixVectorMultiply: A and Y dont match!");
        
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
            ap::vmul(&y(iy1), ap::vlen(iy1,iy2), beta);
        }
        
        //
        // alpha*A'*x
        //
        for(i = i1; i <= i2; i++)
        {
            v = alpha*x(ix1+i-i1);
            ap::vadd(&y(iy1), &a(i, j1), ap::vlen(iy1,iy2), v);
        }
    }
}

#if 0

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
						  double beta)
{
	int i;
	double v;

	if( !trans )
	{

		//
		// y := alpha*A*x + beta*y;
		//
		if( i1>i2||j1>j2 )
		{
			return;
		}
		ap::ap_error::make_assertion(j2-j1==ix2-ix1, "MatrixVectorMultiply: A and X dont match!");
		ap::ap_error::make_assertion(i2-i1==iy2-iy1, "MatrixVectorMultiply: A and Y dont match!");

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
			ap::vmul(&y(iy1), ap::vlen(iy1,iy2), beta);
		}

		//
		// alpha*A*x
		//
		for(i = i1; i <= i2; i++)
		{
			v = ap::vdotproduct(&a(i, j1), &x(ix1), ap::vlen(j1,j2));
			y(iy1+i-i1) = y(iy1+i-i1)+alpha*v;
		}
	}
	else
	{

		//
		// y := alpha*A'*x + beta*y;
		//
		if( i1>i2||j1>j2 )
		{
			return;
		}
		ap::ap_error::make_assertion(i2-i1==ix2-ix1, "MatrixVectorMultiply: A and X dont match!");
		ap::ap_error::make_assertion(j2-j1==iy2-iy1, "MatrixVectorMultiply: A and Y dont match!");

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
			ap::vmul(&y(iy1), ap::vlen(iy1,iy2), beta);
		}

		//
		// alpha*A'*x
		//
		for(i = i1; i <= i2; i++)
		{
			v = alpha*x(ix1+i-i1);
			ap::vadd(&y(iy1), &a(i, j1), ap::vlen(iy1,iy2), v);
		}
	}
}

#endif

double pythag2(double x, double y)
{
    double result;
    double w;
    double xabs;
    double yabs;
    double z;

    xabs = ::fabs(x);
    yabs = ::fabs(y);
    w = ap::maxreal(xabs, yabs);
    z = ap::minreal(xabs, yabs);
    if( z==0 )
    {
        result = w;
    }
    else
    {
        result = w*sqrt(1+ap::sqr(z/w));
    }
    return result;
}


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
     ap::real_1d_array& work)
{
    int arows;
    int acols;
    int brows;
    int bcols;
    int crows;
    int ccols;
    int i;
    int j;
    int k(0);
    int l;
    int r;
    double v;

    
    //
    // Setup
    //
    if( !transa )
    {
        arows = ai2-ai1+1;
        acols = aj2-aj1+1;
    }
    else
    {
        arows = aj2-aj1+1;
        acols = ai2-ai1+1;
    }
    if( !transb )
    {
        brows = bi2-bi1+1;
        bcols = bj2-bj1+1;
    }
    else
    {
        brows = bj2-bj1+1;
        bcols = bi2-bi1+1;
    }
    ap::ap_error::make_assertion(acols==brows, "MatrixMatrixMultiply: incorrect matrix sizes!");
    if( arows<=0||acols<=0||brows<=0||bcols<=0 )
    {
        return;
    }
    crows = arows;
    ccols = bcols;
    
    //
    // Test WORK
    //
    i = ap::maxint(arows, acols);
    i = ap::maxint(brows, i);
    i = ap::maxint(i, bcols);
    work(1) = 0;
    work(i) = 0;
    
    //
    // Prepare C
    //
    if( beta==0 )
    {
        for(i = ci1; i <= ci2; i++)
        {
            for(j = cj1; j <= cj2; j++)
            {
                c(i,j) = 0;
            }
        }
    }
    else
    {
        for(i = ci1; i <= ci2; i++)
        {
            ap::vmul(&c(i, cj1), ap::vlen(cj1,cj2), beta);
        }
    }
    
    //
    // A*B
    //
    if( !transa&&!transb )
    {
        for(l = ai1; l <= ai2; l++)
        {
            for(r = bi1; r <= bi2; r++)
            {
                v = alpha*a(l,aj1+r-bi1);
                k = ci1+l-ai1;
                ap::vadd(&c(k, cj1), &b(r, bj1), ap::vlen(cj1,cj2), v);
            }
        }
        return;
    }
    
    //
    // A*B'
    //
    if( !transa&&transb )
    {
        if( arows*acols<brows*bcols )
        {
            for(r = bi1; r <= bi2; r++)
            {
                for(l = ai1; l <= ai2; l++)
                {
                    v = ap::vdotproduct(&a(l, aj1), &b(r, bj1), ap::vlen(aj1,aj2));
                    c(ci1+l-ai1,cj1+r-bi1) = c(ci1+l-ai1,cj1+r-bi1)+alpha*v;
                }
            }
            return;
        }
        else
        {
            for(l = ai1; l <= ai2; l++)
            {
                for(r = bi1; r <= bi2; r++)
                {
                    v = ap::vdotproduct(&a(l, aj1), &b(r, bj1), ap::vlen(aj1,aj2));
                    c(ci1+l-ai1,cj1+r-bi1) = c(ci1+l-ai1,cj1+r-bi1)+alpha*v;
                }
            }
            return;
        }
    }
    
    //
    // A'*B
    //
    if( transa&&!transb )
    {
        for(l = aj1; l <= aj2; l++)
        {
            for(r = bi1; r <= bi2; r++)
            {
                v = alpha*a(ai1+r-bi1,l);
                k = ci1+l-aj1;
                ap::vadd(&c(k, cj1), &b(r, bj1), ap::vlen(cj1,cj2), v);
            }
        }
        return;
    }
    
    //
    // A'*B'
    //
    if( transa&&transb )
    {
        if( arows*acols<brows*bcols )
        {
            for(r = bi1; r <= bi2; r++)
            {
                for(i = 1; i <= crows; i++)
                {
                    work(i) = 0.0;
                }
                for(l = ai1; l <= ai2; l++)
                {
                    v = alpha*b(r,bj1+l-ai1);
                    k = cj1+r-bi1;
                    ap::vadd(&work(1), &a(l, aj1), ap::vlen(1,crows), v);
                }
                ap::vadd(c.getcolumn(k, ci1, ci2), work.getvector(1, crows));
            }
            return;
        }
        else
        {
            for(l = aj1; l <= aj2; l++)
            {
                k = ai2-ai1+1;
                ap::vmove(work.getvector(1, k), a.getcolumn(l, ai1, ai2));
                for(r = bi1; r <= bi2; r++)
                {
                    v = ap::vdotproduct(&work(1), &b(r, bj1), ap::vlen(1,k));
                    c(ci1+l-aj1,cj1+r-bi1) = c(ci1+l-aj1,cj1+r-bi1)+alpha*v;
                }
            }
            return;
        }
    }
}



