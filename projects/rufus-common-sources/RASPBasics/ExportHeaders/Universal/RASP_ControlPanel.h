#ifndef __rasp_control_panel_h
#define __rasp_control_panel_h

#ifndef __RASP_H_INCLUDED
#error ("This file should be included in the Rasp.h")
#endif //__RASP_H_INCLUDED

#ifndef _WIN32_WINNT
#error ("These functions are supported only in Windows versions")
#endif

extern "C"
	{
	RASP_Result	DLL_EI RASP_ShowControlPanel(int left=-1, int top=-1);
	RASP_Result	DLL_EI RASP_HideControlPanel();
	RASP_Result	DLL_EI RASP_GetControlPanelMetrics (int *left, int *top, int *width, int *height);

	RASP_Result	DLL_EI RASP_PutSettings(const RASP_Settings rsp);
	RASP_Result	DLL_EI RASP_GetSettings(RASP_Settings rsp);
	};



#endif //__rasp_control_panel_h

