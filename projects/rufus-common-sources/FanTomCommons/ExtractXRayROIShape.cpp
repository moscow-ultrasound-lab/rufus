#include "pre.h"
#include "ExtractXRayROIShape.h"
#include "TabDelimitedFile.h"
#include <FanTomCommons/XRayRoiLabels.h>
#include <iostream>

/*!
	\file
	\date 2019/11/08 18:07
	\author kulberg

	\brief 
*/

XRAD_BEGIN


vector<point2_I32>	ParseRawROITable(const wstring &buffer)
{
	vector<point2_I32>	result;

	auto	number_start = [](wchar_t c)
	{
		if(!in_range(c, 0, 255)) return false;
		if(isdigit(c)) return true;
		if(c==L'+' || c==L'-') return true;
		return false;
	};
	auto	ie = buffer.end();
	auto	it = find_if(buffer.begin(), ie, number_start);

	while(it<ie)
	{
		auto	space = std::find_if(it, ie, [](wchar_t c){return c==L' ' || c==L'\t';});
		auto	lf = std::find_if(space, ie, [](wchar_t c){return c==L'\n' || c==L'\r';});
		wstring	x(it, space);
		wstring	y(space, lf);
		it = find_if(lf, ie, number_start);

		point2_I32	p;
		auto	counter = swscanf(x.c_str(), L"%d", &p.x());
		counter += swscanf(y.c_str(), L"%d", &p.y());

		if(counter == 2) result.push_back(p);
	}
	return result;
}

vector<point2_I32>	ParseRawROIFile(text_file_reader &file)
{
	wstring	buffer;
	file.read(buffer);

	return ParseRawROITable(buffer);
}

range2_I32	GetROIRanges(const vector<point2_I32> &points)
{
	XRAD_ASSERT_THROW_EX(points.size(), invalid_argument);
	range2_I32	result(points[0], points[0]);

	for(auto &p: points)
	{
		if(result.x1() > p.x()) result.x1() = p.x();
		if(result.y1() > p.y()) result.y1() = p.y();
		if(result.x2() < p.x()) result.x2() = p.x();
		if(result.y2() < p.y()) result.y2() = p.y();
	}
	return result;
}

roi_file_type DetectROIFileType(const wstring &fn)
{
	cells_type	roi = LoadTabDelimitedFile(fn, true);
	if(!roi.size()) throw invalid_argument(ssprintf("ROI file '%s' is incorrect", convert_to_string(fn).c_str()));
	if(roi[0].size() == 1)
	{
		return e_partial_table;
	}
	else if(roi[0].size() == xray::n_csv_table_columns())
	{
		return e_general_table;
	}
	else throw invalid_argument(ssprintf("ROI file '%s' is incorrect", convert_to_string(fn).c_str()));
}

e_anatomical_location	anatomical_location_from_string(const string &aloc)
{
	if(aloc == "mmg") return e_anatomical_location::mmg;
	if(aloc == "lung") return e_anatomical_location::lung;
	throw invalid_argument("invalid anatomic location");
}


