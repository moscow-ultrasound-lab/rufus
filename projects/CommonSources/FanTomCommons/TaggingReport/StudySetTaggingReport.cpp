#include "pre.h"
#include <set>
#include <XRADSystem/Sources/TextFile/text_file.h>

#include "StudySetTaggingReport.h"
//#include "IDCorrespondence.h"
#include <FanTomCommons/NodulePredicate.h>
#include <FanTomCommons/TaggingReport/TomogramTaggingReport.h>
//#include "DIT_ROC.h"
#include <FanTomCommons/DumpReport_csv.h>
#include <FanTomCommons/DumpReport_json.h>
#include <iostream>

/*!
	\file
	\date 2018/01/16 10:39
	\author kulberg

	\brief 
*/

XRAD_BEGIN

void	DumpStructuredReport(const StudySetTaggingReport &report, const wstring &report_path, wstring title)
{
	DumpReport_csv(report, report_path, title);
	DumpReport_json(report, report_path, title);

	text_file_writer	filelist(title + L"_filelist.txt", text_encoding::utf16_le);
	for(auto &study_ptr: report)
	{
		auto &study = dynamic_cast<TomogramTaggingReport&>(*study_ptr);
		filelist.printf_(L"%s\t%s", study.ids.accession_number().c_str(), study.ids.study_id().c_str());
		
		for(auto &doctor: study.tagging_systems)
		{
			filelist.printf_(L"\t%s", doctor.comment().c_str());
		}
		filelist.printf_(L"\n");

	}
}


StudySetTaggingReport	SelectTopViews(const StudySetTaggingReport &report, const wstring &target_path, size_t	N)
{
	text_file_writer	report_file(target_path + L"/top_500_views_list.txt", text_encoding::utf16_le);

	// выбираются N>500 исследований, просмотренных максимальное количество раз
	StudySetTaggingReport	sorted_report(report.begin(), report.end());

//	auto	pred = [](TomogramTaggingReport& r1, TomogramTaggingReport &r2) { return r1.tagging_systems.size() > r2.tagging_systems.size(); };
	
	auto	pred = [](StudyTaggingReport_ptr &r1, StudyTaggingReport_ptr &r2)
	{ 
		return r1->tagging_systems.size() > r2->tagging_systems.size(); 
	};

	std::sort(sorted_report.begin(), sorted_report.end(), pred);

	size_t	report_size = min(N, report.size());

	for(size_t i = 0; i < report_size; ++i)
	{
		auto &study_report = dynamic_cast<TomogramTaggingReport &>(*sorted_report[i]);
		report_file.printf_(L"%d\t%s\t%s\t%d\n", int(i), study_report.ids.accession_number().c_str(), study_report.ids.study_id().c_str(), study_report.size());
	}
	
	if(report.size() <= N) return report;
	return StudySetTaggingReport(sorted_report.begin(), sorted_report.begin() + N);
}





StudySetTaggingReport	GenerateStructuredProtocol(const reports_heap_t &raw_protocol)
{
	std::set<Dicom::complete_study_id_t> ids;
	for(auto &p: raw_protocol)
	{
		ids.insert(p.ids);
	}

	StudySetTaggingReport findings;
	GUIProgressBar	progress;

	progress.start("Generating reports", ids.size());
	for(auto &id: ids)
	{
		findings.push_back(GenerateOneStudyReport(raw_protocol, id));
		++progress;
	}
	progress.end();
	return findings;
}

StudySetTaggingReport GetClearStudies(const StudySetTaggingReport &report)
{
	StudySetTaggingReport findings(report);
	for(auto it = findings.begin(); it != findings.end();)
	{
		if(!(**it).empty())
		{
			it = findings.erase(it);
		}
		else
		{
			++it;
		}
	}
	return findings;
}


// из полного отчета выделяются только находки, подтвержденные заданное количество раз
StudySetTaggingReport FilterStructuredReport(const StudySetTaggingReport &report, const nodule_predicate &pred)
{
	StudySetTaggingReport findings(report);
	for(auto it = findings.begin(); it != findings.end();)
	{
		FilterProtocolColumns(dynamic_cast<TomogramTaggingReport &>(**it), pred);
		if(pred.erase_empty_study() && (**it).empty())
		{
			it = findings.erase(it);
		}
		else
		{
			++it;
		}
	}
	return findings;
}

void	EraseExcludeList(StudySetTaggingReport &report, const vector<Dicom::complete_study_id_t> &exclude_ids)
{
	auto	it = report.begin();
	for(;it != report.end();)
	{
		bool	exclude = false;
		for(auto &id: exclude_ids)
		{
			if((**it).ids==id) exclude = true;
		}
		if(exclude) it = report.erase(it);
		else ++it;
	}
}



void	GenerateStatistics(const StudySetTaggingReport &findings, RealFunctionUI32	&statistic_counts)
{
	for(auto &one_study_protocol_ptr: findings)
	{
		auto &one_study_protocol = dynamic_cast<TomogramTaggingReport &>(*one_study_protocol_ptr);
		if(one_study_protocol.size() < statistic_counts.size()) ++statistic_counts[one_study_protocol.size()];
	}
}

