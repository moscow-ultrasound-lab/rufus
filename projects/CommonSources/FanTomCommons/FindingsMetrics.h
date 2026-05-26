#ifndef FindingsMetrics_h__
#define FindingsMetrics_h__

/*!
	\file
	\date 2019/05/17 13:02
	\author kulberg

	\brief  
*/

#include "TomogramFinding.h"
#include "NodulesClusters.h"

XRAD_BEGIN

enum nodules_joinability
{
	e_different,
	e_clusterable,
	e_joinable
};



bool	same_nodule(const TomogramFinding &f1, const TomogramFinding &f2);
bool	same_cluster(const TomogramFinding &f1, const TomogramFinding &f2);


bool	clusters_z_compare(const nodules_cluster &cluster_1, const nodules_cluster &cluster_2);//->bool;
double proximity_metrics(const TomogramFinding& f1, const TomogramFinding& f2);

nodules_joinability	CheckNodulesJoinability(const nodule_t &nodule1, const nodule_t &nodule2);


double	greatest_finding_radius(const nodule_t &n);
bool	compare_greatest_finding(const nodule_t &n1, const nodule_t &n2);;


XRAD_END

#endif // FindingsMetrics_h__
