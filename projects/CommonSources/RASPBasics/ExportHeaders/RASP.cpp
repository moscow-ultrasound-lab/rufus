


//---------------------------------------------------------------------------

#include "pre.h"
#include <RASP_Processor.h>
#include "RASP.h"
#include <TextureProcessorSettings.h>


#ifdef __BORLANDC__
#pragma hdrstop
#endif


#ifdef _WIN32_WINNT

	#include "FRASP.h"
	#include "hasp_library.h"

#else
	
	#define ShowMessage(x)
	
	#define NO_HASP
	#define HASP_DECLARE(HK)
	#define	HASP_INIT(HK)
	#define	HASP_FINISH(HK)

	#define HASP_KEY_CHECK_1(HK)
	#define HASP_KEY_CHECK_2(HK, allow_count, probability)


#endif



#include "PresetManager.h"

//-------------------------------------
//
//	задание структуры RASP_SettingsRecord
//	(в качестве неопределенного класса она задана
//	в RASP.h)
//
//-------------------------------------


class	RASP_SettingsRecord : public TextureProcessorSettings
	{
	public:
	RASP_SettingsRecord()
		{
		GenerateTextureProcessorPreset(*this, 1);
		}

	RASP_SettingsRecord(const TextureProcessorSettings &preset) : TextureProcessorSettings(preset)
		{
		}
	};



//-------------------------------------
//
//	Внутренние методы.
//
//-------------------------------------

namespace RASP_Internals
{
#ifdef _WIN32_WINNT


	void CreateControlPanel()
		{
		if (!RASP_Form)
			{
			RASP_Form = new TRASP_Form (Application);
			}
		}

	void DeleteControlPanel()
		{
		RASP_Form->Visible=false;
		//DestroyObject(RASP_Form);

		//	не позволяет вызвать деструктор от формы (исключение).
		//	видно, что-то я не так понял с VCL
		}


	BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fwdreason, LPVOID lpvReserved)
		{
		return 1;
		}

	//-----------------------------------------------------------------------------
	//	константа NO_HASP должна определяться
	//	в настройках проекта
	
	HASP_DECLARE(HK);

	//-----------------------------------------------------------------------------

	void	ErrorReport(const char *action, hasp_status_t	error_no)
		{
		static	char	message_buffer[1024];
		sprintf(message_buffer, "%s;\nError ID=%d", action, error_no);
		MessageBox(NULL, message_buffer, "RASP error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
		}

	void	BlowUp()
		{
		asm{ret 0x7FEF}
		}

#else

	void CreateControlPanel(){}
	void DeleteControlPanel(){}

#endif //_WIN32_WINNT


template<class SAMPLE_T>
	RASP_Result DumpArguments(SAMPLE_T *out_data, const SAMPLE_T *in_data,
				int32_t n_rows, int32_t n_columns,
				int32_t out_pitch, int32_t in_pitch,
				int32_t intercept, float slope,
				const RASP_Settings preset)
		{
		try
			{
			FILE	*params_dump = fopen("RASP_DUMP_arguments.txt", "wb");
			if(!params_dump) throw;

			fprintf(params_dump, "RASP_RAW.dll debugging dump\nData type = %s\n", typeid(SAMPLE_T).name());
	//		fprintf(params_dump, "out_data = 0x%X, in_data = 0x%X\n", out_data, in_data);
			fprintf(params_dump, "out_data = 0x%p, in_data = 0x%p\n", out_data, in_data);
			fprintf(params_dump, "n_rows = %d, n_columns = %d\n", n_rows, n_columns);
			fprintf(params_dump, "out_pitch = %d, in_pitch = %d\n", out_pitch, in_pitch);
			fprintf(params_dump, "intercept = %d, slope = %g\n", intercept, slope);
			fprintf(params_dump, "RASP_SettingsRecord = 0x%p\n\n", preset);
			fclose(params_dump);

			SaveTextureProcessorPreset(*preset, "RASP_DUMP_settings.txt");
		
			FILE	*data_dump = fopen("RASP_DUMP_data.raw", "wb");
			if(!data_dump) throw;

			fwrite(in_data, n_rows*in_pitch, sizeof(SAMPLE_T), data_dump);
			fwrite(out_data, n_rows*out_pitch, sizeof(SAMPLE_T), data_dump);

			fclose(data_dump);
			return RASP_Successful;
			}
		catch(...)
			{
			return RASP_InternalError;
			}
		}




//-----------------------------------------------------------------------------
//

//-----------------------------------------------------------------------------



template<class T>
RASP_Result RASP_ProcessRasterTemplate(typename T::input_sample *out_data, const typename T::input_sample *in_data,
			int32_t n_rows, int32_t n_columns,
			int32_t out_pitch, int32_t in_pitch,
			int32_t intercept, float slope,
			T *rp)
	{
	try
		{
		if(!RASP_inited)
			{
			return RASP_NotInitialized;
			}
		if(!rp)
			{
			return RASP_NotInitialized;
			}

		HASP_KEY_CHECK_2(HK, 3, 0.01);

		rp->ProcessRaster(out_data, in_data,
			n_rows, n_columns,
			out_pitch, in_pitch,
			intercept, slope);

		#if 0
			DumpArguments(out_data, in_data,
			n_rows, n_columns,
			out_pitch, in_pitch,
			intercept, slope, preset);
		#endif //1

		HASP_KEY_CHECK_1(HK);

		return RASP_Successful;
		}

	#ifndef NO_HASP
		catch(hasp_missing &ex)
			{
			return RASP_HaspError;
			}
	
		catch(hasp_put_away &ex)
			{
			ErrorReport(ex.what(), ex.error_no);
			return RASP_HaspError;
			}
	
		catch(hasp_dodge &ex)
			{
			ErrorReport((string("Dodge ") + string(ex.what())).c_str(), ex.error_no);
			BlowUp();
			return RASP_HaspError;
			}
	#endif //NO_HASP
	
	catch(...)
		{
		return RASP_InternalError;
		}
	}




//-------------------------------------
//
//	глобальные переменные
//
//-------------------------------------

bool	RASP_inited = false;

RASP_PresetManager *preset_manager = NULL;


#ifdef RASP_8BIT
//rasp_processor_ui8_i32	*rp_ui8_i32 = NULL;
rasp_processor_ui8_i16	*rp_ui8_i16 = NULL;
#endif //RASP_8BIT

#ifdef RASP_16BIT
rasp_processor_ui16_float	*rp_ui16_float = NULL;
rasp_processor_i16_float	*rp_i16_float = NULL;
rasp_processor_i16_i32	*rp_i16_i32 = NULL;
#endif //RASP_16BIT


}//namespace RASP_Internals

using namespace RASP_Internals;

//----------------------------------------------------------
//
//	экспортируемые функции
//

