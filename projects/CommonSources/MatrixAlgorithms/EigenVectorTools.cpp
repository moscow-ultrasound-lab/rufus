#include "pre.h"
#include "EigenVectorTools.h"
//#include <XRADGUI.h>

#include <stdio.h>


	#include "alglib.sources.ru/libs/ap.h"

	#include "alglib.sources.ru/blas.h"
	#include "alglib.sources.ru/rotations.h"
	#include "alglib.sources.ru/tdevd.h"
	#include "alglib.sources.ru/cblas.h"
	#include "alglib.sources.ru/creflections.h"
	#include "alglib.sources.ru/hblas.h"
	#include "alglib.sources.ru/htridiagonal.h"

	#include "alglib.sources.ru/hevd.h"
	#include "alglib.sources.ru/nsevd.h"


XRAD_BEGIN

//	вызовы алгоритмов для данных в формате XRAD

bool	eigenvectors_non_symmetrical(const RealMatrixF32 &m, ComplexMatrixF32 &v_left, ComplexMatrixF32 &v_right, ComplexVectorF32 &eig_val)
{
	size_t	n = m.vsize();
	if(m.hsize() != n)
	{
//		Error("Eigenvectors : Matrix must be square");
		return false;
	}
	v_right.realloc(n,n);
	eig_val.realloc(n);
	v_left.realloc(n,n);

	ap::real_2d_array a;

	ap::real_1d_array	wr;
	ap::real_1d_array	wi;
	ap::real_2d_array	vl;
	ap::real_2d_array	vr;

	a.setbounds(0,int(n-1),0,int(n-1));
	
	for(size_t i = 0; i < n; i++)
	{
		for(size_t j = 0; j < n; j++)
		{
			a(int(i),int(j)) = m.at(i,j);
		}
	}	
	bool	wsucc = rmatrixevd(a, int(n),
		     3, // both right and left eigen vectors needed
		     wr, wi,
		     vl, vr);
	
	if(wsucc)
	{
		for(size_t i = 0; i < n; i++) 
		{
			eig_val[i].re = wr(int(i));
		}
		for(size_t i = 0; i < n; i++) 
		{
			if(wi(int(i))==0)
			{
				for(size_t j = 0; j < n; j++)
				{
					v_right.at(j,i).re = vr(int(j),int(i));
					v_left.at(j,i).re = vl(int(j),int(i));
				}
			}
		}
//TODO может быть, как-то понадежнее переписать
		{
			size_t	i = 0;
			while(i < n)
			{
				if(wi(int(i))>0)
				{
					eig_val[i].im = wi(int(i));
					eig_val[i+1].im = -wi(int(i));
					for(size_t j = 0; j < n; j++)
					{
						v_right.at(j, i).re = vr(int(j), int(i));
						v_right.at(j, i).im = vr(int(j), int(i+1));
						v_right.at(j, i+1).re = vr(int(j), int(i));
						v_right.at(j, i+1).im = -vr(int(j), int(i+1));
						v_left.at(j, i).re = vl(int(j), int(i));
						v_left.at(j, i).im = vl(int(j), int(i+1));
						v_left.at(j, i+1).re = vl(int(j), int(i));
						v_left.at(j, i+1).im = -vl(int(j), int(i+1));
// 						v_right.at(j,i).re = vr(j,i);
// 						v_right.at(j,i).im = vr(j,i+1);
// 						v_right.at(j,i+1).re = vr(j,i);
// 						v_right.at(j,i+1).im = -vr(j,i+1);
// 						v_left.at(j,i).re = vl(j,i);
// 						v_left.at(j,i).im = vl(j,i+1);
// 						v_left.at(j,i+1).re = vl(j,i);
// 						v_left.at(j,i+1).im = -vl(j,i+1);
					}
					i+=2;
				}
				else
				{
					++i;
				}
			}
		}
	}
	return wsucc;
}


