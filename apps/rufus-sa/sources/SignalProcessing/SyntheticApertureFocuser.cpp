#include "pre.h"
#include "SyntheticApertureFocuser.h"
#include "BeamformerTesting.h"
#include <Attic/CalculateEntropy.h>
#include <XRADSystem/Sources/TextFile/text_file.h>

XRAD_BEGIN

SyntheticApertureFocuser::SyntheticApertureFocuser()
{
}

SyntheticApertureFocuser :: ~SyntheticApertureFocuser()
{
}


void	SyntheticApertureFocuser::InitWork()
{
	Modify_Options();

//	AllocateSectorData();
 	focused_data.realloc({n_frames, n_rays, n_samples}, complexF32(0));

	strcpy(SIMIO::Process_Name, "Rays focuser");
	delayed_signals.realloc({data_source->n_tx_elements, data_source->n_rx_elements, n_samples});


//	SetOutputFileName(data_source->GetFileName(), "focused");

	ComputeApodization();


	strcpy(SIMIO::Data_Complexity, ANALYTIC);

	// задаем шаг решетки в отсчетах
	XRAD_ASSERT_THROW_M(data_source->n_tx_elements == data_source->n_rx_elements, logic_error, "Invalid array configuration");

	elements_offsets.realloc(data_source->n_tx_elements);
	TX_Delays.realloc(data_source->n_tx_elements, n_samples);
	RX_Delays.realloc(data_source->n_rx_elements, n_samples);

	ComputeElementsOffsets();

//	ISignal_Write();
}


void	SyntheticApertureFocuser::EndWork()
{
	try
	{
		Write_Data();
	}
	catch(...)
	{
		Error(GetExceptionString());
	}
	Display("Focused signal");
}



void	SyntheticApertureFocuser::ComputeElementsOffsets()
{
	double	aperture_center = double(data_source->n_tx_elements - 1)/2;

	for(size_t el = 0; el < data_source->n_tx_elements; el++)
	{
		double	merged_el = el-el%n_merged_elements;
		elements_offsets[el] = (merged_el - aperture_center)*data_source->array_pitch;
	}
}


void	SyntheticApertureFocuser::ComputeCorrections(physical_angle angle, TimeTable &correction_delays, RealFunction2D_F64	&amplitude_corrections)
{
	correction_delays.fill(sec(0));
	amplitude_corrections.fill(1);

	double	aberration_profile_offset_factor = (data_source->aberrator_thickness/data_source->array_pitch)/cosine(angle);
	TimeVector	aberrations_moved(data_source->n_tx_elements, sec(0));
	RealFunctionF64 amplitude_moved(data_source->n_tx_elements, 1);
	ptrdiff_t	roll_distance = tangent(angle - data_source->calibrator_angle)*aberration_profile_offset_factor;

	for(ptrdiff_t i = 0; i < ptrdiff_t(data_source->n_tx_elements); ++i)
	{
		ptrdiff_t	j = range(i+roll_distance, 0, data_source->n_tx_elements-1);
		if(data_source->aberration_profile.size() == data_source->n_tx_elements) aberrations_moved[i] = data_source->aberration_profile[j];
		if(data_source->amplitude_correction.size() == data_source->n_tx_elements) amplitude_moved[i] = data_source->amplitude_correction[j];// амплитудная коррекци без учета толщины и угла направления
	}

	double	center = data_source->n_tx_elements/2;
	physical_length	ds = data_source->sound_speed / data_source->raw_signal_sample_rate;

	for(size_t element = 0; element < data_source->n_tx_elements; ++element)
	{
		for(size_t sample = 0; sample < n_samples; ++sample)
		{
			physical_length	r = (data_source->first_raw_sample + sample)*ds;
			double	factor = (r - data_source->aberrator_thickness/cosine(angle))/r;
			double	el = center + (element - center)*factor;// это коррекция фокусировки ближней зоны, пока неясно, есть ли от нее толк
				
			correction_delays.at(element, sample) += aberrations_moved.in(el, &interpolators::linear);
			amplitude_corrections.at(element, sample) /= amplitude_moved.in(el);//в амплитудную коррекцию вносится поправка на толщину аберратора и угла наклона луча

		/*if(angle.radians()<0) */amplitude_corrections.at(element, sample) *= gauss((element-center)*data_source->array_pitch.mm(), 0.1*r.mm());// динамическая аподизация (апертура уменьшается в ближней зоне)
		}
	}
	if(ControlPressed()) DisplayMathFunction2D(amplitude_corrections, "Amplitude corrections");
}


