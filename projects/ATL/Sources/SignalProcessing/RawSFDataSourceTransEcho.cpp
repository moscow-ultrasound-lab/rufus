#include "pre.h"
#include <XRADBasic/DataArrayIO.h>
#include <XRADSystem/Sources/TextFile/text_file.h>

#include "RawSFDataSourceTransEcho.h"
#include "SyntheticApertureFocuserAina.h"


/*!
 * \file RawSFDataSourceTransEcho.cpp
 * \date 2017/06/23 10:57
 *
 * \author kulberg
 *
 * \brief 
 *
 * TODO: long description
 *
 * \note
*/

XRAD_BEGIN

// Version=2
// SampleRate=10000000
// ProbeName=3.0S19
// NumOfElements=64
// ProbeType=Array
// Width=18.270
// Angle=1.571
// IsPencil=0
// SamplesPerLine=2338


void	PrefilterCalibrationPulses(ComplexFunction2D_F32 &calibration_pulses)
{
	size_t	samples_per_element = calibration_pulses.hsize();
	size_t	n_tx_elements = calibration_pulses.vsize();
	size_t	fft_size = ceil_fft_length(samples_per_element);
	ComplexFunctionF32	buffer(fft_size);
	RealFunctionF32	accumulator(buffer.size(), 0);

	for(size_t i = 0; i < n_tx_elements; ++i)
	{
		ApplyWindowFunction(calibration_pulses.row(i), e_nuttall_window);
		buffer.CopyData(calibration_pulses.row(i));
		FFTf(buffer, fftFwd|fftRollAfter);
		ApplyFunction(buffer, cabs_functor<float, complexF32>());
		accumulator += real(buffer);
	}
//	DisplayMathFunction(accumulator, 0, 10./fft_size, "Spectrum for band narrowing");
	size_t	maxpos;
	MaxValue(accumulator, &maxpos);
	size_t	windowsize = 256;
	RealFunctionF64	window(windowsize, 1);
	ApplyWindowFunction(window, e_nuttall_window);
	window.resize(fft_size);
	window.roll(ptrdiff_t(maxpos) - windowsize/2);

//	DisplayMathFunction(window, 0, 10./fft_size, "Полосовой фильтр");

	for(size_t i = 0; i < n_tx_elements; ++i)
	{
		buffer.CopyData(calibration_pulses.row(i));
		FFTf(buffer, fftFwd|fftRollAfter);
		buffer *= window;
		FFTf(buffer, fftRev|fftRollBefore);
		calibration_pulses.row(i).CopyData(buffer);
	}

}

void	RawSFDataSourceTransEcho::LoadInfFile(string ini_filename)
{
	try
	{
		string	inf_ext = ".inf";
		std::copy(inf_ext.rbegin(), inf_ext.rend(), ini_filename.rbegin());
		text_file_reader inf_file;

		inf_file.open(ini_filename);

		float	buffer;
		inf_file.scanf_("Version=%f\n", &buffer);//ignored
		inf_file.scanf_("SampleRate=%f\n", &buffer);
		raw_signal_sample_rate = Hz(buffer);

		inf_file.scanf_("ProbeName=%fS19\n", &buffer);//ignored
		inf_file.scanf_("NumOfElements=%f\n", &buffer);

		n_rx_elements = int(buffer);
		n_tx_elements = int(buffer);

		inf_file.scanf_("ProbeType=Array\n", &buffer);
		inf_file.scanf_("Width=%f\n", &buffer);
		//array_pitch = mm(18.27/(n_tx_elements-1));
		array_pitch = mm(18.27 / (n_tx_elements));

		inf_file.scanf_("Angle=%f\n", &buffer);//ignored
		inf_file.scanf_("IsPencil=%f\n", &buffer);//ignored

		inf_file.scanf_("SamplesPerLine=%f\n", &buffer);
		samples_per_element = int(buffer);
	}
	catch(quit_application&){ throw; }
	catch(canceled_operation&){ throw; }
	catch(...)
	{
		n_rx_elements = 64;
		n_tx_elements = 64;
		raw_signal_sample_rate = MHz(10);
		array_pitch = mm(18.27/(n_tx_elements-1));
		samples_per_element = 2338;
	}

}
/*
 Важные соображения по поводу разбора калибровочных импульсов и определения аберраций

 1. Ищется "полоска" в сигнале калибровочного источника.
 Сейчас поиск сделан "на коленке", нужно сделать более точно с помощью преобразования Хафа или Радона.

 2. Такие же полоски присутствуют и на обычных (не трансмиссионных) данных. Когда появится нормальный алгоритм,
 попробовать искать их и также делать коррекцию.

 */

