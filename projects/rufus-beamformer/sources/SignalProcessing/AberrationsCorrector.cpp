#include "Pre.h"
#include "AberrationsCorrector.h"
//#include <QMMacDialog.h>
#include <XRADBasic/Sources/Utils/LeastSquares.h>

extern	char	this_process_comment[MAXSTRINGSIZE];
//extern	qmMacDialog	*FX_Dialog;


XRAD_BEGIN




AberrationsCorrector::AberrationsCorrector()
{
//	focus_Algorithm = CORRECT_ABERRATION;
	Correlations_Count = 1;
	Bad_Correlations_Count = 0;
	Correlation_Power = GetFloating("Correlation power", 1, 1, 4);
	Intervals_To_Compute_Correction = 1;
}

AberrationsCorrector :: ~AberrationsCorrector()
{
//	if(Aberrations_Map) DestroyObject(Aberrations_Map);
	printf("\rCorrelation coefficient was found %.1f times", Correlations_Count);
	printf("\n%.1f bad values occured (%.1f per cent)",
		   Bad_Correlations_Count, 100. * (Bad_Correlations_Count/Correlations_Count));

	fflush(stdout);
}

struct time_to_float 
{
	typedef float result_type;
	typedef	physical_time argument_type;

	float &operator()(float &x, const physical_time &y) const { return x = y.sec(); }
};



void	AberrationsCorrector::Display(const char *Title)
{
	size_t	answer = 0;
	do
	{
		answer = GetButtonDecision(Title, {"Focused data","Aberrations","Exit display"});
		switch(answer)
		{
			case 0:	inherited::Display(Title);
				break;
			case 1:
			{
				RealFunctionMD_F32	displayer;

				displayer.MakeCopy(AberrationsMap, time_to_float());
				DisplayMathFunction3D(displayer, string("Aberrations for '") + string(Title) + string("'"));
			}
				break;
		}
	} while(answer != 2);
}



void	AberrationsCorrector::CreateComment()
{
// 	if(!TX_Focus.size()) sprintf(Object_Comment, "%s dynamic focus on TX aperture,\n", Object_Comment);
// 	else	sprintf(Object_Comment, "%s TX aperture focused at infinity,\n", Object_Comment);
// 	if(!RX_Focus.size()) sprintf(Object_Comment, "%s dynamic focus on RX aperture,", Object_Comment);
// 	else	sprintf(Object_Comment, "%s RX aperture focused at infinity,", Object_Comment);
// 
// 
// 	sprintf(Object_Comment, "%s, aberrations corrected", Object_Comment);
// 
// 	if(apodization_tx || apodization_rx) sprintf(Object_Comment, "%s Apodization.", Object_Comment);
// 	else sprintf(Object_Comment, "%s no Apodization.", Object_Comment);
// 
// 	sprintf(this_process_comment, Object_Comment);
}

void	AberrationsCorrector::InitWork()
{
	inherited::InitWork();
	AberrationsMap.realloc({n_rays, data_source->n_tx_elements, n_samples});
}


void AberrationsCorrector::ComputeAberration()
{
	//	SetAngleUnits(RADIANS);


	printf("\nIt may take some hours\r");
fflush(stdout);

//	RealFunction2D_F32	delays;

const TimeTable	correction_delays(data_source->n_tx_elements, n_samples, sec(0));
//const RealFunction2D_F32	amplitude_corrections(data_source->n_tx_elements, n_samples, 1);
amplitude_corrections.realloc(data_source->n_tx_elements, n_samples, 1);
//	const DistancesMatrix	correction_delays(n_samples, data_source->n_tx_elements, 0);

GUIProgressBar	progress;
progress.start("Analyzing delays", n_rays);
for (size_t ray = 0; ray < n_rays; ++ray)
{
	//		SetCurrentRay(ray);
	physical_angle	angle = CurrentRayAngle(ray);
	ComputeDelayedSignals(angle, correction_delays, amplitude_corrections);

	TimeTable	aberrations_slice;
	AberrationsMap.GetSlice(aberrations_slice,{ ray, slice_mask(0), slice_mask(1) });
	AnalyzeDelays(aberrations_slice, phase_detection);

	++progress;
}
progress.end();

//	DisplayAberrations("Computed aberrations map");

//	RealFunctionMD_F32	displayer;

displayer.MakeCopy(AberrationsMap, time_to_float());
DisplayMathFunction3D(displayer, string("Computed aberrations map 3D"));
//DataArrayMD<RealFunction2D_F32>::slice_type::invariable displayer_slice;
displayer.GetSlice(displayer_slice,{ slice_mask(0), slice_mask(1), 0 });
//	DisplayMathFunction2D(displayer_slice, string("Computed aberrations map"));
}

