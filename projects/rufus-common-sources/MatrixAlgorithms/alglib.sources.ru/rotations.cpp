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

//#include "pre.h"
#include "rotations.h"

/*************************************************************************
Применение последовательности элементарных вращений к матрице

Алгоритм  умножает  матрицу слева   на  последовательность  преобразований
вращения, заданную массивами C и S. В зависимости от  параметра  IsForward
вращению подвергаются последовательно либо 1 и 2, 3 и 4 и т.д. строки (если
IsForward=True), либо N и N-1-ая, N-2 и N-3 и т.д. строки.

Преобразованию подвергается не вся матрица а только её часть (строки от M1
до M2, столбцы от N1 до N2). Элементы, не попавшие в указанную подматрицу,
остаются без изменений.

Входные параметры:
    IsForward   -   последовательность применения вращений
    M1,M2       -   преобразованию подвергаются строки от M1 до M2
    N1,N2       -   преобразованию подвергаются столбцы от N1 до N2
    C,S         -   коэффициенты преобразования.
                    Массив с нумерацией элементов [1..M2-M1]
    A           -   обрабатываемая матрица
    WORK        -   рабочий массив с нумерацией элементов [N1..N2]

Выходные параметры
    A           -   преобразованная матрица

Служебная подпрограмма
*************************************************************************/
void applyrotationsfromtheleft(bool isforward,
     int m1,
     int m2,
     int n1,
     int n2,
     const ap::real_1d_array& c,
     const ap::real_1d_array& s,
     ap::real_2d_array& a,
     ap::real_1d_array& work)
{
    int j;
    int jp1;
    double ctemp;
    double stemp;
    double temp;

    if( m1>m2||n1>n2 )
    {
        return;
    }
    
    //
    // Form  P * A
    //
    if( isforward )
    {
        if( n1!=n2 )
        {
            
            //
            // Common case: N1<>N2
            //
            for(j = m1; j <= m2-1; j++)
            {
                ctemp = c(j-m1+1);
                stemp = s(j-m1+1);
                if( ctemp!=1||stemp!=0 )
                {
                    jp1 = j+1;
                    ap::vmove(&work(n1), &a(jp1, n1), ap::vlen(n1,n2), ctemp);
                    ap::vsub(&work(n1), &a(j, n1), ap::vlen(n1,n2), stemp);
                    ap::vmul(&a(j, n1), ap::vlen(n1,n2), ctemp);
                    ap::vadd(&a(j, n1), &a(jp1, n1), ap::vlen(n1,n2), stemp);
                    ap::vmove(&a(jp1, n1), &work(n1), ap::vlen(n1,n2));
                }
            }
        }
        else
        {
            
            //
            // Special case: N1=N2
            //
            for(j = m1; j <= m2-1; j++)
            {
                ctemp = c(j-m1+1);
                stemp = s(j-m1+1);
                if( ctemp!=1||stemp!=0 )
                {
                    temp = a(j+1,n1);
                    a(j+1,n1) = ctemp*temp-stemp*a(j,n1);
                    a(j,n1) = stemp*temp+ctemp*a(j,n1);
                }
            }
        }
    }
    else
    {
        if( n1!=n2 )
        {
            
            //
            // Common case: N1<>N2
            //
            for(j = m2-1; j >= m1; j--)
            {
                ctemp = c(j-m1+1);
                stemp = s(j-m1+1);
                if( ctemp!=1||stemp!=0 )
                {
                    jp1 = j+1;
                    ap::vmove(&work(n1), &a(jp1, n1), ap::vlen(n1,n2), ctemp);
                    ap::vsub(&work(n1), &a(j, n1), ap::vlen(n1,n2), stemp);
                    ap::vmul(&a(j, n1), ap::vlen(n1,n2), ctemp);
                    ap::vadd(&a(j, n1), &a(jp1, n1), ap::vlen(n1,n2), stemp);
                    ap::vmove(&a(jp1, n1), &work(n1), ap::vlen(n1,n2));
                }
            }
        }
        else
        {
            
            //
            // Special case: N1=N2
            //
            for(j = m2-1; j >= m1; j--)
            {
                ctemp = c(j-m1+1);
                stemp = s(j-m1+1);
                if( ctemp!=1||stemp!=0 )
                {
                    temp = a(j+1,n1);
                    a(j+1,n1) = ctemp*temp-stemp*a(j,n1);
                    a(j,n1) = stemp*temp+ctemp*a(j,n1);
                }
            }
        }
    }
}


