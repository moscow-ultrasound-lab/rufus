#ifndef __rasp_processor_h
#define __rasp_processor_h

#include <RASPBasics.h>
#include <RASPMathFunction2D.h>
#include <TextureProcessor.h>
#include <ImportExportSample.h>




XRAD_BEGIN

//------------------------------------------------------------------------------
//
//	выравнивание строк внутренних данных в байтах.
//	в общем случае все алгоритмы написаны без выравнивания.
//	этот общий случай выражается шаблоном
//	если в какой-либо реализации оно появляется, следует описать
//	этот случай здесь.
//


#ifdef __TI_COMPILER_VERSION__

//	никаких других конфигураций, кроме (uint8, int16)
//	для TI не предусмотрено. будет ошибка компилятора,
//	и правильно

inline int	rasp_internal_data_alignment(const uint8_t*, const int16_t*)
	{
	return 8;//bytes
	}

#else

//	для прочих платформ никакой привязки к выравниванию и, следовательно,
//	никаких ограничений в этом смысле

template<class T1, class T2>
inline int	rasp_internal_data_alignment(const T1*, const T2*)
	{
	return sizeof(T2);
	}

#endif //__TI_COMPILER_VERSION

//------------------------------------------------------------------------------
//
//	класс-обработчик
//
//	данный класс представлЯет собой интерфейс длЯ вызовов всех методов класса
//	TextureProcessor. есть идеЯ даже запретить кому-либо кроме него вызывать методы
//	класса-обработчика, так как в противном случае возникают разные неприЯтные нестыковки:
//	например, функцию отмасштабировали в целочисленный формат, а настройки -- нет
//	todo

template<class T1, class T2>
class RASP_Processor
	{
    public:
		typedef	T1	in_data_t;
		typedef	T2	proc_data_t;

		typedef	typename proc_data_t::value_type processing_sample;
		typedef	typename in_data_t::value_type input_sample;

	private:
		TextureProcessor<proc_data_t>	texture_processor;
		proc_data_t	process_buffer;
		in_data_t	in_data_shell;
		in_data_t	out_data_shell;
		const	int	row_alignment_bytes;

		void	ImportTresholdValue(float &imported_value, const float &original_value);

	public:
		RASP_Processor() : row_alignment_bytes(rasp_internal_data_alignment((input_sample*)NULL, (processing_sample*)NULL)), texture_processor(MaxAllowedSampleValue<processing_sample>(input_sample(0))){};
			// вычисляется и передается texture_processor максимальное допустимое для него
			// значение внутреннего отсчета
		void	SetActivePreset(const TextureProcessorSettings *preset);

		void	ProcessRaster(input_sample *out_data, const input_sample *in_data,
			int n_rows, int n_columns,
			int out_pitch, int in_pitch,
			int intercept, float slope);

		};




//-----------------------------------------------------------------------------
//
//	основные типы обработчиков, которые следует ипользовать
//	с различными типами входных данных

typedef RASP_Processor<uchar_data, integer_data>	rasp_processor_ui8_i32;

typedef RASP_Processor<unsigned_short_data, float_data>	rasp_processor_ui16_float;
typedef RASP_Processor<signed_short_data, float_data>	rasp_processor_i16_float;
typedef RASP_Processor<float_data, float_data>	rasp_processor_float_float;

//-----------------------------------------------------------------------------
//
//	вспомогательные типы, которые следует использовать
//	с осторожностью

typedef RASP_Processor<uchar_data, float_data>	rasp_processor_ui8_float;
typedef RASP_Processor<uchar_data, double_data>	rasp_processor_ui8_double;
typedef RASP_Processor<uchar_data, signed_short_data>	rasp_processor_ui8_i16;

typedef RASP_Processor<signed_short_data, integer_data>	rasp_processor_i16_i32;


//typedef RASP_Processor<signed_short_data, integer_data>	rasp_processor_i16;

XRAD_END

#include <RASP_Processor.cc>

#endif // __rasp_processor_h

