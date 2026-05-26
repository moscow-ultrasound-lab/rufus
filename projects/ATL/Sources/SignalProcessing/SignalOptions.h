#ifndef __signal_options_h
#define __signal_options_h

#include <XRADBasic/Sources/ScanConverter/ScanAreaGeometry.h>
// Сигнал сфокусирован некоторым образом. Имеются лучи
// в некотором диапазоне дальностей. Система координат неизвестна

XRAD_BEGIN


enum	frame_sense_t
{
	fs_frame,
	fs_subaperture,
	fs_focus,
	fs_steering
};

class	SignalOptions : public SampledFrameDimensions
{
public:
	SignalOptions();
	virtual ~SignalOptions();

//	int	n_rays;
//	int	n_samples;
//	int	n_intervals;
//	int	interval_increment;
//	int	n_Groups;	'ray groups' canceled after 14 jan 2005
//	int	n_Group_Rays;

	size_t	n_frames;
	frame_sense_t	frame_sense;


	size_t	n_elements;
// 	size_t	n_subaperture_elements_active;
// 	size_t	n_subaperture_elements_full;
};

XRAD_END


#endif //__signal_options_h