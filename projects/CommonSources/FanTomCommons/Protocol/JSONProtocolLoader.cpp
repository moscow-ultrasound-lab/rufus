#include "pre.h"
#include "JSONProtocolLoader.h"
#include <FanTomCommons/NodulesClusters.h>
#include <fstream>
#include <FanTomCommons/JoinFindings.h>


/*!
	\file
	\date 2018/07/18 16:26
	\author kulberg

	\brief 
*/

XRAD_BEGIN


nodule_t	NoduleFromJson(const json &jnodule, const vector<tagging_system_identity> doctors)
{
// 	nodule_t	result(doctor_ids);
	nodule_t	result;

// 	for(auto j = jnodule.begin(); j != jnodule.end(); ++j)
	for(auto &j : jnodule.items())
	{
		auto &jfinding = j.value();
		auto &jkey = j.key();

		TomogramFinding	f = json_to_finding(string8_to_wstring(jkey), jfinding);
		if(!f.empty()) result.add_finding(f);
	}
	return result;
}

nodules_cluster	NodulesClusterFromJson(const json &jcluster, const vector<tagging_system_identity> doctors)
{
	nodules_cluster	result;
	for(auto &jnodule: jcluster)
	{
		result.push_back(NoduleFromJson(jnodule, doctors));
	}
	return result;
}

StudyTaggingReport_ptr	StructuredStudyFromJson(const json &j)
{
	Dicom::complete_study_id_t ids;
	vector<tagging_system_identity> doctors;

	ids.accession_number() = string8_to_wstring(j["ids"]["accession number"]);
	ids.study_id() = string8_to_wstring(j["ids"]["study id"]);

	for(auto jdoc: j["doctors"])
	{
		doctors.push_back(tagging_system_identity(string8_to_wstring(jdoc["id"]), string8_to_wstring(jdoc["comment"])));
// 		doctor_ids.push_back(string8_to_wstring(jdoc["id"]));
// 		doctor_comments.push_back(string8_to_wstring(jdoc["comment"]));
	}

// 	structured_study_protocol	result(ids, doctor_ids, doctor_comments);
	

	StudyTaggingReport_ptr	result(new TomogramTaggingReport(ids, doctors));
	auto &r = dynamic_cast<TomogramTaggingReport &>(*result);

//	TomogramTaggingReport	result(ids, doctors);
	for(auto jcluster: j["nodules"])
	{
		r.push_back(NodulesClusterFromJson(jcluster, doctors));
	}
//	JoinClusters(result);
	ReassembleClusters(r);
	return result;
}


StudyTaggingReport_ptr	StructuredStudyFromJsonFile(const wstring &filename)
{
	ifstream	json_file(filename);
	json	j;
	json_file >> j;

	return StructuredStudyFromJson(j);
}

StudySetTaggingReport	StructuredReportFromJSONFolder(const wstring &report_path, bool subfolders)
{
	StudySetTaggingReport	result;
	wstring filter =L"*.json";

	vector<wstring> names = GetDirectoryFiles(report_path, filter, subfolders);

	for(auto &name: names)
	{
		try
		{
			result.push_back(StructuredStudyFromJsonFile(name));
		}
		catch(...)
		{
		}
	}

	return result;
}


XRAD_END

