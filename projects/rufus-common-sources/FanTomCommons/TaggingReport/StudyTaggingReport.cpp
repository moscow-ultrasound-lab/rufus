#include "pre.h"
#include "StudyTaggingReport.h"
/*!
	\file
	\date 2019/12/11 4:21
	\author kulberg

	\brief 
*/

XRAD_BEGIN

bool StudyTaggingReport::tagging_system_participates(const wstring &tested_doctor_id) const
{
	auto	criterium = [&tested_doctor_id](auto &doctor) {return doctor.id() == tested_doctor_id; };
	if(find_if(tagging_systems.begin(), tagging_systems.end(), criterium) != tagging_systems.end())
		return true;
	return false;
}


XRAD_END

