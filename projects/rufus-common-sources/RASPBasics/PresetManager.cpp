#include "pre.h"
#include "TextureProcessor.h"
#include "PresetManager.h"


XRAD_BEGIN

RASP_PresetManager :: RASP_PresetManager() : DataArray<TextureProcessorSettings>(GetMaximumPresetNo() + 1)
	{
	// инициализация всегда происходит со встроенным набором пресетов.
	// потом его можно изменять
	for(int n = 0; n < size(); n++)
		{
		GenerateTextureProcessorPreset((at(n)), n);
		}
	}


TextureProcessorSettings RASP_PresetManager :: GetPreset(float tps_no)
	{
	TextureProcessorSettings result;
	TextureProcessorSettings t1;
	TextureProcessorSettings t2;

	t1 = at(range(floor(tps_no), 0, size()-1));
	t2 = at(range(ceil(tps_no), 0, size()-1));
	float   x = tps_no - floor(tps_no);

//  result = at(range(int(tps_no), 0, size()-1));
	result = Interpolate(t1, t2, x);
	return result;
	}

float RASP_PresetManager :: Interpolate(float t1, float t2, float v) const
	{
	return  t1*(1.-v) + t2*v;
	}

TextureProcessorSettings RASP_PresetManager :: Interpolate(const TextureProcessorSettings &t1, const TextureProcessorSettings &t2, float v) const
	{
	TextureProcessorSettings result = t1;

	#define inter(variable) result.variable = Interpolate(t1.variable, t2.variable, v)
	#define fade1(variable) result.variable = Interpolate(t1.variable, 0, v)
	#define fade2(variable) result.variable = Interpolate(0, t2.variable, v)
	#define prefer1(variable) result.variable = t1.variable
	#define prefer2(variable) result.variable = t2.variable

	inter(lateral_scale);
	inter(axial_scale);
	inter(fine_mask_const);
	inter(fine_struct_transparency);

//       	if(v > 0.5) result.smooth_rays = t2.smooth_rays;

	if(t1.peak_border_enhance && t2.peak_border_enhance){
		inter(peak_border_factor);
		inter(peak_enhance_treshold);
		inter(border_enhance_treshold);
		inter(peak_enhance_factor);
		inter(border_enhance_factor);
		inter(border_shadow_factor);
		inter(border_search_distance);
		inter(border_shadow_radius);

		inter(border_peak_bluring);
		inter(amplification_treshold);
		inter(search_aspect_ratio);
		}
	else if(!t1.peak_border_enhance && t2.peak_border_enhance)
		{
		fade2(peak_border_factor);
		prefer2(peak_enhance_treshold);
		prefer2(border_enhance_treshold);
		prefer2(peak_enhance_factor);
		prefer2(border_enhance_factor);
		prefer2(border_shadow_factor);
		prefer2(border_search_distance);
		prefer2(border_shadow_radius);
		prefer2(peak_border_enhance);

		prefer2(border_peak_bluring);
		prefer2(amplification_treshold);
		prefer2(search_aspect_ratio);
		}
	else if(t1.peak_border_enhance && !t2.peak_border_enhance)
		{
		fade1(peak_border_factor);
		prefer1(peak_enhance_treshold);
		prefer1(border_enhance_treshold);
		prefer1(peak_enhance_factor);
		prefer1(border_enhance_factor);
		prefer1(border_shadow_factor);
		prefer1(border_search_distance);
		prefer1(border_shadow_radius);
		prefer1(peak_border_enhance);

		prefer1(border_peak_bluring);
		prefer1(amplification_treshold);
		prefer1(search_aspect_ratio);
		}

	#undef fade1
	#undef fade2

	#define fade1(variable) result.variable = Interpolate(t1.variable, 1, v)
	#define fade2(variable) result.variable = Interpolate(1, t2.variable, v)


	if(t1.apply_gamma_correction && t2.apply_gamma_correction){
				inter(gamma);
				inter(gamma_midpoint);
				}
	else if(!t1.apply_gamma_correction && t2.apply_gamma_correction){
                fade2(gamma);
                prefer2(gamma_midpoint);
                prefer2(apply_gamma_correction);
                }
	else if(t1.apply_gamma_correction && !t2.apply_gamma_correction){
                fade1(gamma);
                prefer1(gamma_midpoint);
                prefer1(apply_gamma_correction);
                }

	#undef fade1
	#undef fade2
	#undef prefer1
	#undef prefer2
	#undef inter


	return result;
	}
	
XRAD_END
