#include "pre.h"
#include <FanTomCommons/TomogramQualityMetrics.h>
/*!
	\file
	\date 2019/12/04 16:30
	\author kulberg

	\brief 
*/

XRAD_BEGIN

bool	DIT_response(const wstring &id)
{
	if(id[0] == L'D' && id[1] == L'I' && id[2] == L'T')
	{
		return true;
	}
	return false;
}


pathology_detection_report	PathologyDetectionReportTomogram(const TomogramTaggingReport	&study_rep, const wstring tested_doctor_id, double confidence)
{
	pathology_detection_report	pr(study_rep.ids);

	if(study_rep.tagging_system_participates(tested_doctor_id))
	{
		for(auto &cluster : study_rep)
		{
			double	max_expert_nodule(0);
			double	max_tester_nodule(0);
			for(auto &nodule : cluster)
			{
				for(auto f = nodule.begin(); f != nodule.end(); ++f)
				{
					if(!f->second.empty())
					{
						if(f->first == tested_doctor_id)
						{
							if(f->second.confidence() > confidence)
							{
								pr.testee_marked_rois = true;
								max_tester_nodule = max(max_tester_nodule, cube(f->second.diameter().mm()));
							}
						}
						else if(!DIT_response(f->first))
						{
//							if(!f->second.rejected()) pr.ground_truth_contains_marks = true;
							if(!f->second.rejected() && f->second.malignant()) pr.ground_truth_contains_marks = true;
							max_expert_nodule = max(max_expert_nodule, cube(f->second.diameter().mm()));
						}
					}
				}
			}
		}
	}

	return pr;
}



dice_metrics	DiceMetricsTomogram(const TomogramTaggingReport &study, const wstring &tested_doctor_id, double confidence)
{
	dice_metrics	dmetrics;



	if(study.tagging_system_participates(tested_doctor_id))
	{
		for(auto &cluster : study)
		{
			double	max_expert_nodule(0);
			double	max_tester_nodule(0);
			for(auto &nodule : cluster)
			{

				for(auto f = nodule.begin(); f != nodule.end(); ++f)
				{
					if(!f->second.empty())
					{
						if(f->first == tested_doctor_id)
						{
							if(f->second.confidence() > confidence)
							{
								max_tester_nodule = max(max_tester_nodule, cube(f->second.diameter().mm()));
							}
						}
						else if(!DIT_response(f->first))
						{
							max_expert_nodule = max(max_expert_nodule, cube(f->second.diameter().mm()));
						}
					}
				}
			}
			// учет обнаруженных объемов
			dmetrics.true_volume += max_expert_nodule;
			dmetrics.testee_volume += max_tester_nodule;
			dmetrics.tp_volume += min(max_expert_nodule, max_tester_nodule);
			dmetrics.fp_volume += positive(max_tester_nodule - max_expert_nodule);
		}
	}
	return dmetrics;
}


XRAD_END

