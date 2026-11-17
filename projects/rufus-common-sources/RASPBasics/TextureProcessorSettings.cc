//TextureProcessorSettings.cc


template <class floating, class boolean>
RASP_ParamSet<floating, boolean> :: RASP_ParamSet()
	{
	lateral_scale = 3;
	axial_scale = 3;
//	smooth_rays = false;
	peak_border_enhance = true;
	peak_border_factor = 1.6;
	apply_gamma_correction = false;
	gamma = 1.2;
	gamma_midpoint = 0.75/1.75;

	fine_struct_transparency = 0.3;
	fine_mask_const = 2;

	amplification_treshold = 0;
	search_aspect_ratio = 2;

	peak_enhance_treshold = 6;
	border_enhance_treshold = 18;
	peak_enhance_factor = 1;
	border_enhance_factor = 0.5;

	border_shadow_factor = 1;
	border_search_distance = 1;
	border_shadow_radius = 1;
	border_peak_bluring = 0.75;
	}


template<class f1, class b1, class f2, class b2>
void	CopyRaspSettings(RASP_ParamSet<f1, b1> &r1, const RASP_ParamSet<f2, b2> &r2)
	{
	r1.lateral_scale = r2.lateral_scale;
	r1.axial_scale = r2.axial_scale;
	r1.fine_mask_const = r2.fine_mask_const;

//	r1.smooth_rays = r2.smooth_rays;
	r1.peak_border_enhance = r2.peak_border_enhance;
	r1.apply_gamma_correction = r2.apply_gamma_correction;
	r1.gamma = r2.gamma;
	r1.gamma_midpoint = r2.gamma_midpoint;

	r1.peak_border_factor = r2.peak_border_factor;
	r1.fine_struct_transparency = r2.fine_struct_transparency;

	r1.amplification_treshold = r2.amplification_treshold;
	r1.search_aspect_ratio = r2.search_aspect_ratio;

	r1.peak_enhance_treshold = r2.peak_enhance_treshold;
	r1.border_enhance_treshold = r2.border_enhance_treshold;
	r1.peak_enhance_factor = r2.peak_enhance_factor;
	r1.border_enhance_factor = r2.border_enhance_factor;
	r1.border_shadow_factor = r2.border_shadow_factor;   
	r1.border_search_distance = r2.border_search_distance;
	r1.border_shadow_radius = r2.border_shadow_radius;
    r1.border_peak_bluring = r2.border_peak_bluring;
	}
