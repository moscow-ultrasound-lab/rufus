#ifndef DIT_ROC_h__
#define DIT_ROC_h__

/*!
	\file
	\date 2018/04/20 15:00
	\author kulberg

	\brief  
*/

//#include "Nodule.h"
#include <FanTomCommons/TaggingReport/StudySetTaggingReport.h>

XRAD_BEGIN

//void	ComputeROC(const findings_report_t	&total);
void	ComputeROC(const StudySetTaggingReport	&total, wstring target_folder, wstring testee_id, double cutoff = 0);

XRAD_END

#endif // DIT_ROC_h__
