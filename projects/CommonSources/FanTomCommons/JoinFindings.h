#ifndef JoinFindings_h__
#define JoinFindings_h__

/*!
	\file
	\date 2019/05/17 12:48
	\author kulberg

	\brief  
*/

#include <FanTomCommons/TaggingReport/TomogramTaggingReport.h>
#include "FindingsMetrics.h"

XRAD_BEGIN



void	JoinClusters(TomogramTaggingReport &study, vector<nodule_t> &nodules);

vector<nodule_t> DisassembleClusters(TomogramTaggingReport &study);
void ReassembleClusters(TomogramTaggingReport &study);

TomogramTaggingReport JoinSimilarFindings(one_study_findings_t &one_study_findings);


XRAD_END

#endif // JoinFindings_h__
