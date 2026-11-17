#ifndef __antispeckle_h
#define __antispeckle_h

#include <RASPBasics.h>
#include <RASPMathFunction2D.h>
#include "BlurAlgorithms.h"
#include "ImportExportSample.h"
#include "TextureProcessorSettings.h"
#include "RaspTableFunction.h"
#include <StatisticUtils.h>


XRAD_BEGIN


//void	InitTextureProcessorSettings(TextureProcessorSettings &tps); -- теперь это в конструкторе класса
// устанавливает по умолчанию какие-то правдоподобные параметры

void	GenerateTextureProcessorPreset(TextureProcessorSettings &tps, int preset_no);
// загружает предустановленные наборы параметров

int		GetMaximumPresetNo();
//	возвращает количество встроенных пресетов

bool	SaveTextureProcessorPreset(const TextureProcessorSettings &tps, const char *filename);
	// сохраняет параметры в ини-файл
bool	LoadTextureProcessorPreset(TextureProcessorSettings &tps, const char *filename);
	// читает параметры из ини-файла



template<class TPA>
class	TextureProcessor : public TextureProcessorSettings
	{
		typedef	typename TPA::value_type sample_t;
		typedef	typename TPA::array_type array1D_t;
		typedef	TPA array2D_t;

	private:
		const sample_t	max_allowed_sample_value;
		// эту величину будет назначать RaspProcessor.
		// видимо, неуклюже, но пока лучше не придумал.

		int	n_rays, n_samples;  // не менять

		array2D_t	original;
		array2D_t	rough;
		array2D_t	rough_struct_mask;
		array2D_t	fine_struct;
		array2D_t	fine_struct_mask;


		RaspTableFunction<sample_t>	gamma_f, mask_f;

		void	ComputeFineStructMask();
		void	RoughStructGamma();

		void	ComputeRoughStructMask();
		void	CombineResult(array2D_t &im);

		void	RangeCorrection(array2D_t &im);

		RASPFunctors::shift_multiplier<sample_t>	mask_multiplier;

	private:

	void	CopyTextureProcessorSettings(TextureProcessorSettings &tp1, const TextureProcessorSettings &tp2)
		{
		memcpy(&tp1, &tp2, sizeof(TextureProcessorSettings));
		}


	public:
		TextureProcessor(sample_t	max_sample);
		void	ProcessTextures(array2D_t &im);

	public:
		void	InitBuffers(int nr, int ns);
		void	InitFilters(const TextureProcessorSettings &tp);

	};

//--------------------------------------------
//
//	последующие функции нужны для подстановки
//	алгоритмы наподобие ApplyFunction.
//	как оказалось, Eclipse не желает различать
//	эти функции по аргументу в некоторых случаях, когда
//	у других компиляторов все было в порядке


inline short fast_tp_norma(const short &a, const short &b){return abs(a) + abs(b);}
inline int	fast_tp_norma(const int &a, const int &b){return abs(a) + abs(b);}
//inline int	fast_tp_norma(const int &a, const int &b){return int(abs(a.value) + abs(b.value));}

inline float fast_tp_norma(const float &a, float b){return fabs(a) + fabs(b);}
inline double fast_tp_norma(const double &a, const double &b){return fabs(a) + fabs(b);}

XRAD_END

#include "TextureProcessor.cc"

#endif //__antispeckle_h

