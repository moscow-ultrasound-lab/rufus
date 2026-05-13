#include "pre.h"
#include <ProbeOptions/ApertureFocusingOptions.h>

	
XRAD_BEGIN

const physical_speed	focusing_defect::standard_sound_speed = cm_sec(1.54e5);


//	вычислЯетсЯ фокусировочнаЯ задержка длЯ заданной точки пространства.
//	длЯ экономии вычислений координаты цели задаютсЯ не в виде (x,z),
// 	а в виде (z,r)

//	TODO привести все это в порядок
physical_length ComputeLensDelay(physical_length element_x, physical_length target_z, physical_length target_r, aperture_focusing focus)
	{
	physical_length lens_delay;
	
	double	direction_cosine = target_z/target_r;
	double	x_el_projection_sq = square(element_x.cm()*direction_cosine);
	double	distance_sq = square(target_r.cm());
//	double	focus_sq = square(focus.depth.cm());
	physical_length	focal_depth = focus.GetFocus(target_r);
	double	focus_sq = square(focal_depth.cm());

	if(focus.dynamical_focusing())
		{
		lens_delay = cm(sqrt(x_el_projection_sq + distance_sq)) - target_r;
		//lens_delay = x_el_projection*(x_el_projection/(2*target_r));
		}
	else
		{
		if(focal_depth.cm() > 0)
			{
			lens_delay = cm(sqrt(x_el_projection_sq + focus_sq)) - focal_depth;
			}
		else if(focal_depth.cm() < 0)
			{
			lens_delay = -cm(sqrt(x_el_projection_sq + focus_sq)) + focal_depth;
			//lens_delay = element_x*(element_x/(2*focus.depth));
			}
		else
			{
			// нулевой фокус означает фокусировку на бесконечность
			lens_delay = cm(0);
			}
		}
	return lens_delay;
	}


physical_length	focusing_defect::ComputeDelay(physical_length element_x, physical_length target_z, physical_length target_r, aperture_focusing focus) const
	{
	return ComputeLensDelay(element_x, target_z, target_r, focus) * (standard_sound_speed/deviated_sound_speed - 1.);
	}



XRAD_END


