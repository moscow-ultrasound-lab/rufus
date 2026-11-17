#ifndef ExtractROIShape_h__
#define ExtractROIShape_h__

/*!
	\file
	\date 2019/11/08 18:07
	\author kulberg

	\brief  
*/

#include <XRADSystem/Sources/TextFile/text_file.h>
#include <FanTomCommons/TaggingReport/XRayRoiTaggingReport.h>

XRAD_BEGIN

enum roi_file_type
{
	e_general_table, e_partial_table
};

using roi_id_correspondence_t = map<wstring, vector<int>>;

const vector<int>	&GetRoiIDCorrespondence(const roi_id_correspondence_t &roi_id_correspondence, const wstring &accession_number);

vector<XRayRoiTaggingReport>	LoadPrimaryROIFiles_csv(const vector<wstring> &roi_filenames);
vector<XRayRoiTaggingReport>	LoadTesteeROIFile_csv(const wstring &roi_filename);


vector<XRayRoiTaggingReport>	LoadROIFiles_json(const vector<wstring> &json_filenames);
map<wstring, vector<int>> LoadDiagnosesCorrespondenceFile(const wstring &diagnoses_correspondence);

//vector<point2_I32>	ParseRawROIFile(text_file_reader &file);

XRAD_END

#endif // ExtractROIShape_h__