void	SyntheticApertureFocuser::FocusSubapertures(physical_angle start_angle, physical_angle end_angle)
{
	// фокусирует раздельно по субапертурам. каждый кадр массива focused_data соответствует одной субапертуре
	// не следует смешивать с функцией FocusWholeAperture из-за особенностей вложения циклов
	GUIProgressBar	progress;
	progress.start("Computing rays", n_rays);
	for(size_t ray = 0; ray < n_rays; ++ray)
	{
		physical_angle	angle = start_angle + ray*(end_angle-start_angle)/n_rays;

		TimeTable	correction_delays(data_source->n_tx_elements, n_samples, sec(0));
		RealFunction2D_F64	amplitude_corrections(data_source->n_tx_elements, n_samples, 1);

		ComputeCorrections(angle, correction_delays, amplitude_corrections);
		ComputeDelayedSignals(angle, correction_delays, amplitude_corrections);

		for(size_t sub = 0; sub < n_frames; ++sub)
		{
			ComplexFunctionF32	current_ray;
			focused_data.GetRow(current_ray, {sub, ray, slice_mask(0)});

			GatherDelayedSignals(current_ray, FirstSubapertureElement(sub), LastSubapertureElement(sub));
		}
		++progress;
	}
	progress.end();
}



void	SyntheticApertureFocuser::FocusWholeAperture(size_t frame_no, physical_angle start_angle, physical_angle end_angle)
{

	TimeTable	correction_delays(data_source->n_tx_elements, n_samples, sec(0));
	RealFunction2D_F64	amplitude_corrections(data_source->n_tx_elements, n_samples, 1);

	for(size_t ray = 0; ray < n_rays; ++ray)
	{
		physical_angle	angle = start_angle + ray*(end_angle - start_angle)/n_rays;
		ComplexFunctionF32 ray_buffer;
		focused_data.GetRow(ray_buffer, {frame_no, ray, slice_mask(0)});

		ComputeCorrections(angle, correction_delays, amplitude_corrections);
		ComputeDelayedSignals(angle, correction_delays, amplitude_corrections);
		GatherDelayedSignals(ray_buffer, 0, data_source->n_rx_elements);
	}
}


//#error see comment
// процедуру поиска уточняем следующим образом.
// 1. скорость звука постоянна, меняем только параболическую
// задержку на апертуре. максимум энтропии ищем отдельно для
// трех поддиапазонов (ближний, средний, дальный)
//
// 2. если максимумы всех трех совпадают, исправляется параболическая аберрация, и всЮ.
// 3. иначе: вводится постоянная параболическая задержка, и с ней уже проводится подбор скорости звука.
//
//   	для печени с аберратором такая задержка была найдена на 14 шаге ныне действующей процедуры поиска.
//	ввести ее руками и пустить поиск по скорости звука.

void	SyntheticApertureFocuser::AnalyzeFocusingDefect()
{
	physical_speed	start_sound_speed = km_sec(1.686);//sound_speed*0.8;
		// стартовую скорость выбираем в стороне, посмотрим, что найдет
	physical_speed	d_sound_speed = km_sec(0.125);
	physical_speed	current_sound_speed(start_sound_speed);
		// стартовую скорость выбираем в стороне, посмотрим, что найдет

	int	n_analyze_rays = GetSigned("How many rays", 10, 5, 128);
	double	angle_factor = 0.5*GetFloating("Sector width", 20, 10, 90)/n_analyze_rays;

	physical_angle	start_analyze_angle = degrees(-angle_factor*n_analyze_rays);
	physical_angle	end_analyze_angle = degrees(angle_factor*n_analyze_rays);
}

void	SyntheticApertureFocuser::FocusFramesWithFocusingDefect()
{
	GUIProgressBar	progress;
	progress.start("Computing rays", n_frames);

	physical_speed	sound_speed_0 = km_sec(GetFloating("Min. sound speed (km/s)", sound_speed.km_sec(), sound_speed.km_sec()/n_frames, sound_speed.km_sec())); // начальная скорость звука
	physical_speed	d_sound_speed = (km_sec(GetFloating("Max. sound speed (km/s)", sound_speed.km_sec(), sound_speed.km_sec()/n_frames, 2*sound_speed.km_sec())) - sound_speed_0)/ (n_frames-1);

	physical_length	parabolic_defect_0 = cm(-0.05);
	physical_length	dpd = -parabolic_defect_0*2./n_frames;

	for(size_t frame_no = 0; frame_no < n_frames; ++frame_no)
	{
		data_source->sound_speed = sound_speed_0 + frame_no*d_sound_speed;
		FocusWholeAperture(frame_no, start_angle(), end_angle());

		++progress;
	}
	progress.end();
}


void	SyntheticApertureFocuser::Batch()
{
	if(frame_sense == fs_focus)
	{
		if(Decide2("Choose focusing task", "Record multifocus", "Iterative procedure", 0))
			AnalyzeFocusingDefect();
		else
			FocusFramesWithFocusingDefect();
	}
	else FocusSubapertures(start_angle(), end_angle());
}



