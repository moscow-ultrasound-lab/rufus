#include "pre.h"
#include "XRayStudyTaggingReport.h"
#include <FanTomCommons/XRayRoiLabels.h>

/*!
	\file
	\date 2019/12/11 4:48
	\author kulberg

	\brief 
*/

XRAD_BEGIN




wstring	XRayStudyTaggingReport::export_csv_row(const vector<wstring> &row)
{
	wstring	result;
	auto	inc = [&result](const wstring &s){result += s; result += L"\t";};
	for(auto &s: row)
	{
		inc(s);
	}
	result.pop_back();//удаляем последний tab
	result += L"\n";
	return result;
}


wstring	XRayStudyTaggingReport::export_csv_rows() const
{
	wstring	result(L"");
	static	int	counter(0);

	if(!contains_rois)
	{
		if(!study_confidence) return result;
		else
		{
			XRayRoiTaggingReport	roi;//empty_roi
			roi.adjust_common_fields(*this);

			auto	row_vec = roi.export_csv_vector();

			SetColumnValue(row_vec, ssprintf(L"%08x", RandomUniformUI32()), xray::e_finding_id_column);
			SetColumnValue(row_vec, ssprintf(L"%d", ++counter), xray::e_row_no_column);

			return export_csv_row(row_vec);
		}
//	#pragma message если ненулевой study_confidence, нужно экспортировать строку пустого рои как для исследования без локализации
	}

	for(auto &instance_record: frames_protocols)
	{
		for(auto &roi: instance_record.instance_rois)
		{
			auto	row_vec = roi.export_csv_vector();

			SetColumnValue(row_vec, ssprintf(L"%08x", RandomUniformUI32()), xray::e_finding_id_column);
			SetColumnValue(row_vec, ssprintf(L"%d", ++counter), xray::e_row_no_column);

			result += export_csv_row(row_vec);
		}
	}
	return result;
}



bool XRayStudyTaggingReport::empty() const
{
	bool	result(true);
	for(auto &frame_report: frames_protocols)
	{
		if(!frame_report.instance_rois.empty()) result = false;
	}
	return result;
}

XRayStudyTaggingReport::XRayStudyTaggingReport(const study_loader &study, const XRayFrameProtocolMaker &ipm, const wstring &in_default_diagnosis) :
	//global_frames_protocols(ipm.frames_rois_records),
	StudyTaggingReport(study.complete_study_id(), vector<tagging_system_identity>())
{
	study_id = study.complete_study_id().study_id();
	accession_number = study.complete_study_id().accession_number();
	study_instance_uid = study.complete_study_id().study_instance_uid();

	contains_rois = ipm.contains_roi();

	set<wstring> roi_diagnoses;//TODO


	for(auto &global_roi: ipm.frames_rois_records)
	{
		if(global_roi.study_instance_uid == study_instance_uid)
		{
			frames_protocols.push_back(global_roi);
		}
	}

	if(contains_rois)
	{
		diagnosis = in_default_diagnosis;
	}
	else
	{
		diagnosis = L"no roi selected";
	}
}


nlohmann::json XRayStudyTaggingReport::export_json()
{
	nlohmann::json	jprotocol;
	jprotocol[xray::study_id_tag()] = convert_to_string8(study_id);
	jprotocol[xray::accession_number_tag()] = convert_to_string8(accession_number);
	jprotocol[xray::study_instance_uid_tag()] = convert_to_string8(study_instance_uid);

	for(auto &instance_record: frames_protocols)
	{
		jprotocol[xray::instances_tag()].push_back(instance_record.export_json());
	}
	jprotocol[xray::diagnosis_tag()] = convert_to_string8(diagnosis);

	return jprotocol;
}


XRAD_END

