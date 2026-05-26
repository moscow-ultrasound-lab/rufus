#ifndef ProtocolReport_h__
#define ProtocolReport_h__

/*!
	\file
	\date 2018/01/16 10:39
	\author kulberg

	\brief  
*/

#include <FanTomCommons/NodulePredicate.h>

XRAD_BEGIN

//struct TomogramSetTaggingReport : public vector<TomogramTaggingReport>
struct StudySetTaggingReport : public vector<StudyTaggingReport_ptr>
{
	PARENT(vector<StudyTaggingReport_ptr>);
	using self = StudySetTaggingReport;
	using parent::parent;

	set<Dicom::complete_study_id_t> GetStudiesList(const wstring &doctor_id) const
	{
		set<Dicom::complete_study_id_t>	result;
		for(auto &study: *this)
		{
			if(doctor_id.empty() || study->tagging_system_participates(doctor_id))
			{
				result.insert(study->ids);
			}
			else
			{
				//ForceDebugBreak();
			}
		}
		return result;
	}

	void	rename_doctor(const wstring &new_doctor_id, const wstring &old_doctor_id)
	{
		for(auto &report: *this)
		{
			report->rename_doctor(new_doctor_id, old_doctor_id);
		}
	}

	set<wstring> doctor_ids() const 
	{
		set<wstring>	result;
		for(auto &study_report: *this)
		{
			for(auto &tagging_system : study_report->tagging_systems)
			{
				result.insert(tagging_system.id());
			}
		}
		return result;
	}

	self &operator += (const self &r2);
};



inline vector<wstring> GetTaggingSystemIDs(const StudySetTaggingReport &study_set_report)
{
	set<wstring>	result;
	for(auto &study_report: study_set_report)
	{
		for(auto &doctor: study_report->tagging_systems)
		{
			result.insert(doctor.id());
		}
	}
	return vector<wstring>(result.begin(), result.end());
}


/*
using structured_report = vector<structured_study_protocol>;
*/


StudySetTaggingReport	GenerateStructuredProtocol(const reports_heap_t &raw_protocol);
StudySetTaggingReport FilterStructuredReport(const StudySetTaggingReport &report, const nodule_predicate &pred);
StudySetTaggingReport GetClearStudies(const StudySetTaggingReport &report);

void	EraseExcludeList(StudySetTaggingReport &report, const vector<Dicom::complete_study_id_t> &exclude_ids);

StudySetTaggingReport	SelectTopViews(const StudySetTaggingReport &report, const wstring &target_path, size_t N = 550);


void	DumpSummary(const StudySetTaggingReport &total, const wstring &target_path, const wstring &filename);

//void	DumpProtocolReport(reports_heap_t &protocol, const wstring &report_path, nodule_predicate &pred);
void	DumpStructuredReport(const StudySetTaggingReport &report, const wstring &report_path, wstring title);


XRAD_END

#endif // ProtocolReport_h__