void	PreprocessCalibrationPulses(ComplexFunction2D_F32 &calibration_pulses)
{
	// находит пиковые значения в каждой строке, 
	size_t	n_tx_elements = calibration_pulses.vsize();
	size_t	samples_per_element = calibration_pulses.hsize();

	for(size_t i = 0; i < samples_per_element; ++i)
	{
		calibration_pulses.col(i).FilterGauss(2);
	}

//	DisplayMathFunction2D(calibration_pulses, "Калибровочный импульс, 1 ступенька шумодава");

	size_t	window_size(64);
	size_t	wide_window_size(128);
	RealFunctionF32	window_pattern(window_size, 1);
	RealFunctionF32	wide_window(wide_window_size, 1);
	ApplyWindowFunction(window_pattern, e_blackman_nuttall_window);
	ApplyWindowFunction(wide_window, e_blackman_nuttall_window);
	wide_window.resize(samples_per_element);

	RealFunctionF64	average(samples_per_element, 0);
	RealFunctionF64	inc(samples_per_element, 0);

	for(size_t i = 0; i < n_tx_elements; ++i)
	{
		CopyData(inc, calibration_pulses.row(i), [](double &r,const complexF32 &v) { r = cabs2(v); });
		average += inc;
	}

	//вводится экспоненциальное убывание, чтобы не ловить шум в дальней зоне
	double	factor = 1;
	double	max_attenuation_factor = 4;//максимальный коэффициент ослабления сигнала.
	double	dfactor = pow(max_attenuation_factor, -1./samples_per_element);//накопительный множитель max_attenuation_factor^(-1/n_samples)
	for(auto &a: average){ a *= (factor*=dfactor); }

	size_t	max_position;
	MaxValue(average, &max_position);
//	DisplayMathFunction(average, 0, 1, "Среднее арифметическое всех импульсов (смотрим, хорошо ли выделяется пик)");
	wide_window.roll(ptrdiff_t(max_position - wide_window_size/2));

//	DisplayMathFunction(window_pattern, 0, 1, "window");
	RealFunction<size_t, int> max_positions(n_tx_elements);

	for(size_t i = 0; i < n_tx_elements; ++i)
	{
		calibration_pulses.row(i) *= wide_window;
		MaxValue(calibration_pulses.row(i), &max_positions[i]);
	}
//	DisplayMathFunction2D(calibration_pulses, "Калибровочные импульсы, подавлен шум вдали от главного пика");
	/*
	GraphSet	maxposgraph("Положение максимумов калибровочных импульсов", "Отсчет", "Элемент");
	maxposgraph.AddGraphUniform(max_positions, 0, 1, "before filter");
	max_positions.FilterMedian(7);
	maxposgraph.AddGraphUniform(max_positions, 0, 1, "after median filter");
	maxposgraph.Display(false);
	*/

	for(size_t i = 0; i < n_tx_elements; ++i)
	{
		RealFunctionF32	window(window_pattern);
		window.resize(samples_per_element);

		window.roll(ptrdiff_t(max_positions[i])-window_size/2);

		calibration_pulses.row(i) *= window;
	}
//	DisplayMathFunction2D(calibration_pulses, "Калибровочные импульсы, самое узкое окно");
	PrefilterCalibrationPulses(calibration_pulses);
}

