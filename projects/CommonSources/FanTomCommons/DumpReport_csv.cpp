#include "pre.h"
#include "DumpReport_csv.h"
#include <XRADSystem/Sources/TextFile/text_file.h>

/*!
	\file
	\date 2018/07/12 13:13
	\author kulberg

	\brief 
*/

XRAD_BEGIN


void	DumpFindingsHeader_csv(text_file_writer &file)
{
	file.printf_(L"%s\t", L"Accession number");
	file.printf_(L"%s\t", L"Study ID");
	file.printf_(L"%s\t", L"ID доктора");
	file.printf_(L"%s\t", L"Комментарий");
	file.printf_(L"%s\t", L"Количество находок");

	for(int i = 1; i <= 20; ++i)
	{
		file.printf_(L"%d. x\t", i);
		file.printf_(L"%d. y\t", i);
		file.printf_(L"%d. z\t", i);
		file.printf_(L"%d. Размер (мм)\t", i);
		file.printf_(L"%d. Тип\t", i);
	}
	file.printf_(L"\r\n\r\n");

}

void	DumpNodule_csv(const TomogramFinding &nodule, text_file_writer &file)
{
	if(nodule.empty())
	{
		file.printf_(L"-\t", nodule.center.x());// второй параметр фиктивный, указан для пояснения
		file.printf_(L"-\t", nodule.center.y());
		file.printf_(L"-\t", nodule.center.z());
		file.printf_(L"-\t", nodule.diameter().mm());
		file.printf_(L"-\t", nodule.type().c_str());
	}
	else
	{
		file.printf_(L"%g\t", nodule.center.x());
		file.printf_(L"%g\t", nodule.center.y());
		file.printf_(L"%g\t", nodule.center.z());
		file.printf_(L"%g\t", nodule.diameter().mm());
		file.printf_(L"%s\t", nodule.type().c_str());
	}

}

void	DumpStudy_csv(const TomogramTaggingReport &protocol, text_file_writer &file)
{
	// Подсчет непустых находок у одного врача. В итоговом списке учитываются все находки по исследованию, в том числе сделанные одним доктором и пропущенные другими
	auto CountNonEmptyFindings = [](const TomogramTaggingReport &sf, const wstring &doctor_id)
	{
		size_t result(0);
		for(auto &cluster: sf)
		{
			for(auto &nodule: cluster)
			{
				auto it = nodule.find(doctor_id);
				if(it != nodule.end())
				{
					if(!it->second.empty()) ++result;
				}
			}
		} 
		return result;
	};

	for(size_t doctor_no = 0; doctor_no < protocol.tagging_systems.size(); ++doctor_no)
	{
		file.printf_(L"%s\t", protocol.ids.accession_number().c_str());
		file.printf_(L"%s\t", protocol.ids.study_id().c_str());
		file.printf_(L"'%s'\t", protocol.tagging_systems[doctor_no].id().c_str());
		file.printf_(L"%s\t", protocol.tagging_systems[doctor_no].comment().c_str());
		file.printf_(L"%d (%d)\t", CountNonEmptyFindings(protocol, protocol.tagging_systems[doctor_no].id()), protocol.size());
		for(size_t cluster_no = 0; cluster_no < protocol.size(); ++cluster_no)
		{
			for(size_t nodule_no = 0; nodule_no < protocol[cluster_no].size(); ++nodule_no)
			{
				auto it = protocol[cluster_no][nodule_no].find(protocol.tagging_systems[doctor_no].id());
				if(it!= protocol[cluster_no][nodule_no].end())
				{
					//if(it->second.empty());
					DumpNodule_csv(it->second, file);
				}
				else
				{
//					ForceDebugBreak();
					TomogramFinding	empty_finding(protocol.tagging_systems[doctor_no].id());
					DumpNodule_csv(empty_finding, file);
				}
//				auto &nodule(protocol[cluster_no][nodule_no][doctor_no]);
// 				DumpNodule_csv(nodule, file);
			}
		}
		file.printf_(L"\r\n");
	}
	file.printf_(L"Оценки эксперта\r\n\r\n");
}

void	DumpReport_csv(const vector<StudyTaggingReport_ptr> &report, const wstring &report_path, wstring title)
{
	wstring	report_filename = report_path + L"/" + title + L".txt";
	try
	{
		text_file_writer	report_file(report_filename, text_encoding::utf16_le);

		DumpFindingsHeader_csv(report_file);
		GUIProgressBar	progress;
		progress.start("Writing findings", report.size());
		for (auto &one_study_protocol_ptr : report)
		{
			auto &one_study_protocol = dynamic_cast<TomogramTaggingReport&>(*one_study_protocol_ptr);
			DumpStudy_csv(one_study_protocol, report_file);
			++progress;
		}
		progress.end();
	}
	catch (...)
	{
		Error(L"Отчет не был сохранен, файл " + report_filename + L" не может быть создан (возможно, он уже существует и открыт в другом приложении)");
	}

}




XRAD_END

