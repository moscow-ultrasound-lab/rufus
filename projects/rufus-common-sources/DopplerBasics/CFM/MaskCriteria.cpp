#include "pre.h"
#include "MaskCriteria.h"

//------------------------------------------------------------------
//
//	created:	2021/02/12	9:37
//	filename: 	MaskCriteria.cpp
//	file path:	Q:\Projects\CommonSources\DopplerBasics\CFM
//	author:		kns
//	
//	purpose:	
//
//------------------------------------------------------------------

namespace xrad
{

void mask_criteria_set::init_bloodflow_simple(size_t n_cfm_shots)
{
	phase_dispersion_criterium.init(64./n_cfm_shots, -32./n_cfm_shots, 0);
	correlation_criterium.init(1.85/sqrt(n_cfm_shots), 0.1, 1);
	stddev_criterium.init(1, 0.5, 1);
	stddev_cavitation_criterium.init(7, 1, 0);
//	re_im_correlation_criterium.init(1.5/sqrt(n_cfm_shots), 0.25, 0);

	combined_mask_threshold = 0.75;
	combined_mask_threshold_width = 0.2;
}


void mask_criteria_set::init_bloodflow(size_t n_cfm_shots)
{
	phase_dispersion_criterium.init(64./n_cfm_shots, -32./n_cfm_shots, 1);
	correlation_criterium.init(1.85/sqrt(n_cfm_shots), 0.1, 1);
	stddev_criterium.init(1, 0.5, 1);
	stddev_cavitation_criterium.init(7, 1, -1);
	re_im_correlation_criterium.init(1.5/sqrt(n_cfm_shots), 0.25, -2);

	combined_mask_threshold = 1;
	combined_mask_threshold_width = 0.2;
}

void mask_criteria_set::init_cavitation(size_t n_cfm_shots)
{
  	phase_dispersion_criterium.init(32./n_cfm_shots, -16./n_cfm_shots, -1);

 	correlation_criterium.init(1./sqrt(n_cfm_shots), -0.1, 1);
  	stddev_criterium.init(7, 1, 1);
 	stddev_cavitation_criterium.init(30, 4, 1);
	re_im_correlation_criterium.init(1.5/sqrt(n_cfm_shots), 0.05, -0.5);

	combined_mask_threshold = 1;
	combined_mask_threshold_width = 0.5;
}

void mask_criteria_set::init_oscillation(size_t n_cfm_shots)
{
	phase_dispersion_criterium.init(45./n_cfm_shots, -16./n_cfm_shots, 0.5);
 	correlation_criterium.init(0.9/sqrt(n_cfm_shots), 0.1, -1);
 	stddev_criterium.init(3, 1, 0.5);
 	stddev_cavitation_criterium.init(30, 4, -1);
 	re_im_correlation_criterium.init(1.5/sqrt(n_cfm_shots), 0.1, 1.5);

	combined_mask_threshold = 1;
	combined_mask_threshold_width = 0.1;
}


mask_layer_append_criterium::mask_layer_append_criterium(double in_T /*= 0*/, double in_w /*= 0*/, double in_weight/*=1*/)
{
	init(in_T, in_w, in_weight);
}

void mask_layer_append_criterium::init(double in_T, double in_w, double in_weight/*=1*/)
{
	m_threshold = in_T;
	m_width = in_w;
	m_weight = in_weight;

	m_active = true;
}

}//namespace xrad