// struct s1{};
// 
// struct s2 : s1{};
// 
// struct myclass
// {
// 	s1 *a;
// 	s1 *b;
// };
// 
// void	f()
// {
// 	myclass	c;
// 	auto a = c.a;
// 	auto a1 = &c.a;
// 	s1 *xrad::myclass:: *pa = &myclass::a;
// 	auto ac = c.*pa;
// 	myclass b;
// 	auto ab = b.*pa;
// 	pa = &myclass::b;
// 	auto bc = c.*pa;
// 
// }

void	RawSFDataSourceTransEcho::ComputeOffsetsCrossCorrelation()
{
	RealFunctionF32 offsets(n_tx_elements, 0);
	amplitude_correction.realloc(n_tx_elements, 0);

	ComplexFunctionF32	buffer(ceil_fft_length(samples_per_element));
	RealFunctionF32	accumulator(buffer.size(), 0);

	for(size_t i = 0; i < n_tx_elements; ++i)
	{
		amplitude_correction[i] = ElementSumTransformed(calibration_pulses.row(i), Functors::absolute_value());
	}
	//DisplayMathFunction2D(calibration_pulses, "calibration_pulses");
	//DisplayMathFunction(amplitude_correction, 0, 1, "Amplitude corrections", false);
	amplitude_correction /= MaxValue(amplitude_correction);
	//DisplayMathFunction(amplitude_correction, 0, 1, "Amplitude corrections", false);
//	if(!YesOrNo("Использовать амплитудную коррекцию?", saved_default_value))
	{
		amplitude_correction.fill(1);
	}
	//DisplayMathFunction(amplitude_correction, 0, 1, "Amplitude corrections", false);
	RealFunctionF64	maximum_finder(calibration_pulses.hsize(), 0);
	for(size_t i = 0; i < n_tx_elements-1; ++i)
	{
		buffer.CopyData(calibration_pulses.row(i));
		FFTf(buffer, fftFwd|fftRollAfter);
		ApplyFunction(buffer, cabs_functor<float, complexF32>());
		//if (CapsLock) { DisplayMathFunction(buffer, 0, 1, "buffer"); }
		accumulator += real(buffer);

		offsets[i] = arg(sp(calibration_pulses.row(i), calibration_pulses.row(i+1)));
		Apply_AA_1D_F2(maximum_finder, calibration_pulses.row(i), [](double &y, const complexF32 &x){y += cabs2(x);});
	}
	offsets *= 180 / pi();
//	DisplayMathFunction(offsets, 0, 1, "Разность фаз между элементами (град)");
	aberration_profile_function.MakeCopy(offsets);
#if 1

	//DisplayMathFunction(maximum_finder, 0, 1, "maximum_finder");
	//DisplayMathFunction(accumulator, 0, 1, "accumulator");
	size_t	max_position(0);
	MaxValue(maximum_finder, &max_position);
	auto	distance_to_calibrator = max_position * sound_speed / raw_signal_sample_rate;
	//ShowFloating("Расстояние до калибратора (см)", distance_to_calibrator.cm());

	DataArray<double>	moments(1);
	WeightMoments(accumulator, moments, true);
	physical_frequency	df = raw_signal_sample_rate/accumulator.size();//samples_per_element
	physical_speed	c = cm_sec(1.54e5);
	physical_frequency	f0 = moments[0]*df;

	physical_length	lambda = c/f0;


	offsets *= lambda.mm()/two_pi();
	offsets *= 2;//эмпирическая добавка
	// теперь это вроде бы разность хода в мм
//	DisplayMathFunction(offsets, 0, 1, "Разность хода между элементами (мм)");

	//уничтожаем линейную составляющую аберрации и находим из нее направление угла на калибровочный элемент перед вычитанием
	double	slope = AverageValue(offsets);//mm per element
	offsets -= slope;
	calibrator_angle = -radians(std::atan2(slope, array_pitch.mm()));


	
	// интегрируем
	for(size_t i = 1; i < n_tx_elements; ++i)
	{
		offsets[i]+=offsets[i-1];
	}

//	DisplayMathFunction(offsets, 0, 1, "Профиль аберрации до исправления параболы (мм)", false);

	// Уничтожаем параболическую составляющую коррекции, вызванную близостью калибровочного источникат ц
	for(size_t i = 0; i < n_tx_elements; ++i)
	{
		double	x_mm = (double(i)-n_tx_elements/2)*array_pitch.mm();

// 		offsets[i] -= 0.25*square(x_mm)/distance_to_calibrator.mm();//0,25 включает 0,5, обословленную формулой, и эмпирическое деление на 2 20 строками выше. Но, видимо, нет в этом нужды
		offsets[i] -= 0.5*square(x_mm)/distance_to_calibrator.mm();
	}
	
	aberration_profile.MakeCopy(offsets, [c](physical_time &t, const float &x){return t=mm(x)/c;});
//	DisplayMathFunction(offsets, 0, 1, "Профиль аберрации (мм)");
	aberration_profile_function.MakeCopy(offsets);
	//aberration_profile_function *= pi() / lambda.mm();
	//aberration_profile_function /= f0.Hz();
	//aberration_profile_function /= 180;
	aberration_profile_function /= lambda.mm() * f0.Hz() * 180 / pi();
	aberration_profile_function -= AverageValue(aberration_profile_function);
	

	/*	

	RealFunctionF64 beacon_offsets(offsets);
	beacon_offsets /= 1000;
	beacon_offsets /= 1540;
	DisplayMathFunction(beacon_offsets, 0, 1, "Корректирующая функция (cекунду)");
	int unique_name(64);
	RealFunction2D_F64 matrix_correction(128, 64, 0);
	RealFunctionF64 rays_corrected(128, 0), line_original(128, 0);
	matrix_correction.row(unique_name) = beacon_offsets;
	rays_corrected[unique_name]=unique_name;
	line_original = GetLine(300, 400, 5, matrix_correction, rays_corrected);
	DisplayMathFunction(line_original, 0, 1, "line для маяка");
*/

//	DisplayMathFunction(aberration_profile, 0, 1, "Профиль аберрации (мм)");
//	aberrator_thickness = mm(GetFloating("Aberrator thickness (mm)", 6, 0, infinity()));
	aberrator_thickness = mm(6);
#endif
}
template<class T> void f(T &&x){ ++x; }
//void g(const float &x){ f(x); }
void h(float &x){ f(x); }

