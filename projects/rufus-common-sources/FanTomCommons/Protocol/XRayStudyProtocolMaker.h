#ifndef ProtocolMaker_h__
#define ProtocolMaker_h__

/*!
	\file
	\date 2019/11/11 10:07
	\author kulberg

	\brief  
*/

#include <FanTomCommons/TaggingReport/StudySetTaggingReport.h>
#include <FanTomCommons/TaggingReport/XRayStudyTaggingReport.h>

XRAD_BEGIN


class XRayStudyProtocolMaker : public StudyProcessor<study_loader>
{
public:
	XRayStudyProtocolMaker(
		const vector<wstring> &roi_filenames_csv,
		const vector<wstring> &roi_filenames_json,
		const wstring &in_protocol_path, 
		const wstring in_preview_path, 
		const wstring &in_default_diagnosis, 
		bool in_make_previews,
		double	in_preview_threshold,
		const vector<wstring> &roi_id_correspondence_files);
	
	map<wstring, ColorImageUI8>		previews;

	StudySetTaggingReport	reports;


	wstring	csv_protocol;

private:
	const wstring	export_protocol_path, export_preview_path;
	vector<XRayRoiTaggingReport>	total_rois;
	virtual	void Apply(study_loader &study, ProgressProxy pp);
	const wstring	default_diagnosis;
	std::mutex	m;
	map<wstring, vector<int>> roi_id_correspondence;
	
	const bool	make_previews;
	const double preview_confidence_threshold;
	
//	const vector<int>	&GetRoiIDCorrespondence(const wstring &accession_number);
};



XRAD_END

#endif // ProtocolMaker_h__
