#ifndef __RASP_H
#define __RASP_H

#include <stdint.h>


#define	__RASP_H_INCLUDED


#if defined(_WIN32_WINNT)
	#ifdef __DLL__
	  #define DLL_EI __declspec(dllexport)
	#else
	  #define DLL_EI __declspec(dllimport)
	#endif
#else
	#define DLL_EI
#endif //_WIN32_WINNT



#if !defined(RASP_8BIT) && !defined(RASP_16BIT)
#error ("Unknown RASP target version")
#endif





//-------------------------------------------------------
//
//	RASP functions return values
//

enum RASP_ResultValues
	{
	RASP_Successful = 0,
	RASP_InvalidArgument = 1,
	RASP_RuntimeError = 2,
	RASP_NotInitialized = 3,
	RASP_IOError = 4,
	RASP_HaspError = 5,
	RASP_InternalError = 6
	};

//-------------------------------------------------------
//
//	RASP functions return type
//

typedef	int32_t RASP_Result;

//-------------------------------------------------------
//
//	RASP settings types
//

class	RASP_SettingsRecord;
typedef	RASP_SettingsRecord*	RASP_Settings;


//-------------------------------------------------------
//
//	RASP processing functions
//

extern "C"
	{
	//-------------------------------------------------------
	//
	//	Initialization/finalization
	//

	RASP_Result	DLL_EI RASP_Init();
	RASP_Result	DLL_EI RASP_Finish();
	const char	*DLL_EI RASP_ErrorDescription(RASP_Result result_code);

	//-------------------------------------------------------
	//
	//	Create/delete preset record
	//


	RASP_Result	DLL_EI RASP_CreateSettingsRecord(RASP_Settings *rsp);
	RASP_Result	DLL_EI RASP_DeleteSettingsRecord(RASP_Settings *rsp);

	//-------------------------------------------------------
	//
	//	Presets manipulations:
	//

	//	generating preset by number
	RASP_Result	DLL_EI RASP_GeneratePreset(RASP_Settings rsp, float preset_no);

	//	information about number of the available presets
	RASP_Result	DLL_EI RASP_GetMaximumPresetNumber(int32_t* n_presets);

	//	changing the presets count
	RASP_Result	DLL_EI RASP_InitPresetManager(int32_t n_presets);

	//	changing the built-in preset
	RASP_Result	DLL_EI RASP_DefinePreset(RASP_Settings rsp, int32_t preset_no);


	//-------------------------------------------------------------------------
	//
	//	choose active preset
	//

	RASP_Result	DLL_EI	RASP_SetActivePreset(const RASP_Settings rsp);

	//-------------------------------------------------------------------------
	//
	//	processing routines
	//

	#ifdef RASP_8BIT
	RASP_Result	DLL_EI RASP_ProcessRaster_UI8(uint8_t *out_data, const uint8_t *in_data,
				int32_t n_rows, int32_t n_columns,
				int32_t out_pitch, int32_t in_pitch);
	#endif//RASP_8BIT

	#ifdef RASP_16BIT
	RASP_Result	DLL_EI RASP_ProcessRaster_UI16(uint16_t *out_data, const uint16_t *in_data,
				int32_t n_rows, int32_t n_columns,
				int32_t out_pitch, int32_t in_pitch,
				int32_t intercept, float slope);

	RASP_Result	DLL_EI RASP_ProcessRaster_I16(int16_t *out_data, const int16_t *in_data,
				int32_t n_rows, int32_t n_columns,
				int32_t out_pitch, int32_t in_pitch,
				int32_t intercept, float slope);
	#endif //RASP_16BIT

	}

#if defined(_WIN32_WINNT)

	//-------------------------------------------------------------------------
	//
	//	additional interface functions available in windows versions
	//
	
	#include "RASP_ControlPanel.h"
	#include "RASP_SettingsFiles.h"
#endif //_WIN32_WINNT



#undef __RASP_H_INCLUDED


#endif // __RASP_H

