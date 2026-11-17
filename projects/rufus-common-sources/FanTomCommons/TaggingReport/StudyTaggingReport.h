#ifndef StudyTaggingReport_h__
#define StudyTaggingReport_h__

/*!
	\file
	\date 2019/12/11 4:21
	\author kulberg

	\brief  
*/

#include <XRADDicom/XRADDicom.h>
#include <FanTomCommons/Identity.h>

XRAD_BEGIN


enum class TaggingReportModality
{
	tomogram, xray
};

struct StudyTaggingReport
{
	virtual ~StudyTaggingReport() = default; // Необходимо для корректной работы cloning_ptr
	virtual	StudyTaggingReport *clone() const = 0;
	virtual void	rename_doctor(const wstring &new_doctor_id, const wstring &old_doctor_id) = 0;
	virtual TaggingReportModality modality() const = 0;

	bool	tagging_system_participates(const wstring &tested_doctor_id) const;


	virtual	bool	empty() const = 0;

	Dicom::complete_study_id_t	ids;
	vector<tagging_system_identity> tagging_systems;

	StudyTaggingReport() {}

	StudyTaggingReport(const Dicom::complete_study_id_t &in_ids, const vector<tagging_system_identity> &in_doctors) :
		ids(in_ids),
		tagging_systems(in_doctors)
	{
	}


};

using StudyTaggingReport_ptr = cloning_ptr<StudyTaggingReport>;

XRAD_END

#endif // StudyTaggingReport_h__
