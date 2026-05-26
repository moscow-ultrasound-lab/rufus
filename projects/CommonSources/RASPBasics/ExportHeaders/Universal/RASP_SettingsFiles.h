#ifndef __rasp_settings_files_h
#define __rasp_settings_files_h

#ifndef __RASP_H_INCLUDED
#error ("This file should be included in the Rasp.h")
#endif //__RASP_H_INCLUDED

#ifndef _WIN32_WINNT
#error ("These functions are supported only in Windows versions")
#endif

extern "C"
	{
	RASP_Result	DLL_EI RASP_LoadSettings(RASP_Settings rsp, const char *file_name);
	RASP_Result	DLL_EI RASP_SaveSettings(const RASP_Settings rsp, const char *file_name);
	};



#endif //__rasp_settings_files_h

