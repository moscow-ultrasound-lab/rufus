#ifndef __texture_processor_settings_h
#define __texture_processor_settings_h
/*
	----------------------------
	30.11.2008
	первая рабочая версия
	кнс
	----------------------------
	02.02.2010
	добавлены новые параметры обработки:

	floating	amplification_treshold;
		порог, ниже которого усиление пиков и границ не осуществляется.
		по умолчанию 0
	floating		search_aspect_ratio;
		дополнительное управление алгоритмом поиска пиков и границ. по умолчанию
		для узи = 2
		для кт = 1
	floating		border_peak_bluring;
		дополнительное управление алгоритмом обработки пиков и границ. по умолчанию
		для узи = 0,75
		для кт = 1
	кнс

	----------------------------
	26.06.2011
	уничтожен параметр
	boolean	smooth_rays;
	так как связанная с ним обработка оказалась малополезной и ресурсоемкой

	----------------------------
	27.09.2011
	в шаблон класса добавлен конструктор по умолчанию. создан соответствующий
	файл .cc. при этом этот файл исключен из файла rasp.h и сделан невидимым
	для внешнего пользователя rasp.dll

	----------------------------

*/



template <class floating, class boolean>
struct	RASP_ParamSet
	{
//	hardware dependent settings

	floating	lateral_scale;
	floating	axial_scale; // 0..10,0.1
	floating	fine_mask_const; //0..20, 0.1

//	basic user changeable settings
//	boolean	smooth_rays;
	boolean	peak_border_enhance;
	boolean	apply_gamma_correction;
	floating gamma;
	floating gamma_midpoint;

	floating	peak_border_factor; // 0..10, 0.1
	floating	fine_struct_transparency; // 0..1, 0.01

//	advanced user changeable settings

	floating	amplification_treshold;
	floating	search_aspect_ratio;

	floating	peak_enhance_treshold;
	floating	border_enhance_treshold;
	floating	peak_enhance_factor;

	floating	border_enhance_factor;
	floating	border_shadow_factor;
	floating	border_search_distance;
	floating	border_shadow_radius;
	floating	border_peak_bluring;

    RASP_ParamSet();
	};


typedef	RASP_ParamSet<float, bool> TextureProcessorSettings;


#include <TextureProcessorSettings.cc>

#endif //__texture_processor_settings_h

