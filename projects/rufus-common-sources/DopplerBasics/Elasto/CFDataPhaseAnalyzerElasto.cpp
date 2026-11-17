#include "pre.h"
#include <XRADBasic/Sources/Utils/ExponentialBlurAlgorithms.h>
#include <DopplerBasics/Utils/Auxiliaries.h>

#include "CFDataPhaseAnalyzerElasto.h"
#include <XRADBasic/Sources/Utils/ImageUtils.h>

/********************************************************************
	created:	2016/10/20
	created:	20:10:2016   14:10
	filename: 	q:\programs\ElastoGrafica\ElastoGrafica\FrameElastoAnalyzer.cpp
	file path:	q:\programs\ElastoGrafica\ElastoGrafica
	file base:	FrameElastoAnalyzer
	file ext:	cpp
	author:		kns
	
	purpose:	
*********************************************************************/


XRAD_BEGIN


namespace
{
void	CancelRowPolynomialPart(RealFunction2D_F32 &f, size_t order)
{
	RealVectorF64	coefficients(order);
	RealFunctionF64	arguments(f.hsize());
	RealFunctionF64	average_values(f.hsize(), 0);

	for(size_t i = 0; i < f.hsize(); ++i) arguments[i] = i;//argument_functor(i);

#pragma omp parallel for schedule (guided)
	for(ptrdiff_t i = 0; i < f.vsize(); ++i)
	{
		average_values += f.row(i);
	}
	DetectLSPolynomNonUniformGrid(average_values, arguments, coefficients); //samples, grid, coefficients

	coefficients /= f.vsize();
	double	average = polynom(coefficients, f.hsize()/2.);

#pragma omp parallel for schedule (guided)
	for(ptrdiff_t j = 0; j < f.hsize(); ++j)
	{
		double correction = (average-polynom(coefficients, j));
		f.col(j) += correction;
	}
}
}



void	CFDataPhaseAnalyzerElasto::AnalyzeFrame(RealFunction2D_F32 &result_elastogram, RealFunction2D_F32 &result_mask)
{
	ConjugateRFData();
	BlurRFData();

	AcquirePhaseElastoOriginal();
	PostfilterElastogram();


	CalculateMask(result_mask);

	result_elastogram.CopyData(m_phase);
}

void	CFDataPhaseAnalyzerElasto::CalculateMask(RealFunction2D_F32 &mask)
{
	// возможно, какую-то маску получится задействовать дополнително, надо подумать...
	mask.fill(1.0);
};

void	CFDataPhaseAnalyzerElasto::NormalizeDynamics(RealFunction2D_F32 &elastogram)
{
	size_t	order_s = 2;
	size_t	order_r = 2;

	CancelRowPolynomialPart(elastogram, order_s);
	elastogram.transpose();
	CancelRowPolynomialPart(elastogram, order_r);
	elastogram.transpose();
}


void	CFDataPhaseAnalyzerElasto::PostfilterElastogram()
{
	tp_postfilter.Start();
	NormalizeDynamics(m_phase);
	CutHistogramEdges(m_phase, range1_F64(0.05, 0.95));
		// пробуем хитрую несимметричную схему фильтрации эластограммы:
		// выше мы фильтровали комплексный сигнал вдоль луча на величину axial_blur.
		// поперечная координата при этом оставалась нетронутой. теперь фильтруем на тот же
		// радиус поперечную координату. продольная фильтрация делается только для учета
		// aspect ratio

//	BiexpBlur2D(m_phase, result_axial_blur, result_lateral_blur);//TODO вернуть реализацию с omp

	//TODO: здесь ведется учет отрицательной составляющей фазы,
	//поправка фазы зависит от средней величины измеренной фазы (пороговый критерий).
	//цель этого: если просто удалять отрицательные значения, мы получаем
	//бесконечную жесткость в этих местах. для отображения ничего, но для
	//подсчета strain ratio нехорошо.
	//перепроверить всё еще раз.
#if 1
	double	quality_factor = 1;//0.5*fabs(average_offset*sqrt(double(n_cfm_beams_in_sweep)))/phase_treshold;
	double minval = MinValue(m_phase) * range(quality_factor, 0, 1);
	//if(minval<0)
	m_phase -= minval;

	ApplyFunction(m_phase, [](const float &v) { return positive(v); });

	NormalizeFrameRange(m_phase);
	m_phase.AcquireFrame(frame_agility_factor, result_axial_blur, result_lateral_blur);
//	ApplyFrameAveraging(phase_buffer, phase_averaging_buffer);
	NormalizeFrameRange(m_phase);
#endif
		//дважды делается нормировка: до усреднения и после.
		//первая нужна, чтобы все эластограммы перед усреднением были
		//в одном диапазоне, вторая чтобы контрастность выходного
		//изображения была всегда максимальной.
		//возможно, есть избыточность действий. продумать.
	tp_postfilter.Stop();
}



void CFDataPhaseAnalyzerElasto::AcquirePhaseElastoOriginal()
{
	tp_acquire.Start();
	RealFunctionF64	offsets(n_cfm_beams(), 0);

#pragma omp parallel for schedule (guided)
	for(ptrdiff_t i = 0; i < n_cfm_beams(); ++i)
	{
	// цикл по лучам ЦДК
		shots_array_t::row_type::iterator shots_row_it = shots_array.row(i).begin();
		shots_array_t::row_type::iterator shots_row_ie = shots_array.row(i).end();
		RealFunction2D_F32::row_type::iterator phase_it = m_phase.row(i).begin();
		RealFunction2D_F32::row_type::iterator phase_it1 = phase_it;
		double	*offset = &offsets[i];

		for(size_t j = 0; shots_row_it<shots_row_ie; ++shots_row_it, ++phase_it, ++j)
		{
		// цикл по глубинам ЦДК
			cfm_array_t::iterator	shot_it = shots_row_it->begin();
			cfm_array_t::iterator	shot_ie = shot_it + (n_cfm_shots()-conjugated_count());
			complexF64	accumulator(0);

			//NB: проверено, что усреднение фазового аккумулятора по глубине ничего хорошего не дает
			for(; shot_it < shot_ie; ++shot_it)
			{
			// цикл внутри "пачки"
				accumulator += *shot_it;
			}

			double	current_offset = arg(accumulator);
			*phase_it = fabs(current_offset);
			*offset += current_offset;

			// дифферецирование вдоль луча
			if(j)
			{
				*phase_it1 = (*phase_it - *phase_it1);
				++phase_it1;
			}
		}
	// последнее значение повторяем
		*(--phase_it) = *(--phase_it1);
	}
	m_phase.col(0).CopyData(m_phase.col(1));

	tp_acquire.Stop();

//	double	normalizer = (n_cfm_beams_in_sweep*n_cfm_samples*(n_cfm_shots-1));// так было сначала
	double	normalizer = n_cfm_beams_in_sweep()*(n_cfm_shots()-1);

	m_average_frame_offset =  AverageValue(offsets)/normalizer;
	#pragma message Необходимо проанализировать нормировку m_average_frame_offset, чтобы прибор получал значение в корректном масштабе

}


XRAD_END
