#ifndef __BlurAlgorithms_h
#define __BlurAlgorithms_h

// Файл устарел, но содержащиеся в нем алгоритмы могут быть полезны. Речь идет о вычислениях с фиксированной запятой над целочисленными данными

#error не следует включать этот файл в какие-либо проекты
//------------------------------------------------------------------
//
//	created:	2014/06/07
//	created:	7.6.2014   14:39
//	filename: 	Q:\programs\ElastographyTest\sources\ElastoBlurAlgorithms.h
//	file path:	Q:\programs\ElastographyTest\sources
//	author:		kns
//	
//	purpose:	
//
//------------------------------------------------------------------

//TODO варианты в этом файле оказываются заметно быстрее, чем в ExponentialBlurAlgorithms.h. Надо понять: почему так.

#include <omp.h>

XRAD_BEGIN

#pragma warning (disable:4018)

	
template<class sample, class factor>
inline	void elasto_iir_one_point_div(sample &result, const sample &b0, const factor &a0, const factor &a1_a0)
	{
	// еще более экономичный вариант предыдущего, не требуется
	// буферная переменная. вместо множителя a1 передается величина a1/a0
	result *= a1_a0;
	result += b0;
	result *= a0;
	}



template<class iterator, class factor>
inline void	biexp_filter_1d_it(iterator &start, iterator &end,
		factor a0, factor a1)
	{
	if(!a0) return;
	
	iterator	b0 = start;
	iterator	b1 = start + 1;
	factor		a1_a0 = a1/a0;
	
#if 1
	for(; b1 < end; ++b0, ++b1)
		{
		elasto_iir_one_point_div(*b1, *b0, a0, a1_a0);
		}	
#else
	int	offset(end-b1);
	b1 += offset;
	b0 += offset;
#endif
	--b0;
	--b1;
	
	
	for(; b0 >= start; --b0, --b1)
		{
		elasto_iir_one_point_div(*b0, *b1, a0, a1_a0);
		}
	}

template<class ARR, class factor>
void	elasto_biexp_filter_1d_arr(ARR &array, factor a0, factor a1)
	{
	if(array.step()==1)
		{
		typename ARR::value_type *it = &array[0];
		typename ARR::value_type *ie = it+array.size();
		biexp_filter_1d_it(it, ie, a0, a1);
		}
	else
		{
		typename ARR::iterator it = array.begin();
		typename ARR::iterator ie = array.end();
		biexp_filter_1d_it(it, ie, a0, a1);
		}
	}

// вириант без OMP (можно встраивать внутрь циклов, учитывающих OMP)
template<class AR2D>
void	ElastoBiexpBlur2D(AR2D &im, double radius_v, double radius_h)
	{
	static double	edge_level = 0.3;
	static double	log_level = log(edge_level);
	
	if(radius_h)
		{
		double	a0h = exp(log_level/radius_h);
		double	a1h = 1.-a0h;
		for(int i = 0; i < im.vsize(); ++i)
			{
			elasto_biexp_filter_1d_arr(im.row(i), a0h, a1h);
			}
		}
	if(radius_v)
		{
		double	a0v = exp(log_level/radius_v);
		double	a1v = 1.-a0v;
		for(int j = 0; j < im.hsize(); ++j)
			{
			elasto_biexp_filter_1d_arr(im.col(j), a0v, a1v);
			}
		}
	}

// вириант с использованием OMP
template<class AR2D>
void	ElastoBiexpBlur2D_omp(AR2D &im, double radius_v, double radius_h)
	{
	static const double	edge_level = 0.3;
	static const double	log_level = log(edge_level);
	
	if(radius_h)
		{
		double	a0h = exp(log_level/radius_h);
		double	a1h = 1.-a0h;
// 		#pragma omp parallel for num_threads(4)
//		вариант guided показал самый лучший результат 6,4 мс
//		num_threads(8) катастрофически хуже: 119 мс
//		num_threads(2) 29 мс
//		num_threads(4) 7,4 мс
//		когда есть возможность применить omp к более крупному
//		циклу, лучше это сделать: вызов с внешним omp показывает
//		5,5 мс.
 		#pragma omp parallel for schedule (guided)
		for(int i = 0; i < im.vsize(); ++i)
			{
			elasto_biexp_filter_1d_arr(im.row(i), a0h, a1h);
			}
		}
	if(radius_v)
		{
		double	a0v = exp(log_level/radius_v);
		double	a1v = 1.-a0v;
 		#pragma omp parallel for schedule (guided)
		for(int j = 0; j < im.hsize(); ++j)
			{
			elasto_biexp_filter_1d_arr(im.col(j), a0v, a1v);
			}
		}
	}

XRAD_END


#endif //__BlurAlgorithms_h