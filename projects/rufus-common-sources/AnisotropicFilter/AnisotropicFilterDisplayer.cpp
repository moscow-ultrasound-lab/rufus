/*
	created:	6:2:2017   12:16
*/
#include "pre.h"
#include "AnisotropicFilterDisplayer.h"

XRAD_BEGIN



void AnisotropicFilterDisplayer::ShowBlurResult(const AnisotropicFilter1D &filter, bool stop)
{
	blur_result_gs.ChangeGraph(0, components_all.row(e_original), 0, 1, "Original");
	blur_result_gs.ChangeGraph(1, components_all.row(e_left), 0, 1, "Blurred Left");
	blur_result_gs.ChangeGraph(2, components_all.row(e_right), 0, 1, "Blurred right");

	blur_result_gs.Display(stop);
}


void AnisotropicFilterDisplayer::ShowDifferences(const AnisotropicFilter1D &filter, bool stop)
{
	differences_gs.ChangeGraph(0, sides_difference, 0, 1, "left-right");
	differences_gs.ChangeGraph(1, difference_all.row(e_left), 0, 1, "difference_left");
	differences_gs.ChangeGraph(2, difference_all.row(e_right), 0, 1, "difference_right");
	differences_gs.ChangeGraph(3, differences_sum, 0, 1, "differences_sum");

	differences_gs.Display(stop);

	parametric_gs.ChangeGraph(0, sides_difference, differences_sum, "f'-f''");
	parametric_gs.ChangeGraph(1, post_sides_difference, post_differences_sum, "f'-f'' post");
	parametric_gs.Display(stop);
}

void AnisotropicFilterDisplayer::ShowMasks(const AnisotropicFilter1D &filter, bool stop)
{
	masks_gs.ChangeGraph(0, mask_all.row(e_original), 0, 1, "Mask original");
	masks_gs.ChangeGraph(1, mask_all.row(e_left), 0, 1, "Mask left");
	masks_gs.ChangeGraph(2, mask_all.row(e_right), 0, 1, "Mask right");

	masks_gs.Display(stop);
}


void	AnisotropicFilterDisplayer::ShowClearedSignal(const AnisotropicFilter1D &filter, bool stop)
{
	result_gs.ChangeGraph(0, combined, 0, 1, L"Combined");
	result_gs.ChangeGraph(1, original, 0, 1, L"Original");
	result_gs.Display(stop);
}



XRAD_END
