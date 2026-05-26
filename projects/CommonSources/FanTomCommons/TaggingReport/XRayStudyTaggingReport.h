#ifndef XRayStudyTaggingReport_h__
#define XRayStudyTaggingReport_h__

/*!
	\file
	\date 2019/12/11 4:48
	\author kulberg

	\brief  
*/

#include <XRADBasic/ThirdParty/nlohmann/json.hpp>
#include <FanTomCommons/Identity.h>
#include <FanTomCommons/TaggingReport/StudyTaggingReport.h>
#include <FanTomCommons/Protocol/XRayFrameProtocolMaker.h>


XRAD_BEGIN



using namespace Dicom;

struct XRayStudyTaggingReport : public StudyTaggingReport, public study_identity
{
	XRayStudyTaggingReport(const study_loader &study, const XRayFrameProtocolMaker &ipm, const wstring &in_default_diagnosis);
	XRayStudyTaggingReport(const XRayStudyTaggingReport &other) = default;


	wstring	diagnosis;

	bool	contains_rois;

	//const vector<xray_frame_protocol>	&global_frames_protocols;
	vector<xray_frame_protocol>	frames_protocols;

	nlohmann::json	export_json();
	static wstring	export_csv_row(const vector<wstring> &row);
	wstring	export_csv_rows() const;
	//vector<wstring>	generate_export_vector(const XRayRoiTaggingReport &roi) const;

	//
	virtual	StudyTaggingReport *clone() const override
	{ 
		return new XRayStudyTaggingReport(*this); 
	};

	virtual void	rename_doctor(const wstring &new_doctor_id, const wstring &old_doctor_id) {/*dummy, recreate*/ }
	virtual TaggingReportModality modality() const override { return TaggingReportModality::xray; }
	virtual	bool	empty() const override;

};


XRAD_END

#endif // XRayStudyTaggingReport_h__