void	RawSFDataSourceTransEcho::LoadCalibrationPulses()
{
	try
	{
		do
		{
			//string	calibration_filename = GetFileNameRead("TransEcho calibration file", SavedGUIValue("*.raw"));
			string	calibration_filename = GetFileNameRead("TransEcho calibration file", SavedGUIValue("*.raw"), "*.raw");

			shared_cfile	calibration_file(calibration_filename, "rb");

			calibration_pulses.realloc(n_rx_elements, samples_per_element);


			for(size_t i = 0; i < n_tx_elements; ++i)
			{
				calibration_file.read_numbers(calibration_pulses.row(i), ioComplexI32_LE);
			}
			double	maxval = cabs(MaxValue(calibration_pulses));
			double	average = sqrt(AverageValueTransformed(calibration_pulses, cabs2_functor<float, complexF32>()));
			if (YesOrNo("Display calibration signal?", MakeGUIValue(false, saved_default_value))) { DisplayMathFunction2D(calibration_pulses, ssprintf(L"Калибровочный сигнал, макс.=%g, среднее=%g", maxval, average)); }
		} 
		//while(!YesOrNo("Использовать этот файл для исправления фокусировки?", saved_default_value));
		while (!true);

		PreprocessCalibrationPulses(calibration_pulses);
// 		RealFunctionF64	offsets(n_tx_elements, 0);
// 		RealFunctionF64	amplitude_correction(n_tx_elements, 0);

	//	DisplayMathFunction2D(calibration_pulses, "Prefiltered offset");
		ComputeOffsetsCrossCorrelation(/*offsets, amplitude_correction*/);



	}
	catch(quit_application&){ throw; }
	catch(canceled_operation&){ return;  }
	catch(...)
	{
		calibration_pulses.realloc(0, 0);
		return;
	}
}


