#include "pre.h"
#include "JoinFindings.h"

/*!
	\file
	\date 2019/05/17 12:48
	\author kulberg

	\brief
*/

XRAD_BEGIN




// bool move_finding_it(TomogramFinding &destination, vector<TomogramFinding> &source_vector, vector<TomogramFinding>::iterator &it)
// {
// 	// если целевой объект уже заполнен, возвращаем false
// 	if(destination.empty())
// 	{
// 		destination = std::move(*it);//std::move
// 		it = source_vector.erase(it);
// 		return true;
// 	}
// 	return false;
// };

bool move_finding(TomogramFinding &destination, vector<TomogramFinding> &source_vector, const TomogramFinding &object)
{
	// ищет элемент, идентичный object в source_vector, и перемещает его в destination. Неуклюже написано, однако
	auto it = std::find(source_vector.begin(), source_vector.end(), object);
	if(it==source_vector.end()) return false;
//	return move_finding_it(destination, source_vector, it);
	// если целевой объект уже заполнен, возвращаем false
	if(destination.empty())
	{
		destination = std::move(*it);//std::move
		it = source_vector.erase(it);
		return true;
	}
	return false;

};



void	JoinNodules(nodule_t &nodule1, nodule_t &nodule2)
{
	for(auto doc2 = nodule2.begin(); doc2 != nodule2.end(); )
	{
		// обходим второй список и все его непустое содержимое переносим в в первый
		const wstring &doctor_id = doc2->first;
		TomogramFinding &finding2 = doc2->second;

		auto finding1 = nodule1.find(doctor_id);
		if(finding1 == nodule1.end() && !finding2.empty())
		{
			nodule1.insert(*doc2);
			nodule2.erase(doctor_id);
			doc2 = nodule2.begin();
//			std::swap(finding1, finding2);// обходим коварный qm_container_swap!!!!!
		}
		else ForceDebugBreak();//Такая же ошибка, как и ниже. Сюда были поданы non-jounable очаги, вызывающая процедура не подготовила их должным образом
	}
	if(!nodule2.empty())
	{
		// 		ForceDebugBreak();
		// важная ошибка, должна сохраняться.
		throw invalid_argument("JoinNodules, non-joinable argument");
	}
}


void	ShowCluster(const wstring &title, const nodules_cluster &cluster, bool stopped)
{
//	return;

	wstring	result;
	size_t	s2(0);
	for(auto &row: cluster) s2 = max(s2, row.size());
	DataArray2D<DataArray<wstring>> cells(cluster.size(), s2, L"");

	static	TextDisplayer	td(L"Clusters", true, true);

	int	i(0);
	for(auto & nodule : cluster)
	{
		int j(0);
		for(auto &finding: nodule)
		{
			cells.at(i, j) += ssprintf(L"'%s':", finding.first.c_str());//?
			if(finding.second.empty())
			{
				cells.at(i, j) += L"-------------------------------\t";
			}
			else
			{
				cells.at(i, j) += ssprintf(L"(%g,%g,%g,%g : %d,%d,%d)\t", finding.second.center.z(), finding.second.center.y(), finding.second.center.x(), finding.second.diameter().mm(),
										   int(!finding.second.rejected()),
										   int(!finding.second.wrong_size()),
										   int(finding.second.malignant()));
			}
			++j;
		}
// 		result += L"\n";
		++i;
	}

	for(size_t ii = 0; ii < cells.hsize(); ++ii)
	{
		for(size_t jj = 0; jj < cells.vsize(); ++jj)
		{
			result += cells.at(jj, ii);
		}
		result += L"\n";
	}

	static	wstring	text;

	text += title + L"\n" + result + L"\n";
	td.SetText(text);
	td.Display(stopped);
//	ShowText(title, result, stopped);
}