/*************************************************************************
Применение последовательности элементарных вращений к матрице

Алгоритм  умножает  матрицу справа   на  последовательность  преобразований
вращения, заданную массивами C и S. В зависимости от  параметра   IsForward
вращению подвергаются последовательно либо 1 и 2,  3  и  4  и т.д.  столбцы
(если IsForward=True), либо N-ый и N-1-ый, N-2-ой и N-3-ий и т.д. столбцы.

Преобразованию подвергается не вся матрица а только её часть (строки от M1
до M2, столбцы от N1 до N2). Элементы, не попавшие в указанную подматрицу,
остаются без изменений.

Входные параметры:
    IsForward   -   последовательность применения вращений
    M1,M2       -   преобразованию подвергаются строки от M1 до M2
    N1,N2       -   преобразованию подвергаются столбцы от N1 до N2
    C,S         -   коэффициенты преобразования.
                    Массив с нумерацией элементов [1..N2-N1]
    A           -   обрабатываемая матрица
    WORK        -   рабочий массив с нумерацией элементов [M1..M2]

Выходные параметры
    A           -   преобразованная матрица

Служебная подпрограмма
*************************************************************************/
void applyrotationsfromtheright(bool isforward,
     int m1,
     int m2,
     int n1,
     int n2,
     const ap::real_1d_array& c,
     const ap::real_1d_array& s,
     ap::real_2d_array& a,
     ap::real_1d_array& work)
{
    int j;
    int jp1;
    double ctemp;
    double stemp;
    double temp;

    
    //
    // Form A * P'
    //
    if( isforward )
    {
        if( m1!=m2 )
        {
            
            //
            // Common case: M1<>M2
            //
            for(j = n1; j <= n2-1; j++)
            {
                ctemp = c(j-n1+1);
                stemp = s(j-n1+1);
                if( ctemp!=1||stemp!=0 )
                {
                    jp1 = j+1;
                    ap::vmove(work.getvector(m1, m2), a.getcolumn(jp1, m1, m2), ctemp);
                    ap::vsub(work.getvector(m1, m2), a.getcolumn(j, m1, m2), stemp);
                    ap::vmul(a.getcolumn(j, m1, m2), ctemp);
                    ap::vadd(a.getcolumn(j, m1, m2), a.getcolumn(jp1, m1, m2), stemp);
                    ap::vmove(a.getcolumn(jp1, m1, m2), work.getvector(m1, m2));
                }
            }
        }
        else
        {
            
            //
            // Special case: M1=M2
            //
            for(j = n1; j <= n2-1; j++)
            {
                ctemp = c(j-n1+1);
                stemp = s(j-n1+1);
                if( ctemp!=1||stemp!=0 )
                {
                    temp = a(m1,j+1);
                    a(m1,j+1) = ctemp*temp-stemp*a(m1,j);
                    a(m1,j) = stemp*temp+ctemp*a(m1,j);
                }
            }
        }
    }
    else
    {
        if( m1!=m2 )
        {
            
            //
            // Common case: M1<>M2
            //
            for(j = n2-1; j >= n1; j--)
            {
                ctemp = c(j-n1+1);
                stemp = s(j-n1+1);
                if( ctemp!=1||stemp!=0 )
                {
                    jp1 = j+1;
                    ap::vmove(work.getvector(m1, m2), a.getcolumn(jp1, m1, m2), ctemp);
                    ap::vsub(work.getvector(m1, m2), a.getcolumn(j, m1, m2), stemp);
                    ap::vmul(a.getcolumn(j, m1, m2), ctemp);
                    ap::vadd(a.getcolumn(j, m1, m2), a.getcolumn(jp1, m1, m2), stemp);
                    ap::vmove(a.getcolumn(jp1, m1, m2), work.getvector(m1, m2));
                }
            }
        }
        else
        {
            
            //
            // Special case: M1=M2
            //
            for(j = n2-1; j >= n1; j--)
            {
                ctemp = c(j-n1+1);
                stemp = s(j-n1+1);
                if( ctemp!=1||stemp!=0 )
                {
                    temp = a(m1,j+1);
                    a(m1,j+1) = ctemp*temp-stemp*a(m1,j);
                    a(m1,j) = stemp*temp+ctemp*a(m1,j);
                }
            }
        }
    }
}


/*************************************************************************
Подпрограмма генерирует элементарное вращение, такое, что

[  CS  SN  ]  .  [ F ]  =  [ R ]
[ -SN  CS  ]     [ G ]     [ 0 ]

CS**2 + SN**2 = 1
*************************************************************************/
void generaterotation(double f, double g, double& cs, double& sn, double& r)
{
    double f1;
    double g1;

    if( g==0 )
    {
        cs = 1;
        sn = 0;
        r = f;
    }
    else
    {
        if( f==0 )
        {
            cs = 0;
            sn = 1;
            r = g;
        }
        else
        {
            f1 = f;
            g1 = g;
            r = sqrt(ap::sqr(f1)+ap::sqr(g1));
            cs = f1/r;
            sn = g1/r;
            if( ::fabs(f)> ::fabs(g)&&cs<0 )
            {
                cs = -cs;
                sn = -sn;
                r = -r;
            }
        }
    }
}



