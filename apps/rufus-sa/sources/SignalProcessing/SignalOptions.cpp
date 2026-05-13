#include "Pre.h"
#include "SignalOptions.h"

XRAD_BEGIN

SignalOptions::SignalOptions()
{
	n_rays = 128;
	n_samples = 200;

//	n_intervals = 0;
//	interval_increment = 0;


	n_elements = 48;
	n_frames = 1;
//	n_subaperture_elements_active = n_elements;
//	n_subaperture_elements_full = n_elements;
	frame_sense = fs_subaperture;
}

SignalOptions :: ~SignalOptions()
{
}

XRAD_END