void	AberrationsCorrector::Batch()
{

	ComputeAberration();


	/*
	for(size_t sample = 0; sample < n_samples; ++sample)
		{
		AberrationsMap.GetSlice(aberrations_slice, quick_iv(slice_mask(1), slice_mask(0), sample));
		for(size_t el = 0; el < data_source->n_tx_elements; ++el)
			{
			aberrations_slice[el].FilterMedian(3);
			aberrations_slice[el].FilterGauss(3);
			}
		}
	DisplayAberrations("Filtered aberrations map");
	*/
	GUIProgressBar	progress;
	progress.start("Computing rays", n_rays);
	for(size_t ray = 0; ray < n_rays; ++ray)
	{
//		SetCurrentRay(ray);
//		SetAngleUnits(RADIANS);
		physical_angle	angle = CurrentRayAngle(ray);

		TimeTable	aberrations_slice;
		AberrationsMap.GetSlice(aberrations_slice, {ray, slice_mask(0), slice_mask(1)});

		ComputeDelayedSignals(angle, aberrations_slice, amplitude_corrections);

		for(size_t sub = 0; sub < n_frames; ++sub)
		{
			ComplexFunctionF32	current_ray;
			focused_data.GetRow(current_ray, {sub, ray, slice_mask(0)});
//			SetCurrentFrame(frame_no);
//			FocusRay(frame_no, angle);
// 			GatherDelayedSignalsSubaperture(frame_no, CurrentRay);
			GatherDelayedSignals(current_ray, FirstSubapertureElement(sub), LastSubapertureElement(sub));
			DCOffsetCorrection(current_ray);
		}
		++progress;
	}
	progress.end();

}


struct	TuneMagnitude
{
	const double p;

	TuneMagnitude() : p(0.1){}

	complexF32 operator()(const complexF32& x) const
	{
		double	divisor = pow(cabs2(x), (1.-p)/2);
		if(divisor) return x/divisor;
		else return complexF32(0);
	}

	complexF32 operator()(const complexF32 &x, const complexF32&) const
	{
		return operator()(x);
	}
};