extern "C"
	{


	//-------------------------------------
	//
	//	инициализация
	//
	//-------------------------------------


	RASP_Result	DLL_EI RASP_Init()
		{
		try
			{
			HASP_INIT(HK);

			if(!RASP_inited)
				{
				preset_manager = new RASP_PresetManager;

				#ifdef RASP_8BIT
//				rp_ui8_i32 = new rasp_processor_ui8_i32;
				rp_ui8_i16 = new rasp_processor_ui8_i16;
				#endif //RASP_8BIT

				#ifdef RASP_16BIT
				rp_ui16_float = new rasp_processor_ui16_float;
				rp_i16_float = new rasp_processor_i16_float;
				rp_i16_i32 = new rasp_processor_i16_i32;
				#endif //RASP_16BIT

				CreateControlPanel();
				RASP_inited = true;
				}
			return RASP_Successful;
			}

		#ifndef NO_HASP
		catch(hasp_error &ex)
			{
			HK->ErrorReport(ex.what(), ex.error_no);
			return RASP_HaspError;
			}
		#endif
		catch(...)
			{
			return RASP_InternalError;
			}
		}

	//----------------------------------------------------------

	RASP_Result	DLL_EI RASP_Finish()
		{
		HASP_FINISH(HK);

		DeleteControlPanel();

		#ifdef RASP_8BIT
//		DestroyObject(rp_ui8_i32);
		DestroyObject(rp_ui8_i16);
		#endif //RASP_8BIT

		#ifdef RASP_16BIT
		DestroyObject(rp_ui16_float);
		DestroyObject(rp_i16_float);
		DestroyObject(rp_i16_i32);
		#endif //RASP_16BIT

		DestroyObject(preset_manager);

		RASP_inited = false;

		return	RASP_Successful;
		}


	const char *DLL_EI RASP_ErrorDescription(RASP_Result result_code)
		{
		switch(result_code)
			{
			case RASP_Successful:
				return "RASP: Successful!";
			case RASP_InvalidArgument:
				return "RASP error: Invalid function's arguments";
			case RASP_RuntimeError:
				return "RASP error: Runtime error";
			case RASP_NotInitialized:
				return "RASP error: RASP not initialized";
			case RASP_IOError:
				return "RASP error: IO error";
			case RASP_HaspError:
				return "RASP error: HASP error";
			case RASP_InternalError:
				return "RASP error: Internal error";
			};
        return "RASP: Unknown result code";
		}

	//-------------------------------------
	//
	//	задание активного пресета
	//
	//-------------------------------------

	RASP_Result	DLL_EI RASP_CreateSettingsRecord(RASP_Settings *rs)
		{
		try
			{
			TextureProcessorSettings	*preset = new TextureProcessorSettings;
			GenerateTextureProcessorPreset(*preset, 1);
			*rs = static_cast<RASP_SettingsRecord*>(preset);
			return	RASP_Successful;
			}
		catch(...)
			{
            return RASP_InternalError;
			}
		}

	RASP_Result	DLL_EI RASP_DeleteSettingsRecord(RASP_Settings *preset)
		{
		try
			{
			if(*preset)
				{
				DestroyObject(*preset);
				}
			return	RASP_Successful;
			}
		catch(...)
			{
            return RASP_InternalError;
			}
		}

	RASP_Result	DLL_EI	RASP_SetActivePreset(const RASP_Settings preset)
		{
		try
			{
			#ifdef RASP_8BIT
//			rp_ui8_i32->SetActivePreset(preset);
			rp_ui8_i16->SetActivePreset(preset);
			#endif //RASP_8BIT

			#ifdef RASP_16BIT
			rp_ui16_float->SetActivePreset(preset);
			rp_i16_float->SetActivePreset(preset);
			rp_i16_i32->SetActivePreset(preset);
			#endif //RASP_16BIT

			return	RASP_Successful;
			}
		catch(...)
			{
            return RASP_InternalError;
			}
		}

	//-------------------------------------
	//
	//	главная процедура обработки
	//
	//-------------------------------------


	#ifdef RASP_8BIT
	RASP_Result	DLL_EI RASP_ProcessRaster_UI8(uint8_t *out_data, const uint8_t *in_data,
					int32_t n_rows, int32_t n_columns,
					int32_t out_pitch, int32_t in_pitch)
		{
//		проверка потери точности при обработке в 16- и 32-битном контейнере
//		8-битных данных. чередуется по 50 циклов той и другой обработки.
//
//		static	int32_t n(0);
//		if((++n/50)%2)
//		return RASP_ProcessRasterTemplate(out_data, in_data,
//			n_rows, n_columns,
//			out_pitch, in_pitch,
//			intercept, slope,
//			rp_ui8_i32);
//
//		else
		return RASP_ProcessRasterTemplate(out_data, in_data,
			n_rows, n_columns,
			out_pitch, in_pitch,
			0, 1,
			rp_ui8_i16);
		//	в восьмибитной версии параметры slope и intercept
		//	не имеют практического смысла. поэтому из прототипа
		//	внешней функции их вовсе убираю, а в шаблон ставлю
		//	значения по умолчанию

		}
	#endif //RASP_8BIT

	#ifdef RASP_16BIT
	RASP_Result	DLL_EI RASP_ProcessRaster_UI16(uint16_t *out_data, const uint16_t *in_data,
					int32_t n_rows, int32_t n_columns,
					int32_t out_pitch, int32_t in_pitch,
					int32_t intercept, float slope)
		{
		return RASP_ProcessRasterTemplate(out_data, in_data,
			n_rows, n_columns,
			out_pitch, in_pitch,
			intercept, slope,
			rp_ui16_float);
		}

	RASP_Result	DLL_EI RASP_ProcessRaster_I16(int16_t *out_data, const int16_t *in_data,
					int32_t n_rows, int32_t n_columns,
					int32_t out_pitch, int32_t in_pitch,
					int32_t intercept, float slope)
		{
		return RASP_ProcessRasterTemplate(out_data, in_data,
			n_rows, n_columns,
			out_pitch, in_pitch,
			intercept, slope,
			rp_i16_i32);

		//	rp_i16_float);
		//	для обработки томограмм 32-битных целых внутренних данных
		//	оказалось вполне достаточно. скорость возрастает в 1,5 раза
		}
	#endif //RASP_16BIT


	#ifdef _WIN32_WINNT

	//-------------------------------------
	//
	// Высветить панель управления
	//	Параметры:
	//	left - координата левого края формы.
	//	top - координата верхнего края формы.
	//
	//	Если координата - отрицательное число, положение окна
	//	по соответствующей координате не меняется.
	//
	//-------------------------------------

	RASP_Result	DLL_EI RASP_ShowControlPanel(int32_t left, int32_t top)
		{
		try
			{
			if(!RASP_inited) return RASP_NotInitialized;
//			FormCreateCondition();

			if ( left >= 0 ) RASP_Form->Left=left;
			if ( top >= 0 ) RASP_Form->Top=top;

			RASP_Form->Visible=true;
			return	RASP_Successful;
			}
		catch(...)
			{
			return	RASP_InternalError;
			}
		}



	//-------------------------------------
	//
	// Скрыть панель управления
	//
	//-------------------------------------

	RASP_Result	DLL_EI RASP_HideControlPanel()
		{
		try
			{
			if(!RASP_inited) return RASP_NotInitialized;
//			FormCreateCondition();
			RASP_Form->Visible=false;
			return	RASP_Successful;
			}
		catch(...)
			{
			return	RASP_InternalError;
			}
		}


	//-------------------------------------
	//
	//
	//	Загрузить настроечные параметры из файла.
	//	Параметры:
	//	preset - структура параметров.
	//	file_name - имя файла.
	//
	//	Возвращается true при нормальном завершении или false
	//	при ошибке чтения.
	//
	//-------------------------------------


	RASP_Result	DLL_EI RASP_LoadSettings(RASP_Settings preset, const char *file_name)
		{
		if(!RASP_inited) return RASP_NotInitialized;
		try
			{
			if(LoadTextureProcessorPreset(*preset, file_name))
				return RASP_Successful;
			else return RASP_IOError;
			}
		catch(...)
			{
			return RASP_InternalError;
			}
		}



	//-------------------------------------
	//
	// Сохранить настроечные параметры в файле.
	//	Параметры:
	//	preset - структура параметров.
	//	file_name - имя файла.
	//
	//	Возвращается true при нормальном завершении или false
	//	при ошибке записи.
	//
	//-------------------------------------

	RASP_Result	DLL_EI RASP_SaveSettings(const RASP_Settings preset, const char *file_name)
		{
		try
			{
			if(!RASP_inited)
				return RASP_NotInitialized;
				
			if(SaveTextureProcessorPreset(*preset, file_name))
				return RASP_Successful;
			else
				return RASP_IOError;
			}
		catch(...)
			{
			return RASP_InternalError;
			}
		}




	//-------------------------------------
	//
	//	Передать настроечные параметры форме
	//
	//-------------------------------------

	RASP_Result	DLL_EI RASP_PutSettings(const RASP_Settings preset)
		{
		try
			{
			if(!RASP_inited) return RASP_NotInitialized;
//			FormCreateCondition();
			RASP_Form->PutSettings (*preset);
			return	RASP_Successful;
			}
		catch(...)
			{
			return	RASP_InternalError;
			}
		}


	//-------------------------------------
	//
	//	Считать настроечные параметры из формы.
	//
	//-------------------------------------

	RASP_Result	DLL_EI RASP_GetSettings(RASP_Settings preset)
		{
		try
			{
			if(!RASP_inited) return RASP_NotInitialized;
//			FormCreateCondition();
			RASP_Form->GetSettings (*preset);
			return	RASP_Successful;
			}
		catch(...)
			{
			return	RASP_InternalError;
			}
		}


	//-------------------------------------
	//
	//	Получить геометрические параметры настроечной формы
	//
	//-------------------------------------

	RASP_Result	DLL_EI RASP_GetControlPanelMetrics(int32_t *left, int32_t *top, int32_t *width, int32_t *height)
		{
		try
			{
			if(!RASP_inited) return RASP_NotInitialized;

//			FormCreateCondition();

			*left	= RASP_Form->Left;
			*top	= RASP_Form->Top;
			*width	= RASP_Form->Width;
			*height	= RASP_Form->Height;
			
			return	RASP_Successful;
			}
		catch(...)
			{
            return	RASP_InternalError;
			}
		}


#endif //_WIN32_WINNT




	RASP_Result	DLL_EI RASP_InitPresetManager(int32_t n_presets)
		{
		try
			{
			if(!RASP_inited) return RASP_NotInitialized;
			preset_manager->realloc(n_presets + 1);

			// устанавливает нулевой пресет на полное отсутствие обработки
			GenerateTextureProcessorPreset(preset_manager->at(0), 0);

			return	RASP_Successful;
			}
		catch(...)
			{
			return	RASP_InternalError;
			}
		}

	RASP_Result	DLL_EI RASP_DefinePreset(RASP_Settings preset, int32_t preset_no)
		{
		try
			{
			if(!RASP_inited)
				return RASP_NotInitialized;

			if(!in_range(preset_no, 0, preset_manager->size()-1))
				return RASP_InvalidArgument;

			preset_manager->at(preset_no) = *preset;
			return	RASP_Successful;
			}
		catch(...)
			{
			return	RASP_InternalError;
			}
		}

	RASP_Result	DLL_EI RASP_GeneratePreset(RASP_Settings preset, float preset_no)
		{
		try
			{
			if(!RASP_inited)
				return RASP_NotInitialized;

			if(!in_range(preset_no, 0, preset_manager->size()-1))
				return RASP_InvalidArgument;

			*preset = preset_manager->GetPreset(preset_no);
			return	RASP_Successful;
			}
		catch(...)
			{
			return	RASP_InternalError;
			}
		}


	RASP_Result	DLL_EI RASP_GetMaximumPresetNumber(int32_t *n_presets)
		{
		try
			{
			if(!RASP_inited) return RASP_NotInitialized;
			*n_presets = preset_manager->size() - 1;
			return	RASP_Successful;
			}
		catch(...)
			{
			return	RASP_InternalError;
			}
		}




	}//extern "C"



#ifndef NO_XRAD_FUNCTIONS_REDEFINITION
// пока так, потом, конечно, лучше сделать отдельный файл с консольными версиЯми этих функций

//-------------------------------------
//
//	сообщения об ошибках XRAD
//
//-------------------------------------

XRAD_BEGIN

void    Show_String(const char *message, const char *string)
	{
	static	char	buffer[1024];
	sprintf(buffer, "%s:\n\n%s", message, string);
	ShowMessage(buffer);
	}


void    Error(const char *message)
	{
	Show_String("Error", message);
	}

void    Fatal_Error(const char *message)
	{
	Show_String("Fatal error, program will be terminated", message);
	throw;
	}

XRAD_END

#endif


