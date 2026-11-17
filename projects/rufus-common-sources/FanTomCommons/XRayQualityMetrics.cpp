#include "pre.h"
#include "XRayQualityMetrics.h"
/*!
	\file
	\date 2019/12/04 16:30
	\author kulberg

	\brief 
*/

XRAD_BEGIN

//TaggingReportModality::xray;

bool	suspicious_roi(const XRayRoiTaggingReport &roi)
{
	switch(roi.anatomical_location)
	{
		case e_anatomical_location::lung:
			return in_range(roi.roi_type_index, 0, 27);// любая патология для легких действительна

		case e_anatomical_location::mmg:
			switch(roi.roi_type_index)
			{
				case 1:
				case 3:
				case 4:
				case 5:
					return true;

				case 0:
				case 2: // для маммограмм исключаем доброкачественные и незнакомые находки
					return false;
				default:
					throw invalid_argument("invalid mmg roi index");
			}
		default: throw invalid_argument("invalid anatomical location");
	}
}

bool	is_near(const range2_I32 &loc1, const range2_I32 &loc2)
{
	auto	distance = norma(loc1.center()-loc2.center());
	auto	d1 = hypot(loc1.width(), loc1.height());
	auto	d2 = hypot(loc2.width(), loc2.height());
	return distance <= (d1+d2)/4;
}



pathology_detection_report	PathologyDetectionReportXRay(const XRayStudyTaggingReport	&study_rep, const wstring testee_id, double confidence_threshold)
{
	pathology_detection_report	pr(study_rep.ids);


	double	max_expert_roi_confidence = 0;
	double	max_expert_study_confidence = 0;

	double	max_testee_study_confidence = 0;
	double	max_testee_roi_confidence_all = 0;
	double	max_testee_roi_confidence_true = 0;

	for(auto &frame : study_rep.frames_protocols)
	{
		for(auto &roi : frame.instance_rois)
		{
			if(roi.tagging_system_id == testee_id)
			{
				max_testee_roi_confidence_all = max(max_testee_roi_confidence_all, roi.roi_confidence);
				max_testee_study_confidence = max(max_testee_study_confidence, roi.study_confidence);
			}

			if(suspicious_roi(roi))
			{
				if(roi.tagging_system_id == testee_id)
				{
					bool	ground_truth_mark_is_near = !roi.automatic_tagging;
					for(auto &roi2: frame.instance_rois)
					{
						if(suspicious_roi(roi2))
						{
							if(roi.tagging_system_id != roi2.tagging_system_id && !roi2.automatic_tagging)
							{
								ground_truth_mark_is_near |= is_near(roi.loc, roi2.loc);
							}
						}
					}

					if(ground_truth_mark_is_near)
					{
						max_testee_roi_confidence_true = max(max_testee_roi_confidence_true, roi.roi_confidence);
					}
				}
				if(!roi.automatic_tagging)
				{
					max_expert_roi_confidence = max(max_expert_roi_confidence, roi.roi_confidence);
					max_expert_study_confidence = max(max_expert_study_confidence, roi.study_confidence);
				}
			}
		}
	}

	double	max_expert_confidence = max_expert_study_confidence ? max_expert_study_confidence : max_expert_roi_confidence;
//	pr.ground_truth_contains_marks = max_expert_confidence >= confidence_threshold;
	pr.ground_truth_contains_marks = max_expert_confidence >= 0.5;

	double	max_testee_confidence = 0;

	if(max_testee_study_confidence)
	{
		max_testee_confidence = max_testee_study_confidence;
	}
	else if(pr.ground_truth_contains_marks)
	{
	// в тех случаях, когда ИИ не записывала study_confidence, берем максимальное значение из roi_confidence
		max_testee_confidence = max_testee_study_confidence ? max_testee_study_confidence : max_testee_roi_confidence_true;
	}
	else
	{
		max_testee_confidence = max_testee_study_confidence ? max_testee_study_confidence : max_testee_roi_confidence_all;
	}

//	max_testee_confidence = max_testee_study_confidence;

	pr.testee_marked_rois = max_testee_confidence >= confidence_threshold;

	return pr;
}




dice_metrics	DiceMetricsXRay(const XRayStudyTaggingReport &study, const wstring &testee_id, double confidence_threshold)
{
	dice_metrics	dmetrics_result;

	const size_t	scale_divisor = 4;
	double	dice_divisor = 1;

	auto	stretch = [&scale_divisor](const range2_I32 &loc)
	{
		// сжимаем диапазон на заданную пропорцию, при этом расширяем результат на 1 пиксель, чтобы избежать полного схлопывания очень маленьких объектов
		range2_I32	result = loc/scale_divisor;
		--result.y1();
		--result.x1();
		++result.y2();
		++result.x2();

		return result;
	};
	if(study.tagging_system_participates(testee_id))
	{
		for(auto &frame : study.frames_protocols)
		{
			dice_metrics	frame_dmetrics;
			int	vs(0), hs(0);
			double	max_roi_confidence = 0;
			for(auto &roi: frame.instance_rois)
			{
				if(suspicious_roi(roi))
				{
					vs = max(vs, roi.loc.y2())/scale_divisor;
					hs = max(hs, roi.loc.x2())/scale_divisor;
					max_roi_confidence = max(max_roi_confidence, roi.roi_confidence);
				}
			}

			if(max_roi_confidence >= confidence_threshold  && vs && hs)
			{
				RealFunction2D_F32	testee(vs, hs, 0);
				RealFunction2D_F32	gt(vs, hs, 0);

				float	fill_value = 256;

				for(auto &roi: frame.instance_rois)
				{
					if(suspicious_roi(roi))
					{
						if(roi.roi_confidence > confidence_threshold)
						{
							if(roi.tagging_system_id == testee_id)
							{
								FillEllipse(testee, stretch(roi.loc), 256);
							}
							if(!roi.automatic_tagging)
							{
								FillEllipse(gt, stretch(roi.loc), 256);
							}
						}
					}
				}

				for(size_t i = 0; i < vs; ++i)
				{
					for(size_t j = 0; j < hs; ++j)
					{
						if(gt.at(i, j)) ++frame_dmetrics.true_volume;
						if(testee.at(i, j)) ++frame_dmetrics.testee_volume;
						if(gt.at(i, j) && testee.at(i, j)) ++frame_dmetrics.tp_volume;
						if(!testee.at(i, j)) ++frame_dmetrics.fp_volume;
					}
				}
				if(CapsLock())
				{
					DisplayMathFunction2D(gt-testee/2, "Difference");
				}
			}
			if(!frame_dmetrics.true_negative())
			{
				frame_dmetrics.normalize();
				dmetrics_result += frame_dmetrics;
				++dice_divisor;
			}
		}
	}

	dmetrics_result /= dice_divisor;
	dmetrics_result.normalize();
	return dmetrics_result;
}


XRAD_END

