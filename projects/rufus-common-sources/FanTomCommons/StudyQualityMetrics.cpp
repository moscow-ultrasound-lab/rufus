#include "pre.h"
#include "StudyQualityMetrics.h"
//#include "q:/projects/AutoStudiesSort/XRayROI/sources/XRayStudyTaggingReport.h"//TODO плохое включение
//#include "q:\projects\AutoStudiesSort\XRayROI\sources\XRayRoiPreview.h"//TODO плохое

//#include <MathFunctionGUI2D.h>

#include <FanTomCommons/TomogramQualityMetrics.h>
#include "XRayQualityMetrics.h"

/*!
\file
\date 2019/12/03
\author kulberg

\brief
*/


XRAD_BEGIN



double dice_metrics::dice() const
{
	if (!true_negative())
	{
		return 2 * tp_volume / (true_volume + testee_volume);
	}
	else
	{
		return 1;//здесь оставляю 1, но далее по true_negative() буду эти случаи исключать вместе с их делителем
	}
}



pathology_detection_report	PathologyDetectionReport(const StudyTaggingReport_ptr	&study_ptr, const wstring tested_doctor_id, double confidence)
{
	if(study_ptr->modality() == TaggingReportModality::tomogram)
	{
		auto	&study = dynamic_cast<TomogramTaggingReport &>(*study_ptr);
		return PathologyDetectionReportTomogram(study, tested_doctor_id, confidence);
	}
	else if(study_ptr->modality() == TaggingReportModality::xray)
	{
		auto	&study = dynamic_cast<XRayStudyTaggingReport &>(*study_ptr);
		return PathologyDetectionReportXRay(study, tested_doctor_id, confidence);
	}
	throw invalid_argument("Modality not supported");
}

pathology_detectioin_metrics	PathologyDetectionMetrics(const StudySetTaggingReport	&report, const wstring tested_doctor_id, double confidence)
{
	pathology_detectioin_metrics	pmetrics;
	static	int dummy = 0;

	for (auto &study_ptr : report)
	{
		pathology_detection_report	pr = PathologyDetectionReport(study_ptr, tested_doctor_id, confidence);
		pmetrics.update_metrics(pr);
	}
	std::sort(pmetrics.tp.begin(), pmetrics.tp.end());
	std::sort(pmetrics.tn.begin(), pmetrics.tn.end());
	std::sort(pmetrics.fp.begin(), pmetrics.fp.end());
	std::sort(pmetrics.fn.begin(), pmetrics.fn.end());
	return pmetrics;
}




dice_metrics	DiceMetrics(const StudyTaggingReport_ptr &study_ptr, const wstring &tested_doctor_id, double confidence_threshold)
{
	switch(study_ptr->modality())
	{
		case TaggingReportModality::tomogram:
		{
			auto	&study = dynamic_cast<TomogramTaggingReport &>(*study_ptr);
			return DiceMetricsTomogram(study, tested_doctor_id, confidence_threshold);
		}
		case TaggingReportModality::xray:
		{
			auto	&study = dynamic_cast<XRayStudyTaggingReport &>(*study_ptr);
			return DiceMetricsXRay(study, tested_doctor_id, confidence_threshold);
		}

		default:
			throw invalid_argument("Invalid study modality");
	}
}




XRAD_END
