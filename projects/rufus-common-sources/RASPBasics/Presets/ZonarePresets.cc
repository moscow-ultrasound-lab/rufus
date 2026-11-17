#ifndef __presets_included
#define __presets_included

XRAD_BEGIN

enum	RASP_Preset
	{
	antispeckle_none = 0,
	antispeckle_light = 1,
	antispeckle_medium = 2,
	antispeckle_strong= 3,
	contrast = 4,
	very_contrast = 5
	};

int		GetMaximumPresetNo()
	{
    return very_contrast;
	}


void	GenerateTextureProcessorPreset(TextureProcessorSettings &tps, int preset_no)
	{
	tps.amplification_treshold = 0;
	tps.search_aspect_ratio = 2;
	tps.border_peak_bluring = 0.75;

	switch(preset_no)
		{
		case 	antispeckle_none:
			tps.lateral_scale = 4;
			tps.axial_scale = 3;
//			tps.smooth_rays = false;
			tps.peak_border_enhance = false;
			tps.peak_border_factor = 0;
			tps.apply_gamma_correction = false;
			tps.gamma = 1.0;
			tps.gamma_midpoint = 0.75/1.75;

			tps.fine_struct_transparency = 0;
			tps.fine_mask_const = 2;

			tps.peak_enhance_treshold = 256;
			tps.border_enhance_treshold = 18;
			tps.peak_enhance_factor = 1;
			tps.border_enhance_factor = 0.5;

			tps.border_shadow_factor = 1;
			tps.border_search_distance = 1;
			tps.border_shadow_radius = 1;
			break;

		case 	antispeckle_light:
			tps.lateral_scale = 4;
			tps.axial_scale = 3;
//			tps.smooth_rays = false;
			tps.peak_border_enhance = false;
			tps.peak_border_factor = 0;
			tps.apply_gamma_correction = false;
			tps.gamma = 1.0;
			tps.gamma_midpoint = 0.75/1.75;

			tps.fine_struct_transparency = 0.2;
			tps.fine_mask_const = 2;

			tps.peak_enhance_treshold = 256;
			tps.border_enhance_treshold = 18;
			tps.peak_enhance_factor = 1;
			tps.border_enhance_factor = 0.5;

			tps.border_shadow_factor = 1;
			tps.border_search_distance = 1;
			tps.border_shadow_radius = 1;
			break;

		case 	antispeckle_medium:
			tps.lateral_scale = 4;
			tps.axial_scale = 3;
//			tps.smooth_rays = false;
			tps.peak_border_enhance = false;
			tps.peak_border_factor = 0;
			tps.apply_gamma_correction = false;
			tps.gamma = 1.0;
			tps.gamma_midpoint = 0.75/1.75;

			tps.fine_struct_transparency = 0.3;
			tps.fine_mask_const = 2;

			tps.peak_enhance_treshold = 256;
			tps.border_enhance_treshold = 18;
			tps.peak_enhance_factor = 1;
			tps.border_enhance_factor = 0.5;

			tps.border_shadow_factor = 1;
			tps.border_search_distance = 1;
			tps.border_shadow_radius = 1;
			break;

		case 	antispeckle_strong:
			tps.lateral_scale = 4;
			tps.axial_scale = 3;
//			tps.smooth_rays = false;
			tps.peak_border_enhance = true;
			tps.peak_border_factor = 0.6;
			tps.apply_gamma_correction = false;
			tps.gamma = 1.0;
			tps.gamma_midpoint = 0.75/1.75;

			tps.fine_struct_transparency = 0.4;
			tps.fine_mask_const = 2;

			tps.peak_enhance_treshold = 256;
			tps.border_enhance_treshold = 18;
			tps.peak_enhance_factor = 1;
			tps.border_enhance_factor = 0.5;

			tps.border_shadow_factor = 1;
			tps.border_search_distance = 1;
			tps.border_shadow_radius = 1;
			break;

		case 	contrast:
			tps.lateral_scale = 4;
			tps.axial_scale = 3;
//			tps.smooth_rays = false;
			tps.peak_border_enhance = true;
			tps.peak_border_factor = 1.0;
			tps.apply_gamma_correction = false;
			tps.gamma = 1.2;
			tps.gamma_midpoint = 0.75/1.75;

			tps.fine_struct_transparency = 0.3;
			tps.fine_mask_const = 2;

			tps.peak_enhance_treshold = 256;
			tps.border_enhance_treshold = 18;
			tps.peak_enhance_factor = 1;
			tps.border_enhance_factor = 0.5;

			tps.border_shadow_factor = 1;
			tps.border_search_distance = 1;
			tps.border_shadow_radius = 1;
			break;

		case 	very_contrast:
			tps.lateral_scale = 4.5;
			tps.axial_scale = 3.5;
//			tps.smooth_rays = false;
			tps.peak_border_enhance = true;
			tps.peak_border_factor = 1.0;
			tps.apply_gamma_correction = true;
			tps.gamma = 1.2;
			tps.gamma_midpoint = 0.75/1.75;

			tps.fine_struct_transparency = 0.4;
			tps.fine_mask_const = 2;

			tps.peak_enhance_treshold = 256;
			tps.border_enhance_treshold = 18;
			tps.peak_enhance_factor = 1;
			tps.border_enhance_factor = 0.5;

			tps.border_shadow_factor = 1.5;
			tps.border_search_distance = 1;
			tps.border_shadow_radius = 1;
			break;
		};
	}

XRAD_END


#else

#error("Include this file once")

#endif  //__presets_included

