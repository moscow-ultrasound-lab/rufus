#include "pre.h"
#include <XRADSystem/Sources/TextFile/text_file.h>
#include "TomogramTaggingReport.h"
#include <FanTomCommons/NodulesClusters.h>
#include <FanTomCommons/JoinFindings.h>
/*!
	\file
	\date 2018/01/19 17:51
	\author kulberg

	\brief 
*/

XRAD_BEGIN





void	AdjustFindingsTypes(TomogramTaggingReport &result)
{
	auto	correct = [](TomogramFinding &finding, wstring correct, const list<wstring> &bads)
	{
	for(auto &bad: bads)
	{
		if(finding.type()==bad)
		{
			finding.type()=correct;
		}
	}
	};

	for(auto &cluster: result)
	{
		for(auto &column: cluster)
		{
			for(auto &finding: column)
			{
				correct(finding.second, L"с", {L"C"/*LAT*/, L"С"/*РУС*/, L"c"/*l*/, L"c "/*l */, L"с "/*р */, L"c  "/*l  */, L"с  "/*р  */, L"‘", L"с2"});
				correct(finding.second, L"п", {L"n", L" п", L"П", L"g", L"пс"});
				correct(finding.second, L"м", {L"v", L"g", L"M"/*LAT*/, L"М"/*РУС*/, L"m"});
				correct(finding.second, L"unknown", {L"", L"x", L"X"});

				if(!finding.second.type().size()) finding.second.type()=L"unknown";//опечатка, восстановить не удается
				if(finding.second.type()==L"X") finding.second.type()=L"unknown";//опечатка, восстановить не удается
				if(finding.second.type()==L"x") finding.second.type()=L"unknown";//опечатка, восстановить не удается

			}
		}
	}
}


void	AdjustRepeatedDoctorIds(one_study_findings_t &one_study_findings)
//	Важная добавка: если исследования просматривали разные доктора, работавшие под одним ID,
//	их находки могли сливаться неправильно. Во избежание такого повторы ID принудительно
//	исключаются
{
	for(auto it = one_study_findings.begin(); it != one_study_findings.end(); ++it)
	{
		auto it2 = it;
		++it2;
		for(; it2 != one_study_findings.end(); ++it2)
		{
			if(it->doctor_id==it2->doctor_id)
			{
				it2->SetDoctorID(it2->doctor_id + L"+");
			}
		}
	}
}


TomogramTaggingReport AdjustFindings(one_study_findings_t &&one_study_findings)
{
	one_doctor_one_study_findings_t &first = one_study_findings.front();
	for(auto &study_findings: one_study_findings)
	{
		// эта процедура согласует находки разных докторов только в одном исследовании, разные ей ни к чему
		XRAD_ASSERT_THROW_M(study_findings.ids == first.ids, invalid_argument, "Mixed studies in one group");
	}
	AdjustRepeatedDoctorIds(one_study_findings);
	TomogramTaggingReport result = JoinSimilarFindings(one_study_findings);
	AdjustFindingsTypes(result);
	return result;
}




StudyTaggingReport_ptr GenerateOneStudyReport(const reports_heap_t protocol, const Dicom::complete_study_id_t &id)
{
	one_study_findings_t one_study_findings(id);
	for(auto &study_findings: protocol)
	{
		if(study_findings.ids==id)
		{
			one_study_findings.push_back(study_findings);
		}
	}

	auto	*r = new TomogramTaggingReport;
	*r	= AdjustFindings(move(one_study_findings));

	return StudyTaggingReport_ptr(r);
}

TomogramTaggingReport &TomogramTaggingReport::operator +=(const TomogramFinding &finding)
{
#if 0
	bool	inserted = false;
	for(auto &cluster: *this)
	{
		//inserted = cluster.add_finding(other);
		if(inserted) break;
	}
	if(!inserted)
	{
		//push_back() new cluster, resort by z
		sort_clusters_z();
	}
#endif
	return *this;
}


TomogramTaggingReport &TomogramTaggingReport::operator +=(const TomogramTaggingReport &other)
{
	if(ids != other.ids) throw invalid_argument("Trying to merge findings from different studies");

	// Добавляем список докторов из второго протокола
	for(size_t i = 0; i < other.tagging_systems.size(); ++i)
	{
		auto	it = find_if(tagging_systems.begin(), tagging_systems.end(), [&other, &i](const tagging_system_identity &d){return d.id() == other.tagging_systems[i].id();});
		if(it==tagging_systems.end())
		{
			tagging_systems.push_back(other.tagging_systems[i]);
		}
		else
		{
			tagging_systems[i].comment() += (L" | " + other.tagging_systems[i].comment());
		}
	}
	// если доктора из второго протокола ничего не нашли, выходим
	if(other.empty()) return *this;

	for(auto &other_cluster: other)
	{
		push_back(other_cluster);
	}
	ReassembleClusters(*this);
	return *this;
}

void TomogramTaggingReport::rename_doctor(const wstring &new_doctor_id, const wstring &old_doctor_id)
{
	// 		auto it = find(doctor_ids.begin(), doctor_ids.end(), old_doctor_id);
	auto it = find_if(tagging_systems.begin(), tagging_systems.end(), [&old_doctor_id](const tagging_system_identity &doc){return doc.id() == old_doctor_id;});
	if(it != tagging_systems.end()) it->id() = new_doctor_id;

	for(auto &cluster: *this)
	{
		cluster.rename_doctor(new_doctor_id, old_doctor_id);
	}
}



void TomogramTaggingReport::sort_clusters_z()
{
	std::sort(begin(), end(), clusters_z_compare);
}


XRAD_END

