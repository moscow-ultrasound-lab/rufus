//ImportExportSample.cc



//	каждая пара входных/внутренних данных расписывается особо.
//	вариант, который не выписан, считается запрещенным


XRAD_BEGIN
//-------------------------------------------------------
//
//	входные и внутренние данные целочисленные
//


template<class T1, class T2>
inline T1&	ImportOriginalSample(T1& result, const T2& x)
	{
	return (result = x)<<=conversion_offset(result, x);
	}

template<class T1, class T2>
inline T2&	ExportProcessedSample(T2& result, const T1& x)
	{
	return proper_range(result, x>>conversion_offset(x,result));
	}


//-------------------------------------------------------
//
//	внутренние данные плавающие, входные любые
//

template<class T>
	inline float&	ImportOriginalSample(float& result, const T& x){return result = x;}
template<class T>
	inline T& ExportProcessedSample(T& result, const float& x){return proper_range(result, x);}

template<class T>
	inline double&	ImportOriginalSample(double& result, const T& x){return result = x;}
template<class T>
	inline T& ExportProcessedSample(T& result, const double& x){return proper_range(result, x);}



//-------------------------------------------------------
//
//	вспомогательные функции для преобразования величины с плавающей запятой
//	в формат внутреннего отсчета, применяемого для обработки
//

//	inline	float&	import_floating_value(float& x, const float& y) {return x=y;}
//	inline	double&	import_floating_value(double& x, const double& y){return x=y;}
//
//	template<class T>
//	inline	T	&import_floating_value(T& x, const double& y)
//		{
//		return x = y*import_value_normalizer(x,y);
//		}

//-------------------------------------------------------
//
//	нахождение максимального допустимого значения внутреннего
//	отсчета, которое не приведет к переполнению при экспорте
//	в формат внешнего отсчета. это значение прописывается в
//	объект типа TextureProcessor<> сразу после создания.
//	далее в процессе обработки возможен учет и коррекция
//

template<class T>
inline	T MaxAllowedSampleValue(uint8_t)
	{
	T	result;
//	MAX_FLOAT
//	return ImportOriginalSample(result, UINT8_MAX);
	return ImportOriginalSample(result, std::numeric_limits<uint8_t>::max());
	}

template<class T>
inline	T MaxAllowedSampleValue(uint16_t)
	{
	T	result;
	return ImportOriginalSample(result, UINT16_MAX);
	}

template<class T>
inline	T MaxAllowedSampleValue(int16_t)
	{
	T	result;
	return ImportOriginalSample(result, INT16_MAX);
	}

template<class T>
inline	T MaxAllowedSampleValue(int32_t)
	{
	T	result;
	return ImportOriginalSample(result, INT32_MAX);
	}

template<class T>
inline	T MaxAllowedSampleValue(uint32_t)
	{
	T	result;
	return ImportOriginalSample(result, UINT32_MAX);
	}

template<class T>
inline	T MaxAllowedSampleValue(float)
	{
	T	result;
	return ImportOriginalSample(result, MAX_FLOAT);
	}

template<class T>
inline	T MaxAllowedSampleValue(double)
	{
	T	result;
	return ImportOriginalSample<T, double>(result, MAX_DOUBLE);
	}




//------------------------------------------------------------------------------
//
//	копирование данных из исходного пользовательского массива в массив обработки
//  функция копирования данных
//
//------------------------------------------------------------------------------

template<class T>
T	InterceptCorrection(T intercept)
	{
	#ifndef KT_INTERCEPT
	return intercept;
	#else
	return intercept - 1024;
	#endif
	}




//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

template<class PROC_BUF, class IN_BUF>
void	ImportOriginalImage(PROC_BUF &a1, const IN_BUF &a2, IN_BUF::value_type intercept, float slope)
	{

//	if(a1.vsize() != a2.vsize() || a1.hsize() != a2.hsize()) return;
//	AA_2D_OpEq(a1, a2, RASPFunctors::import_sample_functor<PROC_BUF::value_type, IN_BUF::value_type>());
//	теперь (с учетом выравнивания) массивы могут быть различных размеров.
//	вместо двух строк идущих выше одна эта:

	DifferentAA_2D_OpEq(a1, a2, RASPFunctors::import_sample_functor<PROC_BUF::value_type, IN_BUF::value_type>(), extrapolation::by_last_value);
//	DifferentAA_2D_OpEq(a1, a2, RASPFunctors::import_sample_functor<PROC_BUF::value_type, IN_BUF::value_type>(), extrapolate_by_zero);

	IN_BUF::value_type	applied_intercept = InterceptCorrection(intercept);

	if(applied_intercept)
		{
		// величина intercept прибавляется к данным до импорта,
		// и вычитается из данных после экспорта. поэтому она должна
		// быть в одно с ними масштабе. значит, на импорте вычитаем ее
		// после преобразования основных значений, а на экспорте напротив,
		// сначала прибавляем интерсепт, а потом масштабируем значения
		PROC_BUF::value_type	imported_intercept;
		ImportOriginalSample(imported_intercept, applied_intercept);
		a1 -= imported_intercept;
		}
	if(slope != 1 && slope != 0) a1 *= slope;
	}


template<class IN_BUF, class PROC_BUF>
void	ExportProcessedImage(IN_BUF &a1, PROC_BUF &a2, IN_BUF::value_type intercept, float slope)
	{
//	здесь экспортируемый аргумент не const, он разрушается в процессе импорта
//	if(a1.vsize() != a2.vsize() || a1.hsize() != a2.hsize()) return;
//	теперь (с учетом выравнивания) массивы могут быть различных размеров.

	if(slope != 1 && slope != 0) a2 *= (1./slope);

	IN_BUF::value_type	applied_intercept = InterceptCorrection(intercept);
	if(applied_intercept)
		{
		PROC_BUF::value_type	imported_intercept;
		ImportOriginalSample(imported_intercept, applied_intercept);
		a2 += imported_intercept;
		}

//	AA_2D_OpEq(a1, a2, RASPFunctors::export_sample_functor<IN_BUF::value_type, PROC_BUF::value_type>());

	DifferentAA_2D_OpEq(a1, a2, RASPFunctors::export_sample_functor<IN_BUF::value_type, PROC_BUF::value_type>(), extrapolate_by_last_value);
	}

XRAD_END