void	AberrationsCorrector::AnalyzeDelays(TimeTable &delays, shift_detection_algorithm algorithm)
{
//	algorithm = cross_correlation;

	XRAD_ASSERT_THROW_M(delays.hsize() == n_samples && delays.vsize()==data_source->n_rx_elements, invalid_argument, "Invalid delays array dimansions");

	size_t	correlator_size;
	size_t n_correlation_ranges;
	size_t correlator_step;

//	if(0 && algorithm == phase_detection)
	if(algorithm == phase_detection)
	{
		correlator_size = n_samples;
		n_correlation_ranges = 1;
		correlator_step = correlator_size;
	}
	else
	{
		correlator_size = 256;// только степень 2, нужно длЯ бпф
		correlator_step = correlator_size/2;

		n_correlation_ranges = n_samples/correlator_step - 1;
	}

	RealFunctionF32	slopes_internal;
	slopes_internal.realloc(n_correlation_ranges);

	RealFunctionF32		window_f(correlator_size);
	ComplexFunction2D_F32	added(data_source->n_rx_elements, n_samples);
	ComplexFunction2D_F32	rx_slice_1, rx_slice_2;

	ComplexFunctionF32	c1(correlator_size), c2(correlator_size);


	int	interp_factor = 16;

	ComplexFunctionF32	ci(correlator_size*interp_factor);
	RealFunctionF32	cri(correlator_size*interp_factor);

	//	внутренние задержки считаются в отсчетах "сырых" данных, потом их переводим в сантиметры. но это пока очень неЯсно
	RealFunction2D_F32	correction_delays_in_raw_samples(n_correlation_ranges, data_source->n_rx_elements, 0);
//	CreateWindowFunction(window_f, blackman_nuttall_win); 
	CreateWindowFunction(window_f, cos2_window());

//-----------------------------------------------------------------------------------
//
//	отслеженные варианты:
//
//	1. ищем разность хода между двумЯ соседними элементами приемника,
//	элементы передатчика используютсЯ длЯ дополнительного усреднениЯ.
//	отработано. результат посредственный
//	******была ошибка в принципе, все отменено,перепроверить всЮ!!!
//
//	2. фокусируем луч передатчика, затем находим задержки между 
//	парами соседних элементов приемника.
//	существенно лучше, чем вариант 1, отрабатываем
//
//	3. старый фазовый алгоритм. на малых аберрациЯх (менее 0.5 pi()/el) работает лучше всего

	GUIProgressBar	progress;
	progress.start("Computing aberrations delays", n_correlation_ranges);
	for(size_t n = 0; n < n_correlation_ranges; ++n)
	{
		size_t	s0 = range(n*correlator_step, 0, n_samples-correlator_size);
		for(size_t rx = 1; rx < data_source->n_rx_elements; ++rx)
		{
		//	реализуем обход "битых" элементов
			size_t	el_1 = data_source->PreviousNonDeadElement(rx-1);
			size_t	el_2 = data_source->NextNonDeadElement(rx);

			delayed_signals.GetSlice(rx_slice_1, {el_1, slice_mask(0),slice_mask(1)});
			delayed_signals.GetSlice(rx_slice_2, {el_2, slice_mask(0),slice_mask(1)});

			ComplexFunctionF32	tx_ray_1(correlator_size, complexF32(0)), tx_ray_2(correlator_size, complexF32(0));

			//	здесь фокусируем передатчик, приемник работает парой соседних элементов
			for(size_t tx = 0; tx < data_source->n_tx_elements-1; ++tx)
			{
				ComplexFunctionF32::iterator it1 = rx_slice_1.row(tx).begin();
				ComplexFunctionF32::iterator it2 = rx_slice_2.row(tx).begin();

				copy(it1 + s0, it1 + s0 + correlator_size, c1.begin());
				copy(it2 + s0, it2 + s0 + correlator_size, c2.begin());

				tx_ray_1 += c1;
				tx_ray_2 += c2;
			}

		//	компенсируем офсет (динамически)
			DCOffsetCorrection(tx_ray_1);
			DCOffsetCorrection(tx_ray_2);

			// компенсируем амплитуду, чтобы увеличить вес
			// коррелированных маломощных спекл-сигналов
			ApplyFunction(tx_ray_1, TuneMagnitude());
			ApplyFunction(tx_ray_2, TuneMagnitude());

			// здесь возможны различные методы
			// измерениЯ сдвига


			if(algorithm == phase_detection)
			{
				tx_ray_1 %= tx_ray_2;
				complexF64	value = AverageValue(tx_ray_1);
				if(!is_number(value.re) || !is_number(value.im))
				{
					DisplayMathFunction(tx_ray_1, 0, 1, "product");
					DisplayMathFunction(tx_ray_2, 0, 1, "second argument");
					ShowComplex("bad value", value);
//					ForceDebugBreak();
				}

//				correction_delays_in_raw_samples.at(n,rx) = -2.*arg(value)/(pi()*(el_2 - el_1));
				correction_delays_in_raw_samples.at(n,rx) = 4.*arg(value)/(pi()*(el_2 - el_1));
				// знак поменялся на противоположный после изменения ядра интерполяции преобразования Гильберта

				if(!is_number(correction_delays_in_raw_samples.at(n,rx)))
				{
//					ForceDebugBreak();
					Pause();
				}
			}
			else
			{
				tx_ray_1 *= window_f;
				tx_ray_2 *= window_f;
				FFT(tx_ray_1, ftForward);
				FFT(tx_ray_2, ftForward);

				if(algorithm == cross_correlation)
				{
				// кросс-коррелЯциЯ
					tx_ray_1 %= tx_ray_2;
				}
				else for(size_t k = 0; k < correlator_size; ++k)
				{
				// деконволюциЯ
					if(cabs(tx_ray_2[k])) tx_ray_1[k] /= tx_ray_2[k];
					else tx_ray_1[k] = 0;
				}

				ci.CopyData(tx_ray_1);
				FFTf(ci, fftRevRollAfter);
				if(ControlPressed())
					DisplayMathFunction(ci, -int(correlator_size)/2, 1./interp_factor, "correlation interpolated");
				size_t	max_pos;
				MaxValue(ci, &max_pos);
	//			correction_delays_in_raw_samples.at(n,rx) = -(double(max_pos)/interp_factor - correlator_size/2)/(el_2 - el_1);
				correction_delays_in_raw_samples.at(n,rx) = 2*(double(max_pos)/interp_factor - correlator_size/2)/(el_2 - el_1);
				// знак изменяется после изменения ядра интерполяции преобразования Гильберта
			}

		}

//		correction_delays_in_raw_samples[n].Display(0,1,"delays internal 0");

		++progress;
	}
	progress.end();

	do_once DisplayMathFunction2D(correction_delays_in_raw_samples, "Delays internal");

	if(n_correlation_ranges >= 3) for(size_t i = 1; i < data_source->n_rx_elements; ++i)
	{
		correction_delays_in_raw_samples.col(i).FilterMedian(3);
	}

	for(size_t n = 0; n < n_correlation_ranges; ++n)
	{
		for(size_t i = 1; i < data_source->n_rx_elements; ++i)
		{
			correction_delays_in_raw_samples.at(n,i) += correction_delays_in_raw_samples.at(n,i-1);
		}

		RealVectorF64	a(2);

		DetectLSPolynomUniformGrid(correction_delays_in_raw_samples.row(n), a);
		for(size_t i = 0; i < data_source->n_rx_elements; ++i)
		{
			correction_delays_in_raw_samples.at(n,i) -= (a[0] + a[1]*i);
		}
		slopes_internal[n] = a[1];
		slopes_internal[n] = -std::atan2(slopes_internal[n]*depth_range().mm()/n_samples, array_Pitch.mm());
	}

	slopes.realloc(n_samples);


	const	double	n_raw_samps_per_cm_1_way = data_source->raw_signal_sample_rate.Hz() / sound_speed.cm_sec();

	for(size_t i = 0; i < n_samples; ++i)
	{
		double	x = double(i)/correlator_step;
		slopes[i] = slopes_internal.in(x);
		for(size_t j = 0; j < data_source->n_rx_elements; ++j)
		{
			delays.at(j,i) = cm(correction_delays_in_raw_samples.col(j).in(x) / n_raw_samps_per_cm_1_way)/sound_speed;
		}
	}
}




