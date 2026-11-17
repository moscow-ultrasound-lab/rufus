#include "pre.h"
#include <FanTomCommons/Protocol/XRayFrameProtocolMaker.h>
#include <FanTomCommons/XRayRoiLabels.h>


/*!
	\file
	\date 2019/11/15 15:23
	\author kulberg

	\brief 
*/

XRAD_BEGIN

std::mutex XRayFrameProtocolMaker::m;

void XRayFrameProtocolMaker::Apply(instance_ptr &instance_p, ProgressProxy pp)
{
	auto	full_filename_with_path = instance_p->instance_storage()->print();
	wstring	dicom_name_with_extension;
	SplitFilename(full_filename_with_path, nullptr, &dicom_name_with_extension, nullptr, nullptr);

	instance_open_close_class open_close(*instance_p);


	ColorImageUI8	preview;

	if(make_previews)
	{
		Dicom::image &img = dynamic_cast<Dicom::image &>(*instance_p);
		float	wc = img.get_double(e_window_center);
		float	ww = img.get_double(e_window_width);
		auto	pi = img.get_wstring(e_photometric_interpretation);
		bool	negative = pi==(L"MONOCHROME1");

		auto	initial = img.get_image();
		preview = GeneratePreview(initial, wc, ww, negative);
	}

	int	whole_frame_roi_no = 0;

	xray_frame_protocol	frame_protocol;
	frame_protocol.init(instance_p, instance_rois);
	for(auto &roi: frame_protocol.instance_rois)
	{
		if(!roi.roi_correspondence_applied)
		{
			roi.roi_type_index = roi_id_correspondence[roi.roi_type_index];
			roi.roi_correspondence_applied = true;
		}
		if(make_previews && max(roi.roi_confidence, roi.study_confidence) >= preview_confidence_threshold)
		{
			if(roi.automatic_tagging)
			{
				if(roi.roi_confidence > 0 && roi.loc.width() && roi.loc.height())
				{
					DrawDottedEllipse(preview, roi.loc, roi.color(), 8, 3);
				}
				else if(roi.study_confidence > 0)
				{
					// Если патология посчитана на все исследование целиком, рисуем эллипс размером во весь кадр. 
					// Чтобы эллипсы не закрывали друг друга, делаем их вложенными
					int	c = ++whole_frame_roi_no * 8;
					range2_I32	total_loc(c,c,int(preview.vsize())-c, int(preview.hsize())-c);
					DrawDottedEllipse(preview, total_loc, roi.color(), 8, 3);
				}
			}
			else
			{
				DrawDottedEllipse(preview, roi.loc, roi.color(), 4, 0.5);
			}
		}
	}
	std::lock_guard<std::mutex>	lock(m);
	frames_rois_records.push_back(frame_protocol);
	if(make_previews) previews[dicom_name_with_extension] = preview;
//	else previews.clear();
}


void xray_frame_protocol::init(instance_ptr &instance_p, vector<XRayRoiTaggingReport> &rois_total)
{
	auto	full_filename_with_path = instance_p->instance_storage()->print();
	wstring	path, dicom_name_with_extension, name_without_extension;
	SplitFilename(full_filename_with_path, &path, &dicom_name_with_extension, &name_without_extension, nullptr);

	dcm_filename = dicom_name_with_extension;
	study_id = instance_p->study_id();
	study_instance_uid = instance_p->study_instance_uid();

	acquisition_number = int(instance_p->acquisition_number());
	instance_number = instance_p->instance_number();
	sop_instance_uid = instance_p->sop_instance_uid();
	series_number = int(instance_p->series_number());


	instance_rois.clear();
	auto	rois_it = rois_total.begin();
	while(rois_it < rois_total.end())
	{
		auto cond = [&](const XRayRoiTaggingReport &roi)
		{
			return roi.dcm_filename==name_without_extension || roi.dcm_filename==dicom_name_with_extension;
		};

		rois_it = find_if(rois_it, rois_total.end(), cond);
		if(rois_it != rois_total.end())
		{			
			adjust_common_fields(*rois_it);
			instance_rois.push_back(*rois_it);
			++rois_it;
		}
	}
}


nlohmann::json xray_frame_protocol::export_json() const
{
	nlohmann::json	result;
	result[xray::dcm_filename_tag()] = convert_to_string8(dcm_filename);
	result[xray::acquisition_number_tag()] = acquisition_number;
	result[xray::instance_number_tag()] = instance_number;
	result[xray::sop_instance_uid_tag()] = convert_to_string8(sop_instance_uid);
	result[xray::series_number_tag()] = series_number;
	result[xray::rois_tag()] = nlohmann::json();
	//vector<XRayRoiTaggingReport> rois;
	int	number(0);
	for(auto &rois_it: instance_rois)
	{
		result[xray::rois_tag()][ssprintf("%02d", ++number)] = rois_it.export_json();
	}
	return result;
}


XRAD_END