void	SyntheticApertureFocuser::ComputeApodization()
{
	apodization.realloc(data_source->n_tx_elements, data_source->n_rx_elements, 1.0);
	if(!(apodization_tx || apodization_rx)) return;

	RealFunctionF32	ap(data_source->n_tx_elements);
//	int answer = Get_Function(ap, "Apodization function");

	CreateWindowFunction(ap, hamming_window());
	for(size_t i = 0; i < data_source->n_tx_elements; i++)
	{
		for(size_t j = 0; j < data_source->n_rx_elements; j++)
		{
			if(apodization_tx) apodization.at(i,j) *= ap[i];
			if(apodization_rx) apodization.at(i,j) *= ap[j];
		}
	}
}




void	CancelMagnitude(ComplexFunctionF32 &f)
{
	ComplexFunctionF32::iterator it = f.begin(), ie = f.end();
	for(; it < ie; ++it)
	{
		double	m = cabs(*it);
		if(m) *it /= m;
	}
}


physical_length SyntheticApertureFocuser::ComputeLens(physical_length element_x, physical_length focus_r, double target_sine, double target_cosine) const
{
	physical_length focus_x = focus_r*target_sine;

	//	формула задержек для динамической фокусировки до 27.11.2012
	//	эталонный расчет, который следует всегда иметь в виду, поэтому должен оставаться перед глазами
	//	delays[i] = cm(sqrt(square(target_x.cm()-element_x.cm()) + square(focus_z.cm())));
	//	для фиксированного фокуса:
	//	delays[i] = cm(sqrt(square(focus_x.cm()-element_x.cm()) + square(focus_z.cm())));



	if(focus_r.cm() == 0) return element_x*target_sine;

	physical_length focus_z = focus_r*target_cosine;

	if(focus_r.cm() > 0)
	{
		return hypot(focus_x-element_x, focus_z) - focus_r;
	}
	else // фокусное расстояние может быть и отрицательным (засветка широкой области расходящимся лучом)
	{
		return -(hypot(focus_x-element_x, focus_z) + focus_r);
	}
}

void	SyntheticApertureFocuser::ComputePathDifferences(TimeTable &delays, physical_angle angle, const aperture_focusing &focus)
{
	double	direction_sine = sine(angle);
	double	direction_cosine = cosine(angle);

	physical_length	r0 = r_min();
	physical_length	dr = depth_range()/n_samples;

	for(size_t sample = 0; sample < n_samples; sample++)
	{
		physical_length	target_r = r0 + dr*sample;
		physical_length	target_x = target_r*direction_sine;
		physical_length	target_z = target_r*direction_cosine;

		physical_length	focus_r;
		physical_length	focus_x;
		physical_length	focus_z;

		if(!focus.size())
		{
			focus_r = target_r;
			focus_x = target_x;
			focus_z = target_z;
		}
		else
		{
			focus_r = focus[0];
			focus_x = focus[0]*direction_sine;
			focus_z = focus[0]*direction_cosine;
		}



		physical_time	target_t = target_r/data_source->sound_speed;

		for(size_t i = 0; i < delays.vsize(); i++)
		{
			physical_length	element_x = elements_offsets[i];


		#if 0
			physical_length	lens = ComputeLens(element_x, focus_r, direction_sine, direction_cosine);
			delays.at(i,sample) = (target_r + lens)/data_source->sound_speed;
		#else
			physical_length	lens = (0.5*element_x*element_x/focus_r);
			physical_length	slope = -element_x*direction_sine;
			delays.at(i,sample) = (target_r + lens + slope)/data_source->sound_speed;
		#endif

			// дефект скорости звука, вообще говоря, влияет на все три слагаемых в случае фазированной решетки.
			// в случае линейного сканирования только на первых два (третье равно нулю, сканирование не по углу).
			// однако рассмотрим пока только влияние на линзу
		}
	}
}


