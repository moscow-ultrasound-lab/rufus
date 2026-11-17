// BlurAlgorithms.cc
// created 22.03.2010
// KNS



XRAD_BEGIN

namespace	rasp_blur_algorithms
	{


	//--------------------------------------------------------------------------
	//
	//	действие бих-фильтра 1-го порядка в одной точке
	//

	inline	void iir_one_point(float &result, const float &b0, const float &a0, const float &a1)
		{
		result = (result*a1 + b0*a0);
		}

	inline	void iir_one_point(double &result, const double &b0, const double &a0, const double &a1)
		{
		result = (result*a1 + b0*a0);
		}

	template<class integer_sample>
	inline	void iir_one_point(integer_sample &result, const integer_sample &b0, const integer_sample &a0, const integer_sample &a1)
//	inline	void iir_one_point(integer_sample &result, const integer_sample &b0, integer_sample a0, integer_sample a1)
		{
		result = (result*a1 + b0*a0) >> fixed_point_position(result);
		//((result *= a1) += (b0*a0)) >>= fixed_point_position(result);
		}



	template<class sample, class index>
	void	iir_array_1d(sample *base,
			index data_size, index data_step,
			sample a0, sample a1)
		{
		sample	*b0 = base;
		sample	*b1 = base + data_step;
		sample	*b_max = b0 + data_size*data_step;

		for(; b1 < b_max; b0+=data_step, b1+=data_step)
			{
			iir_one_point(*b1, *b0, a0, a1);
			}

		b0 -= data_step;
		b1 -= data_step;

		for(; b0 >= base; b0-=data_step, b1-=data_step)
			{
			iir_one_point(*b0, *b1, a0, a1);
			}
		}


	} //namespace rasp_blur_algorithms





template<class ARR>
inline void	BIXBlur1D(ARR &f, ARR::value_type a0, ARR::value_type a1)
	{
	rasp_blur_algorithms::iir_array_1d(&f[0], f.size(), f.step(), a0, a1);
	}




template<class AR2D>
void	UniversalBlur2D(AR2D &im, double radius_v, double radius_h)
	{
		
	static  double	edge_level = 0.3;
	static  double	log_level = log(edge_level);
	static	double	previous_rv = 0, previous_rh;

	static	AR2D::value_type	normalizer_factor = multiply_normalizer(AR2D::value_type());
//	static	AR2D::value_type     a0v = normalizer_factor;
//	static	AR2D::value_type     a1h = normalizer_factor - a0;
	static	double	av(0), ah(0);

	if(radius_v<=0 || radius_h<=0) return;


	if(radius_v != previous_rv)
		{
		av = exp(log_level/radius_v);
		previous_rv = radius_v;
		}

	if(radius_h != previous_rh)
		{
		ah = exp(log_level/radius_h);
		previous_rh = radius_h;
		}
	//TI optimized version
	#ifdef __TI_COMPILER_VERSION__
		
		//-------------------------------------------------------------
		// filtering rows
		//
		uint16_t	a0h, a1h;
		iir2_prepare_coefficients(ah, a0h, a1h);
		//------------------------------------------------------------------------
		//
		//	для фильтрации как по строкам, так по столбцам выбраны оптимальные 
		//	по скорости варианты конвейерной обработки. 
		//	по строкам: основной объем массива обрабатывается алгоритмом свертки,
		//	в котором обрабатываются 8 элементов сразу. Затем идет обработка по 4 и 1.
		//	2 опускается. Обработка по 16 оказалась хуже, чем по 8 (результат измерений)
		//	по столбцам: наилучший вариант обработки по 16, остаток по 4. меньшие
		//	варианты дробления невозможны по соображениям выравнивания начального
		//	элемента строки
		//	 
		//	
		int	i8 = im.vsize() - im.vsize()%8;
		int	i4 = im.vsize() - im.vsize()%4;
		int	i = 0;
		
		for(; i < i8; i+=8)
			TI_OptimizedFunctions::iir2_row_8(&im.row(i).at(0), im.hsize(), a0h, a1h);

		for(; i < i4; i+=4)
			TI_OptimizedFunctions::iir2_row_4(&im.row(i).at(0), im.hsize(), a0h, a1h);

		for(; i < im.vsize(); ++i)
			TI_OptimizedFunctions::iir2_row_1(&im.row(i).at(0), im.hsize(), a0h, a1h);

		//-------------------------------------------------------------
		// filtering columns
		//
		uint16_t	a0v, a1v;
		iir2_prepare_coefficients(av, a0v, a1v);
		
		int	j16 = im.hsize() - im.hsize()%16;
		int	j4 = im.hsize() - im.hsize()%4;
		int	j = 0;
				
		for(; j < j16; j+=16)
			iir2_16_columns(&im.at(0,j), im.vsize(), im.vstep(), a0v, a1v);		
		
		for(; j < j4; j+=4)
			iir2_4_columns(&im.at(0,j), im.vsize(), im.vstep(), a0v, a1v);		
		
		//	вычисления только с выравниванием по 16 и по 4 отсчета.
		//	прочие варианты исключены дефинициями класса RASP_Processor.
		//	NB. Следует, вероятно, синтаксически это обусловить, чтобы
		//	ошибка компиляции возникала?	
			

	//Universal version
	#else
		AR2D::value_type a0v = av*normalizer_factor;
		AR2D::value_type a1v = normalizer_factor - a0v;
		AR2D::value_type a0h = ah*normalizer_factor;
		AR2D::value_type a1h = normalizer_factor - a0h;

		for(int i = 0; i < im.vsize(); i++) rasp_blur_algorithms::iir_array_1d(&im.row(i).at(0), im.hsize(), im.hstep(), a0h, a1h);
		for(int j = 0; j < im.hsize(); j++) rasp_blur_algorithms::iir_array_1d(&im.col(j).at(0), im.vsize(), im.vstep(), a0v, a1v);
	#endif
	}



XRAD_END

