#ifndef OneStudyProtocol_h__
#define OneStudyProtocol_h__

/*!
	\file
	\date 2018/01/19 17:51
	\author kulberg

	\brief  
*/

#include <XRADSystem/Sources/TextFile/text_file.h>

#include <FanTomCommons/Identity.h>
#include <FanTomCommons/Nodule.h>
#include <FanTomCommons/TaggingReport/StudyTaggingReport.h>

#include <FanTomCommons/NodulesClusters.h>
#include <FanTomCommons/FindingsMetrics.h>

XRAD_BEGIN



struct TomogramTaggingReport : public vector<nodules_cluster>, public StudyTaggingReport
{
	PARENT(StudyTaggingReport);

	virtual TaggingReportModality modality() const override
	{
		return TaggingReportModality::tomogram;
	}

	virtual	StudyTaggingReport *clone() const override
	{
		return new TomogramTaggingReport(*this);
	}

	virtual	bool	empty() const override
	{
		return vector<nodules_cluster>::empty();
	}

	TomogramTaggingReport() {}

	TomogramTaggingReport(const Dicom::complete_study_id_t &in_ids, const vector<tagging_system_identity> &in_doctors) :
		parent(in_ids, in_doctors)
	{
	}



	virtual void	rename_doctor(const wstring &new_doctor_id, const wstring &old_doctor_id) override;

	TomogramTaggingReport &operator +=(const TomogramTaggingReport &other);
	TomogramTaggingReport &operator +=(const TomogramFinding &finding);

	void	sort_clusters_z();
};



//!	Выбор протоколов, относящихся к одному исследованию и их предобработка (объединение сходных очагов, создание кластеров)
StudyTaggingReport_ptr	GenerateOneStudyReport(const reports_heap_t protocol, const Dicom::complete_study_id_t &id);

//!	Повторное объединение очагов в кластеры (возможно после корректировки исследования экспертом, 
//!	например, если разные врачи указали один и тот же очаг с использованием различных координат по z
//void	JoinClusters(structured_study_protocol &study);


XRAD_END

#endif // OneStudyProtocol_h__
