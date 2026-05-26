#ifndef StudyMetrics_h__
#define StudyMetrics_h__

#include <FanTomCommons/TaggingReport/StudySetTaggingReport.h>
#include <FanTomCommons/TaggingReport/XRayStudyTaggingReport.h>

//#include "XRayRoiTaggingReport.h"

/*!
\file
\date 2019/12/03
\author kulberg

\brief
*/


XRAD_BEGIN

struct pathology_detection_report
{
	pathology_detection_report(const complete_study_id_t &in_ids) : ids(in_ids){}
	
	const complete_study_id_t	ids;
	bool	ground_truth_contains_marks = false;
	bool	testee_marked_rois = false;
};


struct pathology_detectioin_metrics
{
// 	double	tp = 0;
// 	double	tn = 0;
// 	double	fp = 0;
// 	double	fn = 0;
	vector<wstring>	tp;
	vector<wstring>	tn;
	vector<wstring>	fp;
	vector<wstring>	fn;

	void	update_metrics(const pathology_detection_report	&pr)
	{
		wstring id = pr.ids.accession_number() + L"_" + pr.ids.study_id();

		if(pr.testee_marked_rois && pr.ground_truth_contains_marks) tp.push_back(id);
		if(!pr.testee_marked_rois && !pr.ground_truth_contains_marks) tn.push_back(id);
		if(pr.testee_marked_rois && !pr.ground_truth_contains_marks) fp.push_back(id);
		if(!pr.testee_marked_rois && pr.ground_truth_contains_marks) fn.push_back(id);
	}


};



struct	dice_metrics
{
	double	true_volume = 0;
	double	tp_volume = 0;
	double	fp_volume = 0;
	double	testee_volume = 0;

	void	operator += (const dice_metrics &d)
	{
		true_volume += d.true_volume;
		tp_volume += d.tp_volume;
		fp_volume += d.fp_volume;
		testee_volume += d.testee_volume;
	}

	void	operator /= ( float d)
	{
		true_volume /= d;
		tp_volume /= d;
		fp_volume /= d;
		testee_volume /= d;
	}

	void	normalize()
	{
		float divisor = true_volume + testee_volume;
		if(divisor)
		{
			true_volume /= divisor;
			tp_volume /= divisor;
			fp_volume /= divisor;
			testee_volume /= divisor;
		}
	}

	double	dice() const;
	bool	true_negative() const
	{
		return !true_volume && !testee_volume;
	}

};





pathology_detectioin_metrics	PathologyDetectionMetrics(const StudySetTaggingReport	&report, const wstring tested_doctor_id, double confidence);
dice_metrics	DiceMetrics(const StudyTaggingReport_ptr &study_ptr, const wstring &tested_doctor_id, double confidence);

// Отметка относится к одному из "подозрительных" типов. Для РМЖ это 1,3--5. Для легкого все отметки
bool	suspicious_roi(const XRayRoiTaggingReport &roi);


XRAD_END

#endif //  StudyMetrics_h__
