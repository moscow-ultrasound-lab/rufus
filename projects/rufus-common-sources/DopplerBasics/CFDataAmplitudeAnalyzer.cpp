#include "pre.h"
#include "CFDataAmplitudeAnalyzer.h"

//------------------------------------------------------------------
//
//	created:	2021/03/12	10:10
//	filename: 	CFDataAmplitudeAnalyzer.cpp
//	file path:	q:\Projects\CommonSources\DopplerBasics
//	author:		kns
//	
//	purpose:	
//
//------------------------------------------------------------------

namespace xrad
{


void CFDataAmplitudeAnalyzer::CalculateMask()
{
	m_mask.fill(1);
}

void	CFDataAmplitudeAnalyzer::AnalyzeFrame(RealFunction2D_F32& result_flow_map, RealFunction2D_F32& result_mask)
{
	tp_variadic.Start();
	ComputePreWallFilterAmplitude();
	tp_variadic.Stop();

	CalculateMask();


	result_flow_map.CopyData(m_amplitude / MaxValue(m_amplitude));
	double	gamma = 0.5;
	ApplyFunction(result_flow_map, [gamma](const float& x){return pow(x, gamma);});
	result_mask.CopyData(m_mask);
}


}//namespace xrad