void	RawSFDataSourceTransEcho::Init()
{
	if (number_of_frames == size_t(1)) { number_of_frames = GetSigned("How many frames to read?", number_of_frames, 1, 15); }

	n_frames = number_of_frames;

	probe_carrier = MHz(3);
	probe_bandwidth = MHz(1);//?

	sound_speed = cm_sec(1.45e5);// под фантом
//	sound_speed = cm_sec(1.54e5);// по умолчанию

	filename = GetFileNameRead("TransEcho picture file", SavedGUIValue("*.raw"), "*.raw");
	string	ini_filename = filename;
	
	file.open(filename, "rb");

	LoadInfFile(filename);
//	LoadCalibrationPulses();

	first_raw_sample = mksec(5)*raw_signal_sample_rate;
	last_raw_sample = samples_per_element + first_raw_sample;

	// преобразование Гильберта уже выполнено
	hilbert_samples = samples_per_element;
	hilbert_signal_sample_rate = raw_signal_sample_rate;

	recommended_angle = degrees(45);
	recommended_n_rays = 128;// проверялось на достаточность по поперечному спектру
}

void RawSFDataSourceTransEcho::LoadData()
{
	
	data.realloc({ n_tx_elements * number_of_frames, n_rx_elements, samples_per_element });

	for (size_t frame_no = 0; frame_no < number_of_frames; ++frame_no)
	{
		if (frame_no > 0) { Init(); }
		for (size_t i = 0; i < n_tx_elements; ++i)
		{
			for (size_t j = 0; j < n_rx_elements; ++j)
			{
				row_type row;
				data.GetRow(row, { i + frame_no * n_tx_elements, j, slice_mask(0) });

				fread_numbers(row, file.c_file(), ioComplexI32_LE);
			}
		}
	}

	//DisplayMathFunction3D(data, "just loaded data");




	/*
	
	number_of_frames = GetSigned("How many frames to read?", number_of_frames, 1, 15);

	data.realloc({ n_tx_elements, n_rx_elements, samples_per_element });

	for (size_t n_frames = 0; i < number_of_frames; ++i)
	{

	for (size_t i = 0; i < n_tx_elements; ++i)
	{
		for (size_t j = 0; j < n_rx_elements; ++j)
		{
			row_type row;
			data.GetRow(row, { i, j, slice_mask(0) });

			fread_numbers(row, file.c_file(), ioComplexI32_LE);
		}
	}

	if(YesOrNo("Read another frame?", true))
	{

		Init();
	}

	
	*/


	// отмена гетеродинирования, сделанного в исходных данных
	
	static double	anti_heterodyne = 0.5;
	static bool		cancel_magnitude = false;
	if (CapsLock())
	{
		DisplayMathFunction3D(data, "just loaded data");
		anti_heterodyne = GetFloating("Anti-heterodyne factor", anti_heterodyne, -1, 1);
		cancel_magnitude = YesOrNo("Cancel magnitude?");
	}
	
	for (size_t i = 0; i < data.sizes(2); ++i)
	{
		ComplexFunction2D_F32	slice;
		data.GetSlice(slice, { slice_mask(0), slice_mask(1), i });

		if (anti_heterodyne)
		{
			double	phase_offset = anti_heterodyne * pi();

			complexF64	phasor = polar(1, phase_offset * i);

			slice *= phasor;
			if (!calibration_pulses.empty())calibration_pulses.col(i) *= phasor;
		}
		
		if (cancel_magnitude)
		{
			for (size_t j = 0; j < slice.vsize(); ++j)
			{
				for (size_t k = 0; k < slice.hsize(); ++k)
				{
					double	divisor = cabs(slice.at(j, k));
					if (divisor) slice.at(j, k) /= divisor;
				}
			}
		}	
	}
	if (CapsLock())DisplayMathFunction3D(data, "Transformed data");
}



