#include "pre.h"
#include "NodulePredicate.h"
#include <FanTomCommons/TaggingReport/TomogramTaggingReport.h>

/*!
	\file
	\date 2018/01/19 15:38
	\author kulberg

	\brief 
*/

XRAD_BEGIN



void	FilterProtocolColumns(TomogramTaggingReport &protocol, const nodule_predicate &pred)
{
	// вычищаем колонки, соответствующие заданному предикату
	// контейнеры, содержащие строки таблицы, могут быть в общем случае разной длины

	for(auto cluster_it = protocol.begin(); cluster_it < protocol.end(); )
	{
		for(auto joint_nodule_it = cluster_it->begin(); joint_nodule_it < cluster_it->end(); )
		{
			if(pred.delete_condition(*joint_nodule_it))
			{
				joint_nodule_it = cluster_it->erase(joint_nodule_it);
			}
			else
			{
				++joint_nodule_it;
			}
		}
		if(cluster_it->empty())
		{
			cluster_it = protocol.erase(cluster_it);
		}
		else
		{
			++cluster_it;
		}
	}

}


shared_ptr<nodule_predicate> GetNodulePredicate()
{
	nodule_predicate *result;
	size_t	answer = GetButtonDecision(L"Выберите фильтр",
	{
		L"Все исследования",
		L"Все исследования с очагами",
		L"Только очаги, подтвержденные 1 раз",
		L"Только очаги, подтвержденные 2 раза",
		L"Только очаги, подтвержденные 3 раза",
		L"Только очаги, подтвержденные N раз и больше",
		L"Набор"
	});

	switch(answer)
	{

		case 0:
			result = new keep_all_predicate;
			break;

		case 1:
			result = new delete_dummy_predicate;
			break;

		case 2:
		case 3:
		case 4:
			result = new save_count_predicate(answer-1);
			break;

		case 5:
			result = new save_more_predicate(GetUnsigned("Введите нижнюю границу", 4, 0, max_int()));
			break;

		case 6:
		{
			std::set<size_t>	nns;
			size_t	n0 = 3;
			do
			{
				nns.insert(GetUnsigned("Добавьте число", n0++, 0, max_int()));
			} while(YesOrNo("Добавить еще значение?", true));
			result = new save_count_set_predicate(vector<size_t>(nns.begin(), nns.end()));
		}
		break;
		default:
			throw invalid_argument("GetColumnPredicate");

	}
	return shared_ptr<nodule_predicate>(result);
}

XRAD_END

