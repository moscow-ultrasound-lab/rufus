#ifndef XRayQualityMetrics_h__
#define XRayQualityMetrics_h__

/*!
	\file
	\date 2019/12/04 16:30
	\author kulberg

	\brief  
*/


#include "StudyQualityMetrics.h"

XRAD_BEGIN

pathology_detection_report	PathologyDetectionReportXRay(const XRayStudyTaggingReport	&study_rep, const wstring testee_id, double confidence_threshold);
dice_metrics	DiceMetricsXRay(const XRayStudyTaggingReport &study, const wstring &testee_id, double confidence_threshold);


XRAD_END

#endif // XRayQualityMetrics_h__
