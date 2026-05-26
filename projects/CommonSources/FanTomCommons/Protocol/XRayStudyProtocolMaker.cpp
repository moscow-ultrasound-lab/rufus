#include "pre.h"
#include "XRayStudyProtocolMaker.h"
#include <XRADBasic/Sources/Utils/utf8_ofstream.h>
#include <iostream>

#include <FanTomCommons/XRayRoiLabels.h>
#include <FanTomCommons/ExtractXRayROIShape.h>
#include <FanTomCommons/StudyQualityMetrics.h>

#include <AnonymizerCommons/AnonymizerCommons.h>


/*!
	\file
	\date 2019/11/11 10:07
	\author kulberg

	\brief 
*/

XRAD_BEGIN


void	AnalyzeROIsReport(const vector<XRayRoiTaggingReport> &rois, const wstring &title)
{
	size_t	N = 100;
	RealFunctionF64	roi_confidence_total(N, 0), study_confidence_histogram(N, 0), roi_confidence_suspicious_histogram(N, 0), roi_confidence_benign_histogram(N,0);
	map<wstring, double> ids;

	for(auto &roi: rois)
	{
		ids[roi.study_id] = max(ids[roi.study_id], roi.study_confidence);
		size_t	rc = range(roi.roi_confidence * double(N-1), 0, N-1);

// 		++study_confidence_histogram[sc];

		if(suspicious_roi(roi))
		{
			++roi_confidence_suspicious_histogram[rc];
		}
		else
		{
			++roi_confidence_benign_histogram[rc];
		}
		++roi_confidence_total[rc];
	}

	for(auto &study: ids)
	{
		size_t	sc = range(study.second * double(N-1), 0, N-1);
		++study_confidence_histogram[sc];
	}

// 	study_confidence_histogram /= ElementSum(study_confidence_histogram);
// 	roi_confidence_total /= ElementSum(roi_confidence_total);
// 	roi_confidence_suspicious_histogram /= ElementSum(roi_confidence_suspicious_histogram);
// 	roi_confidence_benign_histogram /= ElementSum(roi_confidence_benign_histogram);

	GraphSet	gs(title + L" histograms", L"N studies", L"Confidence value");

	gs.ChangeGraphUniform(0, study_confidence_histogram, 0, 1./N, title + L" study confidence");
 	gs.ChangeGraphUniform(1, roi_confidence_total, 0, 1./N, title + L" total ROI confidence");
	gs.ChangeGraphUniform(2, roi_confidence_suspicious_histogram, 0, 1./N, title + L" suspicious ROI confidence");
	gs.ChangeGraphUniform(3, roi_confidence_benign_histogram, 0, 1./N, title + L" benign ROI confidence");

	gs.Display(false);

	ShowUnsigned(title + L" contained the following studies number:", ids.size());

// 	ShowText(L"Предварительный подсчет числа просмотренных исследований",
// 			 ssprintf(L"Испытуемый оставил пометки в следующем количестве исследований %zu\nЭталонная разметка содержит %zu исследований\nА всего их было %zu",
// 			 tested_studies.size(), etalon_studies.size(), total_studies.size()));
}

XRayStudyProtocolMaker::XRayStudyProtocolMaker(
		const vector<wstring> &roi_filenames_csv, 
		const vector<wstring> &roi_filenames_json,
		const wstring &in_protocol_path, 
		const wstring in_preview_path, 
		const wstring &in_default_diagnosis, 
		bool in_make_previews,
		double	in_preview_threshold,
		const vector<wstring> &roi_id_correspondence_files):
	make_previews(in_make_previews),
	export_protocol_path(in_protocol_path), 
	export_preview_path(in_preview_path),
	default_diagnosis(in_default_diagnosis),
	preview_confidence_threshold(in_preview_threshold)
{
	
	for(auto &dcf: roi_id_correspondence_files)
	{
		auto	addition  =  LoadDiagnosesCorrespondenceFile(dcf);
		roi_id_correspondence.insert(addition.begin(), addition.end());
	}
	
	if(roi_id_correspondence.empty())
	{
		// если таблиц соответствия не найдено, значит, нужно интерпретировать индексы по прямому значению
		roi_id_correspondence[L"generic"] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25 };
	}
	else 
	{
		// иначе исследования, для которых не найдено соответствия, помечаются как ошибочные
		roi_id_correspondence[L"generic"] = { -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2 };
	}
	
//	auto	rois_csv = LoadPrimaryROIFiles_csv(roi_filenames_csv);
	auto	testee_rois_csv = LoadTesteeROIFile_csv(roi_filenames_csv[0]);
	auto	etalon_rois_json = LoadROIFiles_json(roi_filenames_json);

	AnalyzeROIsReport(testee_rois_csv, L"Testee");

	total_rois.insert(total_rois.end(), testee_rois_csv.begin(), testee_rois_csv.end());
	total_rois.insert(total_rois.end(), etalon_rois_json.begin(), etalon_rois_json.end());

	// к файл экспорта сразу записывается заголовок таблицы
	csv_protocol += XRayStudyTaggingReport::export_csv_row(xray::csv_header());
}


const vector<int>	&GetRoiIDCorrespondence(const roi_id_correspondence_t &roi_id_correspondence, const wstring &accession_number)
{
	auto	it = roi_id_correspondence.find(accession_number);
	if(it != roi_id_correspondence.end()) return it->second;
	
	it = roi_id_correspondence.find(L"generic");
	if(it != roi_id_correspondence.end()) return it->second;
	else throw invalid_argument("GetRoiIDCorrespondence, unknown ID correspondence");
}



void XRayStudyProtocolMaker::Apply(study_loader &study, ProgressProxy pp)
{
	std::lock_guard<std::mutex>	lock(m);
	previews.clear();

	auto frame_protocol_maker = make_shared<XRayFrameProtocolMaker>(
		total_rois, 
		previews, 
		make_previews, 
		preview_confidence_threshold,
		GetRoiIDCorrespondence(roi_id_correspondence, study.complete_study_id().accession_number()));
	StudyProcessorRecursive<study_loader>	sp(frame_protocol_maker);
	sp.Apply(study, pp);

	StudyTaggingReport_ptr protocol_p(new XRayStudyTaggingReport(study, *frame_protocol_maker, default_diagnosis));

	XRayStudyTaggingReport	&xprotocol = dynamic_cast<XRayStudyTaggingReport &>(*protocol_p);
	

	wstring	key = study.complete_study_id().accession_number() + L"_" + study.complete_study_id().study_id() + L"_" + primitive_string_hash(study.complete_study_id().study_instance_uid());

	auto n = export_protocol_path + key + L".json";
	utf8_ofstream	stream(n);
	stream << xprotocol.export_json();
	csv_protocol += xprotocol.export_csv_rows();

	for(auto &p: previews)
	{
		auto	path = export_preview_path;
		path += study.complete_study_id().accession_number() + L"_" + study.complete_study_id().study_id() + L"/";
		CreatePath(path);
		auto filename = path + p.first + L".jpg";
		SaveRasterImage(filename, p.second);
	}
	previews.clear();

	set<wstring>	tagging_systems;
	for(auto &frame: xprotocol.frames_protocols)
	{
		for(auto &roi: frame.instance_rois)
		{
			tagging_systems.insert(roi.tagging_system_id);
		}
	}
	for(auto &system_id: tagging_systems)
	{
		protocol_p->tagging_systems.push_back(tagging_system_identity(system_id, L"no comment"));
	}

	reports.push_back(protocol_p);
}


XRAD_END

