#ifndef NodulesClusters_h__
#define NodulesClusters_h__

/*!
	\file
	\date 2018/10/10 12:14
	\author kulberg

	\brief  
*/

//#include "StructuredStudyProtocol.h"
#include "Nodule.h"

XRAD_BEGIN



struct nodules_cluster : public vector<nodule_t>
{
	PARENT(vector<nodule_t>);
	bool empty() const
	{
		if(parent::empty()) return true;
		bool	result(true);
		for(auto &nodule: *this) if(!nodule.empty()) result = false;
		return result;
	}

	void	rename_doctor(const wstring &new_doctor_id, const wstring &old_doctor_id)
	{
		for(auto &nodule: *this) nodule.rename_doctor(new_doctor_id, old_doctor_id);
	}

	set<wstring> doctor_ids() const
	{
		set<wstring>	result;
		for(auto &nodule: *this)
		{
			set<wstring>	result1 = nodule.doctor_ids();
			result.insert(result1.begin(), result1.end());
		}
		return result;
	}
//	cluster.add_finding
};






XRAD_END

#endif // NodulesClusters_h__
