#include "pre.h"
#include "CFDataPhaseAnalyzerTwinkling.h"
/********************************************************************
	created:	2016/10/21
	created:	21:10:2016   15:32
	filename: 	q:\programs\ElastoGrafica\ElastoGrafica\CFDataPhaseAnalyzerTwinkling.cpp
	file path:	q:\programs\ElastoGrafica\ElastoGrafica
	file base:	CFDataPhaseAnalyzerTwinkling
	file ext:	cpp
	author:		kns
	
	purpose:	 
*********************************************************************/
XRAD_BEGIN

#error Возможности этого класса включены в CFDataPhaseAnalyzerCFM. Использовать его напрямую больше не следует

float mask_val1(float(1)), mask_val2(float(-1)); //определяет цвет на шкале для компонент С и D
												 //для отладки на компьютере ставим float mask_val1(float(1)), mask_val2(float(-1));
												 //при загрузке в прибор выставляется float mask_val1(float(0.8)), mask_val2(float(0.4));
template<class T1, class T2>
struct Merge_std_cor_Cavitation
{
private:
	double	threshold1, threshold2;
public:
	typedef typename remove_const<T1>::type result_type;
	typedef T2 argument_type;
	Merge_std_cor_Cavitation(double in_1, double in_2) : threshold1(in_1), threshold2(in_2) {}
	result_type& operator()(result_type &mask, const T2 &stddev, const T2 &correlation) const
	{
		if ((stddev>threshold1)&&(correlation<threshold2)) return mask = 1;  
		else return mask = 0; 
	}
};

template<class T1, class T2>
struct Merge_re_im_cor_Masks
{
private:
	double	threshold1, threshold2;
public:
	typedef typename remove_const<T1>::type result_type;
	typedef T2 argument_type;
	Merge_re_im_cor_Masks(double in_1, double in_2) : threshold1(in_1), threshold2(in_2) {}
	result_type& operator()(result_type &mask, const T2 &mask_buffer, const T2 &re_im_correlation) const
	{
		if ((mask_buffer > threshold1)&&(re_im_correlation > threshold2)) return mask = mask_val1;
		else if (mask_buffer > threshold1) return mask = mask_val2;
		else return mask = 0;
	}
};

template<class T1, class T2>
struct Merge_cor_Masks
{
private:
	double	threshold1, threshold2;
public:
	typedef typename remove_const<T1>::type result_type;
	typedef T2 argument_type;
	Merge_cor_Masks(double in_1, double in_2) : threshold1(in_1), threshold2(in_2) {}
	result_type& operator()(result_type &mask, const T2 &mask_buffer, const T2 &correlation) const
	{
		if (mask_buffer == mask_val1) return mask = mask_val1;
		else if ((mask_buffer == mask_val2) && (correlation < threshold2)) return mask = mask_val2;
		else return mask = 0;
	}
};

void	 CFDataPhaseAnalyzerCascillation::CalculateMask()
{
	XRAD_ASSERT_THROW(is_number(cor_threshold));// параметр не инициализируется, следов нет. При возврате к этой процедуре сделать правильную инициализацию
	XRAD_ASSERT_THROW(is_number(re_im_cor_threshold));// параметр не инициализируется, следов нет. При возврате к этой процедуре сделать правильную инициализацию
	XRAD_ASSERT_THROW(is_number(std_threshold));// параметр не инициализируется, следов нет. При возврате к этой процедуре сделать правильную инициализацию
	XRAD_ASSERT_THROW(is_number(amplitude_threshold));// параметр не инициализируется, следов нет. При возврате к этой процедуре сделать правильную инициализацию

	m_mask.fill(1);
	Apply_AAA_2D_F3(m_mask, m_mask, m_amplitude, blood_mask<float, float>(0, amplitude_threshold));

	ComputeStandardDeviation();
	Apply_AAA_2D_F3(m_mask, m_mask, m_stddev, blood_mask<float, float>(0, std_threshold));

	ComputeReImCorrelation();
	Apply_AAA_2D_F3(m_mask, m_mask, m_correlation_re_im, Merge_re_im_cor_Masks<float, float>(0, re_im_cor_threshold));

	ComputeCorrelationCoefficient();
	Apply_AAA_2D_F3(m_mask, m_mask, m_correlation, Merge_cor_Masks<float, float>(0, cor_threshold));
}