void	SyntheticApertureFocuser::ComputeDelayedSignals(physical_angle angle, const TimeTable &correction_delays, const RealFunction2D_F32 &amplitude_corrections)
{
	physical_angle	direction_tx, direction_rx;

	direction_rx = angle + sector_centre_angle;
	if(TX_Focus.static_direction) direction_tx = sector_centre_angle;
	else direction_tx = direction_rx;

	ComputePathDifferences(TX_Delays, direction_tx, TX_Focus);
	ComputePathDifferences(RX_Delays, direction_rx, RX_Focus);


	TX_Delays += correction_delays;
	RX_Delays += correction_delays;

//#pragma message где-то здесь вносить динамическую аподизацию

	RealFunctionMD_F32	delays({data_source->n_tx_elements, data_source->n_rx_elements, n_samples});

	#pragma omp parallel for schedule (guided)
	for(ptrdiff_t el_tx = 0; el_tx < ptrdiff_t(data_source->n_tx_elements); ++el_tx)
	{
		for(size_t el_rx = 0; el_rx < data_source->n_rx_elements; ++el_rx)
		{
			auto	row = data_source->GetRow(el_tx,el_rx);
			ComplexFunctionF32	delayed_waveform;
			delayed_signals.GetRow(delayed_waveform, {size_t(el_tx), el_rx, slice_mask(0)});

			double	apodization_factor = apodization.at(el_tx,el_rx);
			ComplexFunctionF32::iterator dwf = delayed_waveform.begin();

			auto	txd = TX_Delays.row(el_tx).begin();
			auto	rxd = RX_Delays.row(el_rx).begin();

			RealFunctionF32	delays_row = delays.GetRow({size_t(el_tx), el_rx, slice_mask(0)});

			for(int sample = 0; sample < int(n_samples); ++sample, ++dwf, ++txd, ++rxd)
			{
				double	raw_sample = (*txd + *rxd) * data_source->raw_signal_sample_rate - data_source->first_raw_sample;
				double	amp_corr = amplitude_corrections.at(el_tx,sample)*amplitude_corrections.at(el_rx,sample);
				delays_row[sample] = raw_sample;
				*dwf = row->in(raw_sample)*apodization_factor*amp_corr;
			}
		}
	}
	if(0)
	{
		// запись задержек для анализа
		wstring	path = L"c:/temp/delays";
		wstring	filename = path + ssprintf(L"/%+06.2f_deg", angle.degrees());

		CreatePath(path);
		shared_cfile	delays_file(filename + L".dat", L"wb");
		text_file_writer delays_file_header(path + L"/info.txt", text_encoding::char_8bit);

		delays_file_header.printf_("n_samples_per_direction = %zu\nn_tx_elements = %zu\nn_rx_elements = %zu\nsample_rate = %g MHz", n_samples, data_source->n_tx_elements, data_source->n_rx_elements, sample_rate.MHz());


		for(size_t sample = 0; sample < n_samples; ++sample)
		{
			auto	slice = delays.GetSlice({slice_mask(0), slice_mask(1), sample});
			delays_file.write_numbers(slice, ioF32_LE);
		}


// 		for(size_t el_tx = 0; el_tx < data_source->n_tx_elements; ++el_tx)
// 		{
// 			auto	slice = delays.GetSlice({el_tx, slice_mask(0), slice_mask(1)});
// 			delays_file.write_numbers(slice, ioF32_LE);
// 		}
	}
	if(CapsLock())
	{
		ComplexFunction2D_F32 displayer_total(data_source->n_tx_elements*data_source->n_rx_elements, n_samples);

		for(size_t el = 0; el < data_source->n_tx_elements; ++el)
		{
			ComplexFunction2D_F32 display_slice;
			delayed_signals.GetSlice(display_slice, {el, slice_mask(0), slice_mask(1)});
			display_slice.PutDataSegment(displayer_total, el*data_source->n_tx_elements, 0);
		}

		DisplayMathFunction2D(displayer_total, "Delayed waveform", ScanFrameRectangle(cm(30), cm(10)));
	}
}

size_t	SyntheticApertureFocuser::FirstSubapertureElement(size_t sub) const
{
	size_t	left = n_subaperture_elements_full*sub;
	size_t	centre = n_subaperture_elements_full*sub + n_subaperture_elements_full/2;

	if(n_subaperture_elements_active==1) return centre;
	else return left;
}

size_t	SyntheticApertureFocuser::LastSubapertureElement(size_t sub) const
{
	return FirstSubapertureElement(sub) + n_subaperture_elements_active;
}

void	SyntheticApertureFocuser::GatherDelayedSignals(ComplexFunctionF32 &ray_buffer, size_t first_rx_el, size_t last_rx_el)
{
	ray_buffer.fill(complexF32(0));

	for(size_t tx_el = 0; tx_el < data_source->n_tx_elements;tx_el++)
	{
		for(size_t rx_el = first_rx_el; rx_el < last_rx_el; rx_el++)
		{
			ComplexFunctionF32	delayed_waveform;
			delayed_signals.GetRow(delayed_waveform, {tx_el, rx_el, slice_mask(0)});
			ray_buffer += delayed_waveform;
		}
	}
	if(!cancel_dc_offset_correction) DCOffsetCorrection(ray_buffer);
}



void	SyntheticApertureFocuser::DCOffsetCorrection(ComplexFunctionF32 &signal)
{
	ComplexFunctionF32	f(signal);
	double	filter_length = sample_rate/MHz(2);
	f.FilterGauss(filter_length);
	signal -= f;

}


XRAD_END
