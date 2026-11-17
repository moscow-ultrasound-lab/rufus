#ifndef DumpReport_json_h__
#define DumpReport_json_h__

/*!
	\file
	\date 2018/07/12 13:51
	\author kulberg

	\brief  
*/

#include <FanTomCommons/TaggingReport/TomogramTaggingReport.h>

XRAD_BEGIN

void	DumpReport_json(const vector<StudyTaggingReport_ptr> &report, const wstring &report_filename, const wstring &title);
wstring	DumpStudy_json(const StudyTaggingReport &study, const wstring &json_path);
//json	structured_study_to_json(const StudyTaggingReport &study_original);
json	structured_study_to_json(const TomogramTaggingReport &study_original);


XRAD_END

#endif // DumpReport_json_h__
