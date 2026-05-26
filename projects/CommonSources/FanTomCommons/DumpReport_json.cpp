#include "pre.h"
#include "DumpReport_json.h"
#include <XRADBasic/Sources/Utils/utf8_ofstream.h>


/*!
	\file
	\date 2018/07/12 13:51
	\author kulberg

	\brief 
*/

XRAD_BEGIN


json	nodule_to_json(const nodule_t &nodule, const vector<tagging_system_identity> &tagging_systems, bool force_negative_findings = false)
{
	json	result;

	for(auto &doctor: tagging_systems)
	{
		string8	doctor_id8 = convert_to_string8(doctor.id());
		auto	it = nodule.find(doctor.id());
		if(force_negative_findings)
		{
			// вариант, отмечающий неудавшиеся находки докторов
			// (если врач не обнаружил очаг, делаем запись null)
			result[doctor_id8] = it==nodule.end() ? 
				finding_to_json(TomogramFinding(doctor.id())) : 
				finding_to_json(it->second);
		}
		else if(it != nodule.end())
		{
			// вариант, отмечающий только полноценные находки
			result[doctor_id8] = finding_to_json(it->second);
		}
	}
	return move(result);
}


void AdjustDoctorIDs(TomogramTaggingReport &study)
{
	for(auto it = study.tagging_systems.begin(); it < study.tagging_systems.end(); ++it)
	{
		for(auto it2 = it+1; it2 < study.tagging_systems.end(); ++it2)
		{
			if(it2->id()==it->id()) it2->id() += L"+";
		}
	}
}

json	study_ids_to_json(const  Dicom::complete_study_id_t	&ids)
{
	json jids;
	jids["accession number"] = convert_to_string8(ids.accession_number());
	jids["study id"] = convert_to_string8(ids.study_id());
	return move(jids);
}



json	doctors_to_json(const vector<tagging_system_identity> &tagging_systems)
{
	auto doctors_data = [&tagging_systems](size_t no)
	{
		json	result;
		result["id"] = convert_to_string8(tagging_systems[no].id());
		result["comment"] = convert_to_string8(tagging_systems[no].comment());
		return result;
	};

	json	jdoctors;
	for(size_t i = 0; i < tagging_systems.size(); ++i) jdoctors.push_back(doctors_data(i));
	return move(jdoctors);
}

json	structured_study_to_json(const TomogramTaggingReport &study_original)
{
	TomogramTaggingReport study(study_original);
	AdjustDoctorIDs(study);

	json jclusters;
	
	json jstudy;
	jstudy["ids"] = move(study_ids_to_json(study.ids));
	jstudy["doctors"] = doctors_to_json(study.tagging_systems);
	
	for(auto &cluster: study)
	{
		json jcluster;
		for(auto &nodule: cluster)
		{
			jcluster.push_back(nodule_to_json(nodule, study.tagging_systems));
		}
		jclusters.push_back(jcluster);
	}

	jstudy["nodules"] = move(jclusters);

	return jstudy;
}

wstring		DumpStudy_json(const StudyTaggingReport &generic_study, const wstring &json_path)
{
 	const TomogramTaggingReport	&study = dynamic_cast<const TomogramTaggingReport&>(generic_study);
	json	j = structured_study_to_json(study);

	wstring	fn = json_path + L"/" + study.ids.accession_number() + L"_" + study.ids.study_id() + L".json";
	
	try
	{
		if(!DirectoryExists(json_path)) CreatePath(json_path);
		utf8_ofstream	file(fn);
		if(!file.is_open()) return L"";//почему-то файл не открывается
		file << j;
		return fn;
	}
	catch(...)
	{
		return L"";
	}
}

void	DumpReport_json(const vector<StudyTaggingReport_ptr> &report, const wstring &report_path, const wstring &title)
{
	wstring	json_path = report_path + L"/" + title + L" [JSON]";

	GUIProgressBar	progress;
	progress.start(L"Saving JSON (" + title + L")", report.size());
	for(auto &study: report)
	{
		DumpStudy_json(*study, json_path);
		++progress;
	}

}


XRAD_END

