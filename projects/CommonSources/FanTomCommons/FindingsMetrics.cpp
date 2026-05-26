#include "pre.h"
#include "FindingsMetrics.h"
/*!
	\file
	\date 2019/05/17 13:02
	\author kulberg

	\brief 
*/

XRAD_BEGIN


bool same_nodule(const TomogramFinding &f1, const TomogramFinding &f2)
{
	if(quite_different(f1, f2)) return false;
	if(f1.doctor_id() == f2.doctor_id()) return false;

	physical_length r = max(TomogramFinding::min_diameter(), min(f1.diameter(), f2.diameter()));
	// если два очага пересекаются так, что центр одного принадлежит другому, считаем их одним.
	if(norma(f1.center-f2.center) < r.mm()) return true;
	return false;
}


bool same_cluster(const TomogramFinding &f1, const TomogramFinding &f2)
{
	if(quite_different(f1, f2)) 
		return false;

//	physical_length d = (f1.diameter() + f2.diameter()) / 2;
	physical_length d = (f1.diameter() + f2.diameter())/1.4;

	// если два очага хоть немного соприкасаются, считаем их принадлежащими к одному кластеру.
	if(norma(f1.center-f2.center) < d.mm()) return true;
	else return false;
}


nodules_joinability CheckNodulesJoinability(const nodule_t &nodule1, const nodule_t &nodule2)
{
	bool	clusterable = false;	// есть пара близкорасположенных очагов
	bool	join_possible  = false;	// есть пара сильно пересекающихся очагов
	bool	prohibit_join = false;	// запрет объединения, самое сильное условие

	for(auto &doc1: nodule1)
	{
		const wstring &key1 = doc1.first;
		const TomogramFinding &finding1 = doc1.second;
		for(auto &doc2: nodule2)
		{
			const wstring &key2 = doc2.first;
			const TomogramFinding &finding2 = doc2.second;

			if(key1==key2)
			{
				//хотя бы один доктор указал на две пересекающиеся находки. их нельзя сливать в одну, должен быть сложный кластер
				if(!finding1.empty() && !finding2.empty()) 
					prohibit_join = true;
			}

			if(same_cluster(finding1, finding2))
			{
				clusterable = true;	//есть хотя одна близкорасположенная пара, находки должны принадлежать к одному кластеру
				if(same_nodule(finding1, finding2))
				{
					join_possible = true;//есть хотя бы одно пересечение находок, находки можно объединить в один очаг
				}
			}
		}
	}

	if(join_possible && !prohibit_join) return e_joinable;
	if(clusterable) return e_clusterable;

	return e_different;
}

double greatest_finding_radius(const nodule_t &n)
{
	double	r = 0;
	for(auto &f: n)
	{
		if(!f.second.empty())
		{
			if(f.second.diameter().mm() > r)
			{
				r = f.second.diameter().mm();
			}
		}
	}

	if(r==0) r = max_double();
	return r;
}

bool compare_greatest_finding(const nodule_t &n1, const nodule_t &n2)
{
	return greatest_finding_radius(n1) < greatest_finding_radius(n2);
}

double proximity_metrics(const TomogramFinding& f1, const TomogramFinding& f2)
{
	return distance(f1, f2) + max(f1.diameter().mm(), f2.diameter().mm());
}

bool clusters_z_compare(const nodules_cluster &cluster_1, const nodules_cluster &cluster_2) //->bool
{
	double	z1(0), z2(0);
	size_t	n1(0), n2(0);
	for(auto nodule: cluster_1)
	{
		for(auto &finding: nodule)
		{
			if(!finding.second.empty()) ++n1, z1+= finding.second.center.z();
		}
	}
	for(auto nodule: cluster_2)
	{
		for(auto &finding: nodule)
		{
			if(!finding.second.empty()) ++n2, z2+= finding.second.center.z();
		}
	}
	if(n1&&n2) return (z1/n1) > (z2/n2);
	else return false;
}


XRAD_END