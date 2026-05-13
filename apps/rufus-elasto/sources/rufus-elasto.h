#ifndef __ElastographyLibrary_h
#define __ElastographyLibrary_h

//------------------------------------------------------------------
//
//	created:	2014/06/14
//	created:	14.6.2014   12:46
//	filename: 	q:\programs\ElastographyTest\ElastoGrafica\ElastoGrafica.h
//	file path:	q:\programs\ElastographyTest\ElastoGrafica
//	author:		kns
//	
//	purpose:	
//
//------------------------------------------------------------------

#include <string>
#include <q:/Projects/CommonSources/DopplerBasics/CFMModes.h >
//#include "ElastographyStatus.h"

#if defined(_MSC_VER) && !defined(ELASTO_DLL_EI)
	#ifdef __DLL__
	  #define ELASTO_DLL_EI __declspec(dllexport)
	#else
	  #define ELASTO_DLL_EI __declspec(dllimport)
	#endif
#else
	#define ELASTO_DLL_EI
#endif //_MSC_VER



namespace rufus-elasto
{
using namespace std;


enum class elasto_status
{
//----------------------------------------------------------
// статус успешного завершения операций
	ok,

	//----------------------------------------------------------
	//	статусы ошибок
	not_initialized,
		// попытка работать с неинициализированной библиотекой
	initialization_error,
		// ошибка инициализации
	finalization_error,
		// ошибка финализации
	operation_error
		// ошибки в процессе вычислений
};


enum axial_blur_t
{
	b0,
	b1,
	elasto_blur_2,
	elasto_blur_3,
	elasto_blur_4
};

enum frame_averaging_t
{
	elasto_frame_averaging_0,
	elasto_frame_averaging_1,
	elasto_frame_averaging_2,
	elasto_frame_averaging_3,
	elasto_frame_averaging_4
};

enum
{
	n_elasto_blur_values = elasto_blur_4 + 1,
	n_elasto_frame_averaging_values = elasto_frame_averaging_4 + 1
};



elasto_status	ELASTO_DLL_EI InitElastographyLib(const char *settings_filename);
elasto_status	ELASTO_DLL_EI FinishElastographyLib();

elasto_status	ELASTO_DLL_EI ResetElastogram();
	// сбрасывает данные межкадрового накопления.
	// обязательно вызывать, когда заново запустили обработку после паузы,
	// чтобы новая картинка не началась с затухающих следов от старой.
	
elasto_status	ELASTO_DLL_EI SetElastogramAgilityDirect(double factor);
elasto_status	ELASTO_DLL_EI SetElastogramAgility(frame_averaging_t n);
	// коэффициент "подвижности" эластограммы (определяет межкадровое накопление)
	// допустимые значения в полуинтервале (0,1]
	// 0 не допускается; 1 означает отсутствие накопления.
	// по умолчанию стоит 0.25.
	// усиленное накопление для малоконтрастной эластограммы 0.1;
	// может быть полезным также значение 0.05, особенно при работе
	// в режиме "тремора", без интенсивных нажатий.
	
elasto_status	ELASTO_DLL_EI SetElastogramBlurDirect(double radius);
elasto_status	ELASTO_DLL_EI SetElastogramBlur(axial_blur_t n);
	// этот параметр способен очень сильно повысить чувствительность
	// ценою резкости, поэтому оператор должен им пользоваться свободно
	// (должен быть под рукой элемент управления). диапазон полезных
	// значений экспериментально, по фантому, определяется от 1 до 5,
	// но функция примет любое число, большее нуля.
	
elasto_status	ELASTO_DLL_EI SetFrameSizes(size_t n_shots, size_t n_beams, size_t n_samples, size_t beams_in_sweep, size_t ray_header_size_in_bytes, size_t in_roi_sample_start, size_t in_roi_sample_end);
	// аллокирует буферные массивы под всю последующую обработку.
	// вызывается однократно при начале работы и при каждой смене размеров кадра.
	// это занимает какое-то время, поэтому нежелательно делать это "на ходу",
	// при вызове функции обработки данных, чтобы вызывающая процедура не сбилась с ритма.
	// далее при вызове функции обработки мы даем только указатели на массивы, без размеров.

elasto_status	ELASTO_DLL_EI BuildElastogram(const __int32 *in_cfm_data, float *out_elastogram, float *offset_value, float *mask);
	// строит эластограмму по данным в массиве in_cfm_data. размер входного массива
	// 2*n_beams*n_shots*n_samples чисел signed __int32. расположение по свипам такое,
	// как было в записанных ранее файлах: идут по beams_in_sweep лучей n-го выстрела
	// разных направлений, потом то же самое для n+1-го выстрела тех же направлений и т.д.
	//
	// массив out_elastogram выделяется не мною, он должен содержать n_beams*n_samples отсчетов.
	// числа в этом массиве в диапазоне от 0 до 1, каждое пропорционально локальной "мягкости".
	// таким образом, отношение этих чисел (или их средних значений по выделенным областям)
	// можно показывать пользователю как strain ratio.
	// умножение на 255 и перевод в unsinged __int8 дает яркость пикселов или индексы палитры
	// для построения видимой эластограммы.
	//
	// указатель float *offset_value содержит одно число, которое отображается на графике компрессии
	//
	// указатель float* mask содержит n_beams*n_samples отсчетов маски прозрачности, накладываемой на эластограмму
	// 0 -- "эластограмма полностью прозрачна" (видно только B)
	// 1 -- "эластограмма полностью закрывает B". В реализации на октябрь 2016 г. по умолчанию всюду пишется 1 ради
	// совместимости с предыдущей версией. Любые изменения будут согласовываться.
	
	
string	ELASTO_DLL_EI GetTimeConsumptionReport();

elasto_status	ELASTO_DLL_EI ChangeMode(xrad::cfm_mode);


// bool	ELASTO_DLL_EI DataUnswept();
	// функция нужна мне для отладки, вам она ни к чему
	
};//namespace rufus-elasto


#endif //__ElastographyLibrary_h