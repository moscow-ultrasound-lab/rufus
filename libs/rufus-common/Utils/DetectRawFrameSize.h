#ifndef DetectRawFrameSize_h__
#define DetectRawFrameSize_h__

/********************************************************************
	created:	2016/07/07
	created:	7:7:2016   11:59
	author:		kns

	purpose:
		автоматическое определение размеров изображения по массиву сырых данных наподобие photoshop.raw
		возможно два варианта использования.

		1.	определение длины строки пикселей в предположении, что данные содержат только один кадр
			в этом случае параметр row_size задается равным 1.

		2.	определение размеров кадра в многокадровом наборе. в этом случае сначала по
			правилам п. 1 определяется длина одной строки, затем найденное значение передается
			в качестве row_size в эту же процедуру.

		источником информации служат разрывы между концом/началом строки/кадра.
		если данные гладкие и таких разрывов нет, алгоритм не сработает.

		величина data_quantum (точность значений данных) для обычных изображений равна 1.
		в общем случае надо подбирать.

		должно работать для скалярных, цветных и комплексных наборов данных

*********************************************************************/

#include <XRADBasic/Containers.h>
#include <XRADBasic/Sources/Containers/DataArrayAnalyze.h>
#include <XRADBasic/Sources/Containers/DataArrayHistogram.h>

XRAD_BEGIN

template<class ARR_T>
size_t DetectRawFrameSize(const ARR_T &raw_data, size_t row_size, double data_quantum)
{
	// многокадровый случай: ищем разрывы между кадрами (строками).
	// вычитаем соседние лучи (отсчеты). на границах велика вероятность скачка.

	RealFunctionF32 peaks_function;
	size_t	col_size = raw_data.size()/row_size;
	peaks_function.realloc(col_size, 0);
	typedef	typename ARR_T::value_type sample_type;
	const size_t n_data_components(n_components(raw_data.at(0)));

	for(size_t col_no = 0; col_no < row_size; ++col_no)
	{
		// строка как правило намного короче, поэтому внутренний цикл по столбцу
		typename ARR_T::const_iterator
			it1 = raw_data.cbegin() + col_no,
			it2 = it1 + row_size,
			ie = raw_data.cend() - row_size;// конечный итератор не доводим совсем до конца массива

		RealFunctionF32::iterator	pe = peaks_function.begin();
		for(; it2<ie; it1+=row_size, it2+=row_size, ++pe)
		{
			// усредненная разность между двумя строками (много кадров)
			// или двумя отсчетами (один кадр)
			for(size_t k = 0; k < n_data_components; ++k)
			{
				*pe += fabs(double(component(*it1, k)) - double(component(*it2, k)));
			}
		}
	}
	peaks_function /= row_size;
	range1_F64 absolute_range_peaks(MinValue(peaks_function), MaxValue(peaks_function));

	// оцениваем размер гистограммы для анализа пиков. поскольку исходные данные целочисленные,
	// просто преобразуем диапазон значений к целому
	size_t peaks_histogram_size = absolute_range_peaks.delta()/data_quantum;
	RealFunctionF64	peaks_histogram(peaks_histogram_size);
	ComputeHistogram(peaks_function, peaks_histogram, absolute_range_peaks);

	// ищем функцию распределения пиков и определяем порог,
	// отделяющий 0,3% самых больших скачков.
	// они и должны соответствовать границам кадров.
	// величина 0,3% получена эмпирически.

	double	up_distribution_threshold = (1. - 1./300.);
	RealFunctionF64	peaks_cdf(peaks_histogram);//cumulative distribution function
	for(size_t i = 1; i < peaks_histogram_size-1; ++i)
	{
		peaks_cdf[i+1] += peaks_cdf[i];
	}
	peaks_cdf /= peaks_cdf[peaks_histogram_size-1];//важная поправка, иначе может не дойти до 1 совсем немного
	size_t threshold_position = 0;
	while(peaks_cdf[threshold_position]< up_distribution_threshold)
	{
		++threshold_position;
	}

	// ищем расстояния между пиками, превышающими найденный порог
	double threshold_peak_value = absolute_range_peaks.p1() + data_quantum*threshold_position;
	size_t	previous_position(0);
	RealFunctionUI32	deltas(128, 0);

	for(size_t i = 0; i < peaks_function.size(); ++i)
	{
		if(peaks_function[i] >= threshold_peak_value)
		{
			size_t	delta = i-previous_position;
			if(delta>=deltas.size()) deltas.resize(delta+128);//расширяем намного, чтобы не делать этого слишком часто
			++deltas[delta];
			previous_position = i;
		}
	}

	size_t ray_i; //буфер для готового решения
	MaxValue(deltas, &ray_i);
	return ray_i;
}

XRAD_END

#endif // DetectRawFrameSize_h__