void	RawSFDataSourceTransEcho::InitCalibrationPulses()
{

	probe_carrier = MHz(3);
	probe_bandwidth = MHz(1);//?

	sound_speed = cm_sec(1.45e5);// под фантом
								 //	sound_speed = cm_sec(1.54e5);// по умолчанию

//	filename = GetFileNameRead("TransEcho picture file", SavedGUIValue("*.raw"));
//	string	ini_filename = filename;

//	file.open(filename, "rb");

//	LoadInfFile(filename);
	LoadCalibrationPulses();

	first_raw_sample = mksec(5)*raw_signal_sample_rate;
	last_raw_sample = samples_per_element + first_raw_sample;

	// преобразование Гильберта уже выполнено
	hilbert_samples = samples_per_element;
	hilbert_signal_sample_rate = raw_signal_sample_rate;



	recommended_angle = degrees(45);
	recommended_n_rays = 128;// проверялось на достаточность по поперечному спектру
}


void RawSFDataSourceTransEcho::HighPassFilter()
{
	for (size_t i = 0; i < n_tx_elements; ++i)
	{
		for (size_t j = 0; j < n_rx_elements; ++j)
		{
			row_type row;
			data.GetRow(row, { i, j, slice_mask(0) });
			ComplexFunctionF32	buffer(row);
			buffer.FilterGauss(10);
			row -= buffer;
		}
	}
}


void RawSFDataSourceTransEcho::Normalize()
{
	for (size_t i = 0; i < n_tx_elements; ++i)
	{
		for (size_t j = 0; j < n_rx_elements; ++j)
		{
			row_type row;
			data.GetRow(row, { i, j, slice_mask(0) });
			for (size_t sample = 0; sample < samples_per_element; ++sample)
			{
				if(cabs(row[sample]) > 0)
				{
					row[sample] /= cabs(row[sample]);
				}
			}
		}
	}
}

physical_frequency RawSFDataSourceTransEcho::EstimateCentralFrequency()
{
	physical_frequency f0;
	double phase(0);
	for (size_t i = 0; i < n_tx_elements; ++i)
	{
		for (size_t j = 0; j < n_rx_elements; ++j)
		{
			row_type row;
			data.GetRow(row, { i, j, slice_mask(0) });
			for (size_t sample = 1; sample < samples_per_element; ++sample)
			{
				phase += (arg(row[sample] % row[sample - 1])) / (samples_per_element * n_tx_elements * n_rx_elements);
			}
		}
	}
	f0 = phase*raw_signal_sample_rate / (two_pi());
	return f0;
}

void RawSFDataSourceTransEcho::LoadOtherSource(RawSFDataSource	&original)
{
	data.realloc({ original.n_tx_elements, original.n_rx_elements, original.samples_per_element });
	for (size_t i = 0; i < original.n_tx_elements; ++i)
	{
		for (size_t j = 0; j < original.n_rx_elements; ++j)
		{
			row_type row;
			data.GetRow(row, { i, j, slice_mask(0) });
			auto other_row = original.GetRow(i, j);
			for (size_t sample = 0; sample < original.samples_per_element; ++sample)
			{
				row[sample] = other_row->in(sample);
			}
		}
	}
}



string	RawSFDataSourceTransEcho::GetFileName() const
{
	return filename;
}


//-----------------------------------------------------------
//
//	анализ битых элементов. сейчас вводятся вручную
//
//TODO	Предусмотреть автоматический поиск по провалам мощности
// 

// void xrad::RawSFDataSourceTransEcho::SetActiveElements(size_t /*tx_el*/, size_t /*rx_el*/)
// {
// //	data.GetRow(current, {tx_el, rx_el, slice_mask(0)});
// }




XRAD_END

