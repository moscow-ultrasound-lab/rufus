#include "pre.h"
#include "PhaseAnalyzerTimers.h"

//------------------------------------------------------------------
//
//	created:	2021/01/27	12:38
//	filename: 	PhaseAnalyzerTimers.cpp
//	file path:	q:\Projects\CommonSources\DopplerBasics
//	author:		kns
//	
//	purpose:	
//
//------------------------------------------------------------------

namespace xrad
{



physical_time PhaseAnalyzerTimers::GetProcessingTime() const
{
	physical_time	parts_sum = msec(0);

	if(tp_wall_filter.Count()) parts_sum += tp_wall_filter.MeanElapsed();
	if(tp_blur.Count()) parts_sum += tp_blur.MeanElapsed();
	if(tp_conjugate.Count())parts_sum += tp_conjugate.MeanElapsed();
	if(tp_acquire.Count()) parts_sum += tp_acquire.MeanElapsed();
	if(tp_postfilter.Count())parts_sum += tp_postfilter.MeanElapsed();
	if(tp_build_mask.Count())parts_sum += tp_build_mask.MeanElapsed();

	return parts_sum;
}

string PhaseAnalyzerTimers::GetTimeConsumptionReport() const
{
	string time_consumption_report;

	if(tp_wall_filter.Count()) time_consumption_report += ssprintf("\tWall Filter time %g ms\n", tp_wall_filter.MeanElapsed().msec());
	if(tp_blur.Count()) time_consumption_report += ssprintf("\tBlur time %g ms\n", tp_blur.MeanElapsed().msec());
	if(tp_conjugate.Count()) time_consumption_report += ssprintf("\tConjugate time %g ms\n", tp_conjugate.MeanElapsed().msec());
	if(tp_acquire.Count()) time_consumption_report += ssprintf("\tAcquire time %g ms\n", tp_acquire.MeanElapsed().msec());
	if(tp_postfilter.Count()) time_consumption_report += ssprintf("\tPost-filtering time %g ms\n", tp_postfilter.MeanElapsed().msec());
	if(tp_build_mask.Count()) time_consumption_report += ssprintf("\tMask build time %g ms\n", tp_build_mask.MeanElapsed().msec());

	time_consumption_report += ssprintf("\tElements sum is %g ms\n", GetProcessingTime().msec());

	if(tp_variadic.Count()) time_consumption_report += ssprintf("\tVariadic time is %g ms\n", tp_variadic.MeanElapsed().msec());

	return time_consumption_report;
}


}//namespace xrad
