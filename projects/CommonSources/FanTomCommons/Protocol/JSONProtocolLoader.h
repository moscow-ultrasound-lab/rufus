#ifndef JSONProtocolLoader_h__
#define JSONProtocolLoader_h__

/*!
	\file
	\date 2018/07/18 16:26
	\author kulberg

	\brief  
*/

#include <FanTomCommons/TaggingReport/StudySetTaggingReport.h>

XRAD_BEGIN

StudyTaggingReport_ptr	StructuredStudyFromJson(const json &j);

StudySetTaggingReport	StructuredReportFromJSONFolder(const wstring &report_path, bool subfolders);


XRAD_END

#endif // JSONProtocolLoader_h__