void	 CFDataPhaseAnalyzerCavitation::CalculateMask() 
{
	ComputeStandardDeviation();
	ApplyFunction(m_stddev, binarization_functor<float>(0, 7));
	ComputeCorrelationCoefficient();
	ApplyFunction(m_correlation, binarization_functor<float>(0.3, 0.7));
	m_correlation *= -1;
	m_correlation += 1;
	m_stddev *= m_correlation;
	ComputeReImCorrelation();
	ApplyFunction(m_correlation_re_im, binarization_functor<float>(0.3, 0.7));
	m_correlation_re_im *= -1;
	m_correlation_re_im += 1;
	m_stddev *= m_correlation_re_im;
	
	m_mask.CopyData(m_stddev);
#pragma message TODO filter
}

void	 CFDataPhaseAnalyzerCavitation::AnalyzeFrame(RealFunction2D_F32 &result_cavitation_map, RealFunction2D_F32 &result_mask)
{
	TissueMotionCompensation();
	ApplyWallFilter();
	
	XRAD_ASSERT_THROW_M(false, logic_error, "Function is not correct, see comment");
	// в настоящем виде неясно, где считается результат, необходимо трассировать код и смотреть, где полезные части

// 	CalculateMask(result_mask);
// 	cavitation_map.CopyData(result_mask);
// 	result_mask.fill(1.0);
}

void CFDataPhaseAnalyzerOscillation::CalculateImaginarySignal()
{
	for (size_t i = 0; i < n_cfm_beams(); ++i)
	{
		// цикл по лучам ЦДК
		shots_array_t::row_type::iterator shots_row_it = shots_array.row(i).begin();
		shots_array_t::row_type::iterator shots_row_ie = shots_array.row(i).end();
		RealFunction2D_F32::row_type::iterator eit_pb = m_phase.row(i).begin();
		for (size_t j = 0; shots_row_it < shots_row_ie; ++shots_row_it, ++j, ++eit_pb)
		{
			// цикл по глубинам ЦДК
			cfm_array_t::iterator	it = shots_row_it->begin();
			cfm_array_t::iterator	ie = it + (n_cfm_shots());
			ComplexFunctionF32 burst(n_cfm_shots(), complexF32(0));
			std::copy(it, ie, burst.begin());
			double frequency(0), a(0), b(0);
			for (size_t l = 0; l < (n_cfm_shots() - 1); ++l)
			{
				burst[l].im = burst[l + 1].re - burst[l].re;
				a += fabs(burst[l].im);
				b += fabs(burst[l].re);
			}
			frequency = (a / b)*double(n_cfm_shots())/two_pi();
			*eit_pb = frequency;
			std::copy(burst.begin(), burst.end(), it);
		}
	}
}

void	 CFDataPhaseAnalyzerOscillation::AnalyzeFrame(RealFunction2D_F32 &result_oscillation_map, RealFunction2D_F32 &result_mask)
{
	ComputePreWallFilterAmplitude();
	Apply_AAA_2D_F3(result_mask, m_amplitude, m_amplitude, blood_mask<float, float>(0, 0));
	TissueMotionCompensation();
	ApplyWallFilter();
	ComputeStandardDeviation();
	ComputeCorrelationCoefficient();
	ComputeReImCorrelation();
	CalculateImaginarySignal();
	m_phase *= prf / two_pi();
	result_mask.CopyData(m_phase); //в маску записывается оценка частоты по амплитуде
#if 1
	// действия вида n_cfm_shots -= 1 опасны с точки зрения скрытых ошибок. Надо подумать, как изменить алгоритм с учетом этого. Пока делаю ошибку
	throw logic_error("CFDataPhaseAnalyzerOscillation::AnalyzeFrame, требуется доработка алгоритма");

#else

	n_cfm_shots -= 1;
	ConjugateRFData();
	AcquirePhaseCFM();
	n_cfm_shots += 1;
#endif
	m_phase *= -prf / two_pi(); //в фазе содержится оценка частоты по действительной части сигнала и её преобразованию Гильберта
	frame_agility_factor = .1;

	//	ApplyFrameAveraging(phase_buffer, elastogram_averaging_buffer);

	m_phase.AcquireFrame(frame_agility_factor, result_axial_blur, result_lateral_blur);
	result_oscillation_map.CopyData(m_phase);
}

template<class T1, class T2=T1> 
struct	filter_one
{
	typedef typename remove_const<T1>::type result_type;
	typedef typename remove_const<T1>::type argument_type;
	result_type operator()(const argument_type &argument) const
	{
		if (argument != 0) return 1;
		else return 0;
	}
};

void	 CFDataPhaseAnalyzerCascillation::AnalyzeFrame(RealFunction2D_F32 &result_cascillation_map, RealFunction2D_F32 &result_mask)
{
	ComputePreWallFilterAmplitude();
	TissueMotionCompensation();
	ApplyWallFilter();
	CalculateMask();
	result_cascillation_map.CopyData(result_mask);
	ApplyFunction(result_mask, filter_one<float>());
};

XRAD_END