#ifndef TomogramQualityMetrics_h__
#define TomogramQualityMetrics_h__

/*!
	\file
	\date 2019/12/04 16:30
	\author kulberg

	\brief  
*/


#include <FantomCommons/StudyQualityMetrics.h>

XRAD_BEGIN

pathology_detection_report	PathologyDetectionReportTomogram(const TomogramTaggingReport	&study_rep, const wstring tested_doctor_id, double confidence);
dice_metrics	DiceMetricsTomogram(const TomogramTaggingReport &study, const wstring &tested_doctor_id, double confidence);


XRAD_END

#endif // TomogramQualityMetrics_h__
