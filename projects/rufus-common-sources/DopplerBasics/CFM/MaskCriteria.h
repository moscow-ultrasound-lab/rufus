#ifndef __MaskCriteria_h
#define __MaskCriteria_h

//------------------------------------------------------------------
//
//	created:	2021/02/12	9:37
//	filename: 	MaskCriteria.h
//	file path:	q:\Projects\CommonSources\DopplerBasics\CFM
//	author:		kns
//	
//	purpose:	
//
//------------------------------------------------------------------

namespace xrad
{


// Разбиение по порогу t с шириной переходной зоны w. Для положительных w значения меньше t-w обнуляются, больше t+w превращаются в 1
// При отрицательныом w, наоборот, малые значения становятся равны 1, большие 0.
inline float	soft_threshold(float x, float threshold, double w) { return 0.5 + 0.5*range((x-threshold)/w, -1, 1);}

struct mask_layer_append_criterium
{
	mask_layer_append_criterium(double in_t, double in_w = 0, double in_weight=1);
	mask_layer_append_criterium(){ m_active = false; }
	void	init(double in_T, double in_w, double in_weight=1);

//	float operator() (float &y, const float &x) { return m_active ? y += m_weight*soft_threshold(x, m_threshold, m_width) : y; }
	float operator() (float &y, const float &x) { return y += m_weight*soft_threshold(x, m_threshold, m_width); }// версия выше также дееспособна, но проверка active в цикле может снизить производительность.
	
	void	deactivate(){ m_active = false; }
	bool	active() const { return m_active; }
private:

	bool	m_active = false;
	double m_threshold, m_width, m_weight;
};

// Набор критериев для вычисления масок кровотока, микроколебаний и кавитации
struct mask_criteria_set
{
	mask_layer_append_criterium	phase_dispersion_criterium;
	mask_layer_append_criterium	correlation_criterium;
	mask_layer_append_criterium	stddev_criterium;
	mask_layer_append_criterium	stddev_cavitation_criterium;
	mask_layer_append_criterium	re_im_correlation_criterium;

	void init_bloodflow_simple(size_t n_cfm_shots);
	void init_bloodflow(size_t n_cfm_shots);
	void init_cavitation(size_t n_cfm_shots);
	void init_oscillation(size_t n_cfm_shots);

	double	combined_mask_threshold;
	double	combined_mask_threshold_width;
};

}//namespace xrad

#endif //__MaskCriteria_h