template<class ARR>
void	histogram_expand_and_inc(ARR &arr, size_t position)
{
	if(position + 1 > arr.size())
	{
		size_t	old_size = arr.size();
		arr.resize(position+1);
		std::fill(arr.begin() + old_size, arr.end(), 0);
	}
	++arr[position];
}

void	DumpSummary(const StudySetTaggingReport &report, const wstring &target_path, const wstring &title)
{
	wstring	summary_name = target_path + L"/" + title + L".summary.txt";
	text_file_writer	report_file(summary_name, text_encoding::utf16_le);
	report_file.printf_(L"Общее количество исследований, удовлетворяющих условию (%s): %d\n", title.c_str(), int(report.size()));

	RealFunctionF32	nodules_counter;
	physical_length	dr = mm(1);//mm


	std::map<wstring, size_t>	types_counter;
	RealFunctionF32	radius_counter;



	for(auto &one_study_protocol_ptr : report)
	{
//		size_t	n_doctors = one_study_findings.doctor_ids.size();

		auto &one_study_findings = dynamic_cast<TomogramTaggingReport &>(*one_study_protocol_ptr);

		histogram_expand_and_inc(nodules_counter, one_study_findings.size());

		for(size_t cluster_no = 0; cluster_no < one_study_findings.size(); ++cluster_no)
		{
			size_t	cluster_size = one_study_findings[cluster_no].size();
			for(size_t nodule_no = 0; nodule_no < cluster_size; ++nodule_no)
			{
				size_t	r_index = size_t(one_study_findings[cluster_no][nodule_no].diameter() / dr);
				histogram_expand_and_inc(radius_counter, r_index);

// 				for(size_t doctor_no = 0; doctor_no < n_doctors; ++doctor_no)
				for(auto &finding: one_study_findings[cluster_no][nodule_no])
				{
//					finding_t
//					auto &nodule(one_study_findings[cluster_no][nodule_no][doctor_no]);
					if(!finding.second.empty() && finding.first != L"DIT")
					{
						++types_counter[finding.second.type()];
					}
				}
			}
		}

	}

	report_file.printf_("types found:\t");
	for(auto &t: types_counter) report_file.printf_(L"%s (%d times), ", t.first.c_str(), int(t.second));

	report_file.printf_("\nСтатистика по количеству находок в одном исследовании:\n");
	report_file.printf_("Число находок\tКоличество исследований, содержащих указанное число находок\n");
	for(size_t i = 0; i < nodules_counter.size(); ++i)
	{
		report_file.printf_("%d\t%d\n", int(i), int(nodules_counter[i]));
	}

	report_file.printf_("\nСтатистика по количеству размерам очагов:\n");
	report_file.printf_("Радиус, мм\tКоличество находок с таким радиусом\n");
	for(size_t i = 0; i < radius_counter.size(); ++i)
	{
		report_file.printf_("%.2f\t%g\n", double(i)*dr.mm(), radius_counter[i]);
	}
}

StudySetTaggingReport &StudySetTaggingReport::operator += (const StudySetTaggingReport &r2)
{
	if(!r2.size()) return *this;


	for(auto &sr1: *this)
	{
		auto	addendum_it = find_if(r2.begin(), r2.end(), [&sr1](auto const &it){return sr1->ids==it->ids;});
		if(addendum_it != r2.end())
		{
			// Если соответствие нашлось, добавляем его в первый список 
			dynamic_cast<TomogramTaggingReport &>(*sr1) += dynamic_cast<TomogramTaggingReport &>(**addendum_it);
		}
		else
		{
			// Если нет, делаем пометку в первом списке, что доктор, составивший второй список, ничего не нашел в этом исследовании
			// Для этого добавляем пустой протокол, помеченный именем доктора из второго списка
			vector<tagging_system_identity> addendum_doctors;
			for(auto &doctor: r2[0]->tagging_systems)
			{
				addendum_doctors.push_back(tagging_system_identity(doctor.id(), L"Nothing found"));
			}
			
			dynamic_cast<TomogramTaggingReport &>(*sr1) += TomogramTaggingReport(sr1->ids, addendum_doctors);
		}
	}

if(0)	for(auto &sr2: r2)
	{
		// Теперь смотрим второй список, нет ли в нем исследований, которые доктора первого списка упустили.
		auto	addendum_it = find_if(begin(), end(), [&sr2](auto const &it){return sr2->ids==it->ids;});
		if(addendum_it != end())
		{
//			cout << "\nfound=" << convert_to_string(sr2.ids.accession_number());
		}
		else
		{
//			cout << "\nnot found=" << convert_to_string(sr2.ids.accession_number());
			// Если есть, создаем запись, в которой отражена уникальная находка и пустые записи остальных докторов
			push_back(sr2);
			auto	&r1_doctors = front()->tagging_systems;
			for(int i = 0; i < r1_doctors.size(); ++i)
			{
				if(r1_doctors[i].id() != sr2->tagging_systems[0].id())
				{
					back()->tagging_systems.push_back(tagging_system_identity(r1_doctors[i].id(),L"Nothing found"));
					//back().doctor_comments.push_back();
				}
			}
		}
	}

	return *this;
}



XRAD_END