vector<XRayRoiTaggingReport>	LoadROIFiles_json(const vector<wstring> &json_filenames)
{
	vector<XRayRoiTaggingReport>	rois;
	for(auto &filename: json_filenames)
	{
		try
		{
			nlohmann::json	j;
			ifstream	json_file(filename);
			json_file >> j;


			study_identity	si;
			si.accession_number = string8_to_wstring(j[xray::accession_number_tag()]);
			si.study_id = string8_to_wstring(j[xray::study_id_tag()]);
			si.study_instance_uid = string8_to_wstring(j[xray::study_instance_uid_tag()]);
			try
			{
				si.study_confidence = j[xray::study_confidence_tag()];
			}
			catch(...)
			{
				si.study_confidence = 0;
			}
			//diagnosis????

			size_t	frame_counter = 0;

			for(auto &instance: j[xray::instances_tag()])
			{

				XRayFrameIdentity	xfi;

				xfi.adjust_common_fields(si);

				xfi.dcm_filename = string8_to_wstring(instance[xray::dcm_filename_tag()]);
				xfi.sop_instance_uid = string8_to_wstring(instance[xray::sop_instance_uid_tag()]);
				xfi.instance_number = instance[xray::instance_number_tag()];
				xfi.frame_number = ++frame_counter;

				xfi.acquisition_number = instance[xray::acquisition_number_tag()];
				xfi.series_number = instance[xray::series_number_tag()];
				int	roi_no_counter = 0;
				for(auto &jroi: instance[xray::rois_tag()])
				{
					XRayRoiTaggingReport	roi;
					roi.adjust_common_fields(xfi);

					//roi identity
					roi.roi_filename_with_extension = string8_to_wstring(jroi[xray::roi_filename_tag()]);

					try
					{
						roi.automatic_tagging = jroi[xray::automatic_tagging_tag()];
					}
					catch(...)
					{
						roi.automatic_tagging = false;
					}
					roi.tagging_system_id = string8_to_wstring(jroi[xray::tagging_system_id_tag()]);
					
					//roi content
					roi.roi_type_index = jroi[xray::roi_type_index_tag()];
					roi.roi_no = ++roi_no_counter;
					roi.anatomical_location = anatomical_location_from_string(jroi[xray::anatomical_location_tag()]);

					int	y = jroi[xray::y_tag()];
					int	x = jroi[xray::x_tag()];
					int	y_size = jroi[xray::y_size_tag()];
					int	x_size = jroi[xray::x_size_tag()];

					roi.loc = range2_I32(y-y_size/2,x-x_size/2,y+y_size/2,x+x_size/2);
					try
					{
						roi.roi_confidence = jroi[xray::roi_confidence_tag()];
					}
					catch(...)
					{
						roi.roi_confidence = 1;
						// json пришел от врача. Это поле вместе c automatic_tagging
					}

					// в разметке врачей не было отдельного прогноза для исследования, из-за этого возникала путаница. Исправляем.
					if(roi.study_confidence == 0) roi.study_confidence = roi.roi_confidence;
					roi.roi_correspondence_applied = true;
				
					if(RoiErrataStatus()) roi.Distort();
					rois.push_back(roi);
				}
			}
		}
		catch(...)
		{
			cout << "\njson file error " + convert_to_string(filename) << "\n";
			fflush(stdout);
		}
	}
	return rois;
}


vector<XRayRoiTaggingReport>	LoadPrimaryROIFiles_csv(const vector<wstring> &roi_filenames)
{
	vector<XRayRoiTaggingReport>	rois;
	for(auto &filename: roi_filenames)
	{
		try
		{
			if(DetectROIFileType(filename) == e_partial_table)
			{
				XRayRoiTaggingReport	roi;
				roi.init_from_primary_roi_file(filename);
				text_file_reader	file(filename, text_encoding::recognize_encoding_content);
				roi.loc	= GetROIRanges(ParseRawROIFile(file));

				if(RoiErrataStatus()) roi.Distort();
				rois.push_back(roi);
			}
		}
		catch(...)
		{
			printf(GetExceptionStringOrRethrow().c_str());
			printf("\n");
			fflush(stdout);
		}
	}
	return rois;

}


vector<XRayRoiTaggingReport>	LoadTesteeROIFile_csv(const wstring &filename)
{
	vector<XRayRoiTaggingReport>	rois;
	try
	{
		if(DetectROIFileType(filename) != e_partial_table)
		{
			auto	cells = LoadTabDelimitedFile(filename, true);

			XRAD_ASSERT_THROW(cells.size() != 0);
			XRAD_ASSERT_THROW(cells[0].size() == xray::n_csv_table_columns());
				
			for(auto &row: cells)
			{
				XRayRoiTaggingReport	roi;
				roi.import_csv_row(filename, row);

				if(RoiErrataStatus()) roi.Distort();
				rois.push_back(roi);
			}
		}
	}
	catch(...)
	{
		printf(GetExceptionStringOrRethrow().c_str());
		printf("\n");
		fflush(stdout);
	}
	return rois;
}

map<wstring, vector<int>> LoadDiagnosesCorrespondenceFile(const wstring &diagnoses_correspondence_filename)
{
	map<wstring, vector<int>> result;
	try
	{
		auto	table = LoadTabDelimitedFile(diagnoses_correspondence_filename, true);

		for(size_t i = 1; i < table.size(); ++i)
		{
			if(table[i].size() >= 3)
			{
				auto &v = result[table[i][1]] ={0};
				for(size_t j = 2; j < table[i].size(); ++j)
				{
					try
					{
						v.push_back(wcstol(table[i][j].c_str(), NULL, 10));
					}
					catch(...){}
				}
			}
		}
	}
	catch(file_container_error &){}

	return result;
}

XRAD_END