///-----------------------------------------
// below obsolete






#if 0
void	AberrationsCorrector::Correct_Interval(ComplexFunction2D_F32 *TX_RX_el_Signal)
{
	size_t	interval_no = 0;//from_sample/interval_increment;
	static	RealFunctionF32 TX_Phases(data_source->n_tx_elements), RX_Phases(data_source->n_rx_elements);

	if(interval_no < Intervals_To_Compute_Correction)
	{
		TX_Phases = Compute_TX_Phases(TX_RX_el_Signal);
		RX_Phases = Compute_RX_Phases(TX_RX_el_Signal);
		for(size_t el2 = 0; el2 < data_source->n_tx_elements; el2++)
		{
			RX_Phases[el2] = (RX_Phases[el2] + TX_Phases[el2])/2.;
		}
	}

	for(size_t sample = from_sample; sample < to_sample; sample++)
	{
		for(size_t sub = 0; sub < n_frames; sub++)
		{
			size_t First_Sub_El = sub * n_subaperture_elements_full;
			size_t Last_Sub_El = (sub + 1) * n_subaperture_elements_full;

			SetCurrentFrame(sub);
			for(size_t el2 = First_Sub_El; el2 < Last_Sub_El; el2++)
			{
				for(size_t el1 = 0; el1 < data_source->n_tx_elements; el1++)
				{
					double	Phase = RX_Phases[el1] + RX_Phases[el2];
					complexF32	Factor = polar(1, -Phase);
					complexF32	increment = (Factor*TX_RX_el_Signal[el1][el2][sample-from_sample]);

					CurrentRay[sample] += increment;
				}
			}
		}
	}
}
#endif

complexF32 AberrationsCorrector::F_Correlation(const complexF32 &x, const complexF32 &y)
{
	complexF32 res = complexF32(1);
	complexF32 factor = x % y;
	double	argument = arg(factor);
	int	power;

	factor /= 1.e8;

	Correlations_Count++;
	if(fabs(argument) >= pi() / Correlation_Power)
	{
		Bad_Correlations_Count++;
		factor = 0;
	}

	for(power = 0; power < Correlation_Power; power++)
	{
		res *= factor;
	}
	return	res;
}


