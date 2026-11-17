#include "pre.h"
#include "AlternativeAnisotropicFilter1D.h"

XRAD_BEGIN



void	CrossAnisotropicFilter1D::ComputeClearedSignal()
{
	// список критериев
	// 1. fabs(f'') > noise_threshold3 ==> original
	// 2. fabs(f'') < noise_threshold1 || fabs(f') < noise_threshold1 ==> left+right
	// 3. sign(p1)=sign(p2)  ==> left
	// 4. sign(p1)!=sign(p2)  ==> right

	combined.fill(0);
	noise_threshold /= square(main_filter_radius);
	double a1 = noise_threshold;
	double a3 = noise_threshold * 2;

	for (size_t i = 0; i < graph_size; ++i)
	{
		double	p2 = differences_sum[i];
		double	p1 = sides_difference[i];

		if (fabs(p2) > a3)
		{
			mask_left[i] = 0;
			mask_right[i] = 0;
			mask_original[i] = 1;
		}

		else if (fabs(p2) < a1 || fabs(p1) < a1)
		{
			mask_left[i] = 0.5;
			mask_right[i] = 0.5;
			mask_original[i] = 0;
		}

		else if (sign(p1) == sign(p2))
		{
			mask_left[i] = 1;
			mask_right[i] = 0;
			mask_original[i] = 0;
		}

		else if (sign(p1) != sign(p2))
		{
			mask_left[i] = 0;
			mask_right[i] = 1;
			mask_original[i] = 0;
		}


		combined[i] = original[i] * mask_original[i] + blured_left[i] * mask_left[i] + blured_right[i] * mask_right[i];
	}

}

void	HyperbolaAnisotropicFilter1D::ComputeClearedSignal()
{
	// список критериев
	// 1. fabs(f'') > noise_threshold3 ==> original
	// 2. fabs(f'') < noise_threshold1 || fabs(f') < noise_threshold1 ==> left+right
	// 3. sign(p1)=sign(p2)  ==> left
	// 4. sign(p1)!=sign(p2)  ==> right

	combined.fill(0);
	noise_threshold /= square(main_filter_radius);
	double a1 = noise_threshold;
	double a3 = noise_threshold * 2;
	double hyperbola = square(a1);

	for (size_t i = 0; i < graph_size; ++i)
	{
		double	p2 = differences_sum[i];
		double	p1 = sides_difference[i];

		if (fabs(p2) > a3)
		{
			mask_left[i] = 0;
			mask_right[i] = 0;
			mask_original[i] = 1;
		}

		else if (fabs(p2*p1) < hyperbola)
		{
			mask_left[i] = 0.5;
			mask_right[i] = 0.5;
			mask_original[i] = 0;
		}

		else if (p2*p1 > 0)
		{
			mask_left[i] = 1;
			mask_right[i] = 0;
			mask_original[i] = 0;
		}

		else if (p2*p1 < 0)
		{
			mask_left[i] = 0;
			mask_right[i] = 1;
			mask_original[i] = 0;
		}

		combined[i] = original[i] * mask_original[i] + blured_left[i] * mask_left[i] + blured_right[i] * mask_right[i];
	}


}
AlternativeAnisotropicFilter1D::AlternativeAnisotropicFilter1D()
{
	condition_01 = 0.25;
	condition_02 = 0.5;
	condition_03 = 0.75;
	condition_04 = 0.25;
	step_der = 0.007;
	step_diff = 0.1;
	large_step = 1;
}

void	AlternativeAnisotropicFilter1D::ComputeClearedSignal()
{
	for (size_t i = 0; i < graph_size; ++i)
	{
		if (differences_sum[i] > large_step)
		{
			combined[i] = original[i];
		}
		else if (fabs(sides_difference[i]) > step_diff)
		{
			if ((sides_difference[i] < 0))
			{
				combined[i] = blured_left[i];
				if (fabs(differences_sum[i]) < step_der)
				{
					combined[i] = condition_01*blured_left[i] + (1 - condition_01)*blured_right[i];
				}
				else if (differences_sum[i] < 0)
				{
					combined[i] = blured_left[i];
				}
				else
				{
					combined[i] = blured_right[i];
				}
			}
			else if ((sides_difference[i] > 0))
			{
				if (fabs(differences_sum[i]) < step_der)
				{
					combined[i] = condition_02*blured_left[i] + (1 - condition_02)*blured_right[i];
				}
				else if (differences_sum[i] > 0)
				{
					combined[i] = blured_left[i];
				}
				else
				{
					combined[i] = blured_right[i];
				}
			}

		}
		else
		{
			if ((sides_difference[i] < 0))
			{
				combined[i] = blured_left[i];
				if (fabs(differences_sum[i]) < step_der)
				{
					combined[i] = condition_03*blured_left[i] + (1 - condition_03)*blured_right[i];
				}
				else if (differences_sum[i] < 0)
				{
					combined[i] = blured_left[i];
				}
				else
				{
					combined[i] = blured_right[i];
				}
			}
			else if ((sides_difference[i] > 0))
			{
				if (fabs(differences_sum[i]) < step_der)
				{
					combined[i] = condition_04*blured_left[i] + (1 - condition_04)*blured_right[i];
				}
				else if (differences_sum[i] > 0)
				{
					combined[i] = blured_left[i];
				}
				else
				{
					combined[i] = blured_right[i];
				}
			}
		}
	}
}



XRAD_END
