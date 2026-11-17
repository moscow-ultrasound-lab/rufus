#include "pre.h"

#ifndef __MWERKS__
#pragma hdrstop
#endif //__MWERKS__

#include "TextureProcessorSettings.h"
#include <XRADIniFile.h>

#if defined (PRESETS_SPECTROMED_ULTRASOUND)
#include <SpectromedPresets.cc>

#elif defined (PRESETS_ZONARE_ULTRASOUND)
#include <ZonarePresets.cc>

#else
#error	Unknown RASP Ultrasound target device
#endif

XRAD_BEGIN



bool	LoadTextureProcessorPreset(TextureProcessorSettings &tps, const char *filename)
	{
	QMIniReader	ifr;
	try
		{
		ifr.Open(filename);

		ifr.SetSection("PROBE_DEPENDENT_SETTINGS");
		tps.lateral_scale = ifr.ReadDouble("lateral_scale", tps.lateral_scale);
		tps.axial_scale = ifr.ReadDouble("axial_scale", tps.axial_scale);
		tps.fine_mask_const = ifr.ReadDouble("fine_mask_const", tps.fine_mask_const);

		ifr.SetSection("USER_CNAHGEABLE_SETTINGS");
		tps.fine_struct_transparency = ifr.ReadDouble("fine_struct_transparency", tps.fine_struct_transparency); // 0..1, 0.01
	//	tps.smooth_rays = ifr.ReadInt("smooth_rays", tps.smooth_rays);
		tps.peak_border_enhance = ifr.ReadInt("peak_border_enhance", tps.peak_border_enhance);
		tps.apply_gamma_correction = ifr.ReadInt("apply_gamma_correction", tps.apply_gamma_correction);


		ifr.SetSection("GAMMA_CORRECTION_SETTINGS");
		tps.gamma = ifr.ReadDouble("gamma", tps.gamma);
		tps.gamma_midpoint = ifr.ReadDouble("gamma_midpoint", tps.gamma_midpoint);

		ifr.SetSection("PEAK_BORDER_ENHANCE_SETTINGS");

		tps.amplification_treshold = ifr.ReadDouble("amplification_treshold", 0);
		tps.search_aspect_ratio = ifr.ReadDouble("search_aspect_ratio", 2);

		tps.peak_border_factor = ifr.ReadDouble("peak_border_factor", tps.peak_border_factor); // 0..10, 0.1
		tps.peak_enhance_treshold = ifr.ReadDouble("peak_enhance_treshold", tps.peak_enhance_treshold);
		tps.border_enhance_treshold = ifr.ReadDouble("border_enhance_treshold", tps.border_enhance_treshold);
		tps.peak_enhance_factor = ifr.ReadDouble("peak_enhance_factor", tps.peak_enhance_factor);
		tps.border_enhance_factor = ifr.ReadDouble("border_enhance_factor", tps.border_enhance_factor);
		tps.border_shadow_factor = ifr.ReadDouble("border_shadow_factor", tps.border_shadow_factor);
		tps.border_search_distance = ifr.ReadDouble("border_search_distance", tps.border_search_distance);
		tps.border_shadow_radius = ifr.ReadDouble("border_shadow_radius", tps.border_shadow_radius);
		tps.border_peak_bluring = ifr.ReadDouble("border_peak_bluring", tps.border_peak_bluring);

		ifr.Close();

		return true;
		}
	catch(ini_file_error &ex)
		{
		return false;
		}
	}

bool	SaveTextureProcessorPreset(const TextureProcessorSettings &tps, const char *filename)
	{
	QMIniWriter	ifw;
	if(!ifw.Open(filename)) return false;

	ifw. WriteSection("PROBE_DEPENDENT_SETTINGS");
	ifw.WriteDouble("lateral_scale", tps.lateral_scale);
	ifw.WriteDouble("axial_scale", tps.axial_scale);
	ifw.WriteDouble("fine_mask_const", tps.fine_mask_const);

	ifw.WriteSection("USER_CNAHGEABLE_SETTINGS");
	ifw.WriteDouble("fine_struct_transparency", tps.fine_struct_transparency); // 0..1, 0.01
//	ifw.WriteInt("smooth_rays", tps.smooth_rays);
	ifw.WriteInt("peak_border_enhance", tps.peak_border_enhance);
	ifw.WriteInt("apply_gamma_correction", tps.apply_gamma_correction);

	ifw.WriteSection("GAMMA_CORRECTION_SETTINGS");
	ifw.WriteDouble("gamma", tps.gamma);
	ifw.WriteDouble("gamma_midpoint", tps.gamma_midpoint);


	ifw.WriteSection("PEAK_BORDER_ENHANCE_SETTINGS");

	ifw.WriteDouble("amplification_treshold", tps.amplification_treshold);
	ifw.WriteDouble("search_aspect_ratio", tps.search_aspect_ratio);

	ifw.WriteDouble("peak_border_factor", tps.peak_border_factor); // 0..10, 0.1
	ifw.WriteDouble("peak_enhance_treshold", tps.peak_enhance_treshold);
	ifw.WriteDouble("border_enhance_treshold", tps.border_enhance_treshold);
	ifw.WriteDouble("peak_enhance_factor", tps.peak_enhance_factor);
	ifw.WriteDouble("border_enhance_factor", tps.border_enhance_factor);
	ifw.WriteDouble("border_shadow_factor", tps.border_shadow_factor);
	ifw.WriteDouble("border_search_distance", tps.border_search_distance);
	ifw.WriteDouble("border_shadow_radius", tps.border_shadow_radius);
	ifw.WriteDouble("border_peak_bluring", tps.border_peak_bluring);

	ifw.Close();
	return true;
	}

XRAD_END