bool	eigenvectors_hermit(const ComplexMatrixF32 &m, ComplexMatrixF32 &vec, RealVectorF32 &eig_val)
{
	size_t	n = m.vsize();
	if(m.hsize() != n)
	{
//		Error("Eigenvectors : Matrix must be square");
		return false;
	}
	eig_val.realloc(n);
	vec.realloc(n,n);

	ap::complex_2d_array a;

	ap::real_1d_array w;
	ap::complex_2d_array v;

	a.setbounds(0,int(n-1),0,int(n-1));

	for(size_t i = 0; i < n; i++)
	{
		for(size_t j = 0; j < n; j++)
		{
			a(int(i),int(j)).re = m.at(i,j).re;
			a(int(i),int(j)).im = m.at(i,j).im;
		}
	}

	bool	wsucc = hmatrixevd(a, int(n), 1, true, w, v);

	if(wsucc)
	{
		for(size_t i = 0; i < n; i++)
		{
			eig_val[i] = w(int(i));
			for(size_t j = 0; j < n; j++)
			{
				vec.at(i,j).re = v(int(i),int(j)).re;
 				vec.at(i,j).im = v(int(i),int(j)).im;
			}
		}
	}

	return wsucc;
}


bool	eigenvectors_non_symmetrical_c(const ComplexMatrixF32 &m, ComplexMatrixF32 &v_left, ComplexMatrixF32 &v_right, ComplexVectorF32 &eig_val)
{
	size_t	n = m.vsize();
	if(m.hsize() != n)
	{
//		Error("Eigenvectors : Matrix must be square");
		return false;
	}
	v_right.realloc(n,n);
	eig_val.realloc(n);
	v_left.realloc(n,n);

	//ap::real_2d_array a;
	ap::complex_2d_array a;

	ap::real_1d_array	wr;
	ap::real_1d_array	wi;
	ap::real_2d_array	vl;
	ap::real_2d_array	vr;
	int vneeded(0);// both right and left eigen vectors needed

	a.setbounds(0,int(n-1),0,int(n-1));

	for(size_t i = 0; i < n; i++)
	{
		for(size_t j = 0; j < n; j++)
		{
			a(int(i),int(j)).re = m.at(i,j).re;
 			a(int(i),int(j)).im = m.at(i,j).im;
		}
	}	
	bool	wsucc = rmatrixevd_c(a, int(n),
		vneeded, 
		wr, wi,
		vl, vr);

	if(wsucc)
	{
		for(size_t i = 0; i < n; i++) 
		{
			eig_val[i].re = wr(int(i));
		}
		for(size_t i = 0; i < n; i++) 
		{
			if(wi(int(i))==0)
			{
				for(size_t j = 0; j < n; j++)
				{
					if((vneeded==1)||(vneeded==3)) 
					{
						v_right.at(j,i).re = vr(int(j),int(i));
					}
					if((vneeded==2)||(vneeded==3)) 
					{
						v_left.at(j,i).re = vl(int(j),int(i));
					}
				}
			}
		}
		//TODO может быть, как-то понадежнее переписать
		{
			size_t	i = 0;
			while(i < n)
			{
				if(wi(int(i))>0)
				{
					eig_val[i].im = wi(int(i));
					eig_val[i+1].im = -wi(int(i));
					for(size_t j = 0; j < n; j++)
					{
						if((vneeded==1)||(vneeded==3)) 
						{
							v_right.at(j,i).re = vr(int(j),int(i));
							v_right.at(j,i).im = vr(int(j),int(i+1));
							v_right.at(j,i+1).re = vr(int(j),int(i));
							v_right.at(j,i+1).im = -vr(int(j),int(i+1));
						}
						if((vneeded==2)||(vneeded==3)) 
						{
							v_left.at(j,i).re = vl(int(j),int(i));
							v_left.at(j,i).im = vl(int(j),int(i+1));
							v_left.at(j,i+1).re = vl(int(j),int(i));
							v_left.at(j,i+1).im = -vl(int(j),int(i+1));
						}
					}
					i+=2;
				}
				else
				{
					++i;
				}
			}
		}
	}
	return wsucc;
}


XRAD_END

