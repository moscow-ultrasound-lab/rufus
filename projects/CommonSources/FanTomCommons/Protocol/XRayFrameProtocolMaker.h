#ifndef InstanceProtocolMaker_h__
#define InstanceProtocolMaker_h__

/*!
	\file
	\date 2019/11/15 15:23
	\author kulberg

	\brief  
*/

#include <XRADDicom/XRADDicom.h>
#include <FantomCommons/TaggingReport/XRayRoiTaggingReport.h>
#include <FantomCommons/TaggingReport/XRayRoiPreview.h>
#include <FanTomCommons/Identity.h>


XRAD_BEGIN

using namespace Dicom;

struct	xray_frame_protocol : public XRayFrameIdentity
{
	vector<XRayRoiTaggingReport> instance_rois;
	void init(instance_ptr &instance_p, vector<XRayRoiTaggingReport>	&rois_total);
	nlohmann::json	export_json() const;
};





class XRayFrameProtocolMaker : public InstanceProcessor<instance_ptr>
{
public:
	XRayFrameProtocolMaker(
		vector<XRayRoiTaggingReport> 	&in_rois, map<wstring, ColorImageUI8> &in_previews,
		bool in_make_previews, 
		double in_preview_confidence_threshold,
		const vector<int>	&in_roi_id_correspondence) :
		instance_rois(in_rois),
		previews(in_previews),
		make_previews(in_make_previews),
		preview_confidence_threshold(in_preview_confidence_threshold),
		roi_id_correspondence(in_roi_id_correspondence)
	{}

	vector<xray_frame_protocol>	frames_rois_records;
	map<wstring, ColorImageUI8> &previews;

	bool	contains_roi() const
	{
		bool	contains_roi_flag(false);
		for(auto &rec: frames_rois_records) contains_roi_flag |= !rec.instance_rois.empty();
		return contains_roi_flag;
	}

protected:
private:
	const vector<int>	&roi_id_correspondence;
	vector<XRayRoiTaggingReport>	&instance_rois;

	virtual	void Apply(instance_ptr &instance_p, ProgressProxy pp);

	static	std::mutex m;
	const bool	make_previews;
	const double preview_confidence_threshold;
};


XRAD_END

#endif // InstanceProtocolMaker_h__