void	GatherNodule(nodule_t &result_nodule, vector<TomogramFinding> &raw_findings)
{
	if(result_nodule.empty() && !raw_findings.empty())
	{
		TomogramFinding	moved(raw_findings[0].doctor_id());
		move_finding(moved, raw_findings, raw_findings[0]);
		result_nodule.add_finding(moved);
	}
	else for(auto search_finding = result_nodule.begin(); search_finding != result_nodule.end(); ++search_finding)
	{
		//	В векторе результата уже есть непустой очаг, найденный одним доктором.
		//	Смотрим, нет ли других похожих на него у других докторов

			//	Делается последовательное сравнение уже найденных очагов с оставшимися очагами, собранными в raw_findings.
			//	Ближайший очаг из raw_findings становится кандидатом на добавление в result
		double	closest_object_distance = max_double();
		TomogramFinding	closest_analog(L"");

		for(auto &new_finding : raw_findings)
		{
			if(same_nodule(search_finding->second, new_finding)) // Очередной объект лежит рядом с одним из уже принятых, при этом идентификаторы врачей не совпадают.
			{
				double	current_distance = proximity_metrics(search_finding->second, new_finding);
				if(current_distance < closest_object_distance)
				{
					if(result_nodule.finding_empty(new_finding.doctor_id())) // Место для находки этого доктора в result_nodule еще не занято!
					{
						closest_object_distance=current_distance;
						closest_analog = new_finding;
					}
				}
			}
		}

		if(!closest_analog.empty())// если ближайший подходящий очаг найден...
		{
			// Объект closest_analog удаляется из списка raw_findings и переносится в список result_nodule.
			// Но это происходит только в том случае, если соответствующее место в result_nodule еще не занято.
			// А занять его могла другая близкорасположенная находка того же доктора
			if(result_nodule.finding_empty(closest_analog.doctor_id()))
			{
				TomogramFinding	moved(closest_analog.doctor_id());
				move_finding(moved, raw_findings, closest_analog);
				result_nodule.add_finding(moved);
			}
		}
	}
}


void	GatherNodule(nodule_t	&result_nodule, one_study_findings_t &one_study_findings)
{
	for(auto &current_doctor_report : one_study_findings)
	{
		GatherNodule(result_nodule, current_doctor_report);
	}
}




vector<nodule_t> DisassembleClusters(TomogramTaggingReport &study)
{
	vector<nodule_t> nodules;

	while(!study.empty())
	{
		auto	&cluster = study.back();
		while(!cluster.empty())
		{
			nodules.push_back(move(cluster.back()));
			cluster.pop_back();
		}
		study.pop_back();
	}


	return nodules;
}



void	JoinClusters(TomogramTaggingReport &study, vector<nodule_t> &nodules)
{
	while(!nodules.empty())
	{
		nodules_cluster	cluster;

		cluster.push_back(move(nodules.back()));
		nodules.pop_back();

		for(auto nodule = nodules.begin(); nodule < nodules.end();)
		{
			bool	found(false);
			for(size_t i = 0; i < cluster.size() && nodule < nodules.end(); ++i)
			{
				auto &nodule_in_cluster = cluster[i];
				auto	joinability = CheckNodulesJoinability(nodule_in_cluster, *nodule);
				switch(joinability)
				{
					case e_joinable:
						JoinNodules(nodule_in_cluster, *nodule);
						nodule = nodules.erase(nodule);
						found = true;
						break;

					case e_clusterable:
						cluster.push_back(move(*nodule));
						nodule = nodules.erase(nodule);
						found = true;
						break;
				}
			}
			if(!found) ++nodule;
		}

		std::sort(cluster.begin(), cluster.end(), compare_greatest_finding);

		if(cluster.size() > 2)
		{
//			ShowCluster(study.ids.accession_number() + L"_" + study.ids.study_id(), cluster, false);
		}

		study.push_back(move(cluster));
	}

	for(auto cluster = study.begin(); cluster < study.end();)
	{
//		if(cluster->size() > 2) ShowCluster(study.ids.accession_number() + L"_" + study.ids.accession_number(), *cluster);
		if(cluster->empty()) cluster = study.erase(cluster);
		else ++cluster;
	}
	study.sort_clusters_z();
}

//#error через эту функцию и сделать объединение с новыми ДИТовскими протоколами
void ReassembleClusters(TomogramTaggingReport &study)
{
	auto	nodules = DisassembleClusters(study);
	JoinClusters(study, nodules);
}

TomogramTaggingReport JoinSimilarFindings(one_study_findings_t &one_study_findings)
{
	// совмещаем одинаковые находки у разных докторов, отсутствующие помечаем флагом empty

	vector<tagging_system_identity> doctors;

	for(auto &doctor: one_study_findings)
	{
		doctors.push_back(tagging_system_identity(doctor.doctor_id, doctor.comment));
	}

	TomogramTaggingReport result(one_study_findings.ids, doctors);

	vector<nodule_t> nodules;

	for(auto &doctor_report : one_study_findings)
	{
		while(doctor_report.size())
		{
			nodule_t	nodule;
			for(auto &current_doctor_report : one_study_findings)
			{
				GatherNodule(nodule, current_doctor_report);
			}
			nodules.push_back(nodule);
		}
	}

	// анализ возможного сходства соседних нодул, объединение их в кластеры
	JoinClusters(result, nodules);

	return result;
}




XRAD_END

