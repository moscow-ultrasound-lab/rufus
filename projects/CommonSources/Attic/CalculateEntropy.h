#ifndef __CalculateEntropy_h
#define __CalculateEntropy_h

//------------------------------------------------------------------
//
//	created:	2014/02/21
//	created:	21.2.2014   19:57
//	filename: 	Q:\programs\SpeedEstimate\SpeedEstimateSources\CalculateEntropy.h
//	file path:	Q:\programs\SpeedEstimate\SpeedEstimateSources
//	author:		KNS
//
//	purpose:	алгоритмы вычисления энтропии изображения
//
//------------------------------------------------------------------

#include <XRADBasic/ContainersAlgebra.h>

XRAD_BEGIN



namespace Functors
{

// вспомогательный функтор, используется для вычисления энтропии
// с помощью оптимизированного алгоритма обхода гистограммы
template<class RT>
struct acquire_entropy_functor
	{
	acquire_entropy_functor(RT *result): result(result) {}
	template <class VT>
	void operator()(const VT &x) const {if(x>0) *result -= x*log(x);}
	private:
		RT *result;
	};



// вспомогательный функтор, используется для вычисления гистограммы целочисленного изображения
// с помощью оптимизированного алгоритма обхода данных
template<class HT, class VT>
struct acquire_integral_histogram_functor
	{
	const VT histogram_min;
	const VT histogram_max;
	const HT increment;
	HT *histogram_ptr;

	acquire_integral_histogram_functor(HT *histogram_ptr, const VT &hmin, const VT &hmax, const HT &inc) :
		histogram_ptr(histogram_ptr), increment(inc), histogram_min(hmin), histogram_max(hmax){}
	void operator()(const VT &x) const
		{
		#ifdef _DEBUG
		if(!in_range(x,histogram_min,histogram_max))
			{
			ForceDebugBreak();
			throw out_of_range(ssprintf("acquire_integral_histogram, index %d out of range (%d,%d)", x, histogram_min, histogram_max));
			}
		#endif

		histogram_ptr[x-histogram_min] += increment;
		}
	};

} // namespace Functors



// вычисление информационной энтропии по гистограмме
// сумма всех элементов гистограммы должна быть равна 1
template<class ROW_T>
double	ComputeHistogramEntropy(const ROW_T &histogram)
	{
	double	result = 0;
	Apply_A_1D_F1(histogram, Functors::acquire_entropy_functor<double>(&result));
	return result/ln_2();
	}


// вычисление информационной энтропии целочисленного двумерного массива
// через вычисление гистограммы по заданному диапазону значений.
// в debug версии выполняется контроль значений, т.о., если изображение
// содержит данные вне диапазона min_value--min_value, выдается сообщение.
// в release версии никаких таких проверок не предусмотрено

template<class ROW_T>
double	ComputeIntegralImageEntropy(const DataArray2D<ROW_T> &image, typename ROW_T::value_type min_value, typename ROW_T::value_type max_value)
	{
	static_assert(is_integral<ROW_T::value_type>::value, "ComputeIntegralImageEntropy, only integral sample is allowed");

	//	assert_integral_type(ROW_T::value_type);
	// нецелочисленный массив даст ошибку компилятора

	int	histogram_size = (max_value - min_value)+1;

	DataArray<double> histogram(histogram_size, 0);
	// поскольку контейнер объявлен здесь, пользуемся тем, что шаг
	// в нем заведомо равен 1, и вместо итератора используем указатель
	double	*h0 = &histogram[0];

	const double	increment(1./double(image.vsize()*image.hsize()));
	Apply_A_2D_F1(image, Functors::acquire_integral_histogram_functor<double, typename ROW_T::value_type>(h0, min_value, max_value, increment));
	return	ComputeHistogramEntropy(histogram);
	}

// вычисление информационной энтропии целочисленного двумерного массива
// через вычисление гистограммы по всем возможным значениям. не следует
// пытаться вызывать эту функцию для данных разрядности более 16 бит

template<class ROW_T>
double	ComputeIntegralImageEntropy(const DataArray2D<ROW_T> &image)
	{
//	assert_integral_type(typename ROW_T::value_type);
	static_assert(is_integral<ROW_T::value_type>::value, "ComputeIntegralImageEntropy, only integral sample is allowed");
	// нецелочисленный массив даст ошибку компилятора
//	assert_nonconst_type(typename ROW_T::value_type);
	static_assert(!std::is_const<typename ROW_T::value_type>::value, "see comment").
	// открылось, что в MSVC numeric_limits некорректно работают с константным value_type
	// во избежание этого запрещаем такие данные (будет ошибка компилятора)

	//assert_typesize_does_not_exceed(typename ROW_T::value_type, 16);
	static_assert(bitsizeof(type) <= 16, "ComputeIntegralImageEntropy, too wide integral type").
	// эту функцию можно использовать только для 8- и 16-битных данных,
	// иначе размеры гистограммы получаются недопустимо большими

	int	min_value = numeric_limits<ROW_T::value_type>::min();
	int	max_value = numeric_limits<ROW_T::value_type>::max();

	return ComputeIntegralImageEntropy(image, min_value, max_value);
	}



XRAD_END

#endif //__CalculateEntropy_h