RealFunctionF32 &AberrationsCorrector::Compute_RX_Phases(ComplexFunction2D_F32 *TX_RX_el_Signal)
{
	static	RealFunctionF32	Phases(data_source->n_rx_elements);
	static	ComplexFunction2D_F32	RX_Signal(data_source->n_rx_elements, n_samples/*interval_increment*/);
	double		Accumulator;
	static	complexF32	Aver_Correlation, Correlation;

	for(size_t el2 = 0; el2 < data_source->n_rx_elements; el2++)
	{
		for(size_t sample = from_sample; sample < to_sample; sample++)
		{
			RX_Signal.at(el2, sample - from_sample) = 0;
			for(size_t el1 = 0; el1 < data_source->n_tx_elements; el1++)
			{
				RX_Signal.at(el2, sample - from_sample) +=
					TX_RX_el_Signal[el1].at(el2, sample - from_sample);
			}
		}
	}
	for(size_t el2 = 1; el2 < data_source->n_rx_elements; el2++)
	{
		Aver_Correlation = 0;
		for(size_t sample = from_sample; sample < to_sample; sample++)
		{
			Correlation = F_Correlation(
				RX_Signal.at(el2-1, sample - from_sample),
				RX_Signal.at(el2, sample - from_sample));
			Aver_Correlation += Correlation;
		}
		Phases[el2] = -arg(Aver_Correlation);
	}

	Accumulator = 0;
	for(size_t el2 = 1; el2 < data_source->n_rx_elements; el2++)
	{
		Accumulator += Phases[el2];
	}
	Accumulator /= (data_source->n_rx_elements-1);
	for(size_t el2 = 1; el2 < data_source->n_rx_elements; el2++)
	{
		Phases[el2] += (Phases[el2 - 1] - Accumulator);
	}
	Accumulator = 0;
	for(size_t el2 = 0; el2 < data_source->n_rx_elements; el2++)
	{
		Accumulator += Phases[el2];
	}
	Accumulator /= data_source->n_rx_elements;
	for(size_t el2 = 0; el2 < data_source->n_rx_elements; el2++)
	{
		Phases[el2] -= Accumulator;
		Phases[el2] /= Correlation_Power;
	}
	return	Phases;
}

RealFunctionF32 &AberrationsCorrector::Compute_TX_Phases(ComplexFunction2D_F32 *TX_RX_el_Signal)
{
	static	RealFunctionF32	Phases(data_source->n_tx_elements);
	static	ComplexFunction2D_F32	TX_Signal(data_source->n_tx_elements, n_samples/*interval_increment*/);
	double		Accumulator;
	complexF32	Aver_Correlation, Correlation;

	for(size_t el1 = 0; el1 < data_source->n_tx_elements; el1++)
	{
		for(size_t sample = from_sample; sample < to_sample; sample++)
		{
			TX_Signal.at(el1, sample - from_sample) = 0;
			for(size_t el2 = 0; el2 < data_source->n_rx_elements; el2++)
			{
				TX_Signal.at(el1, sample - from_sample) +=
					TX_RX_el_Signal[el1].at(el2, sample - from_sample);
			}
		}
	}

	for(size_t el1 = 1; el1 < data_source->n_tx_elements; el1++)
	{
		Aver_Correlation = 0;
		for(size_t sample = from_sample; sample < to_sample; sample++)
		{
			Correlation = F_Correlation(
				TX_Signal.at(el1-1, sample - from_sample),
				TX_Signal.at(el1, sample - from_sample));
			Aver_Correlation = Correlation;
		}
		Phases[el1] = -arg(Aver_Correlation);
	}
//----------------------------------------------------
// коррекциЯ битого элемента -- в решетке, использовавшейсЯ
// при отладке алгоритма, их de facto было 2.
// в финальной версии никаких битых элементов нет, эти строки
// из окончательной сборки изымаютсЯ
	Phases[11] = Phases[10] + (Phases[13] - Phases[10])/3.;
	Phases[12] =  Phases[10] + 2. *(Phases[13] - Phases[10])/3.;
	//----------------------------------------------------

	Accumulator = 0;
	for(size_t el1 = 1; el1 < data_source->n_tx_elements; el1++)
	{
		Accumulator += Phases[el1];
	}
	Accumulator /= (data_source->n_tx_elements-1);
	for(size_t el1 = 1; el1 < data_source->n_tx_elements; el1++)
	{
		Phases[el1] += (Phases[el1 - 1] - Accumulator);
	}
	Accumulator = 0;
	for(size_t el1 = 0; el1 < data_source->n_tx_elements; el1++)
	{
		Accumulator += Phases[el1];
	}
	Accumulator /= data_source->n_tx_elements;
	for(size_t el1 = 0; el1 < data_source->n_tx_elements; el1++)
	{
		Phases[el1] -= Accumulator;
		Phases[el1]  /= Correlation_Power;
	}
	return	Phases;
}

XRAD_END
