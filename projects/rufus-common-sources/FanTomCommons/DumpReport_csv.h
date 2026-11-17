#ifndef DumpReport_h__
#define DumpReport_h__

/*!
	\file
	\date 2018/07/12 13:13
	\author kulberg

	\brief  
*/

#include <FanTomCommons/TaggingReport/TomogramTaggingReport.h>

XRAD_BEGIN


void	DumpReport_csv(const vector<StudyTaggingReport_ptr> &report, const wstring &report_path, wstring title);

XRAD_END

#endif // DumpReport_h__
