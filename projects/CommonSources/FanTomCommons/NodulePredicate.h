#ifndef ColumnPredicate_h__
#define ColumnPredicate_h__

/*!
	\file
	\date 2018/01/19 15:34
	\author kulberg

	\brief  
*/

#include <FanTomCommons/TaggingReport/TomogramTaggingReport.h>
//#include "Nodule.h"

XRAD_BEGIN


struct nodule_predicate
{
	size_t	n_rows_total;//число строк, в которых есть хоть какие-то данные
	size_t	n_rows_dummy; //число строк, для которых установлен флаг empty
	void	reset(){ n_rows_total = 0; n_rows_dummy = 0; }

	nodule_predicate(){ reset(); }

	size_t	non_empty_nodules_count(const nodule_t &sf) const
	{
		size_t result = 0;
		for(auto &nodule: sf)
		{
			if(!nodule.second.empty()) ++result;
		}
		return result; 
	}

	virtual bool	delete_condition(const nodule_t &sf) const = 0;
	virtual	wstring comment() = 0;
	virtual bool	erase_empty_study() const = 0;
};

struct delete_dummy_predicate : public nodule_predicate
{
	//удаление пустых колонок
	virtual bool	delete_condition(const nodule_t &sf) const override
	{
		size_t	count = non_empty_nodules_count(sf);
		return count==0;
	}
	virtual	wstring comment() override { return L"findings_confirmed_01_and_more"; }
	virtual bool	erase_empty_study() const { return true; }
};

struct keep_all_predicate : public nodule_predicate
{
	//без фильтрации, оставить все
	virtual bool	delete_condition(const nodule_t &) const override { return false; }
	virtual	wstring comment() override { return L"keep_all_results"; }
	virtual bool	erase_empty_study() const { return false; }
};

struct save_count_predicate : public nodule_predicate
{
	//остаются колонки, содержащие только n значений
	const size_t	n;
	save_count_predicate(size_t in_n) : n(in_n){}
	bool	delete_condition(const nodule_t &sf) const override { return (non_empty_nodules_count(sf)!=n); }

	virtual	wstring comment() override { return ssprintf(L"findings_confirmed_%02d_times", int(n)); }
	virtual bool	erase_empty_study() const { return true; }
};

struct save_more_predicate : public nodule_predicate
{
	//остаются колонки, содержащие только n значений
	const size_t	n;
	save_more_predicate(size_t in_n) : n(in_n){}
	bool	delete_condition(const nodule_t &sf) const override { return non_empty_nodules_count(sf)<n; }
	virtual	wstring comment() override { return ssprintf(L"findings_confirmed_%02d+_times", int(n)); }
	virtual bool	erase_empty_study() const { return true; }
};

struct save_count_set_predicate : public nodule_predicate
{
	//остаются колонки, содержащие только число значений из заданного множества
	const vector<size_t>	allowed_count_set;
	save_count_set_predicate(vector<size_t> in_n) : allowed_count_set(in_n){}
	bool	delete_condition(const nodule_t &sf) const override 
	{
 		for(auto n: allowed_count_set)
		{
			if(n==non_empty_nodules_count(sf)) return false;
		}
		return true;
	}
	virtual	wstring comment() override
	{
		wstring	result = L"findings_confirmed_";
		for(auto n: allowed_count_set) result += ssprintf(L"%02d_", int(n));
		result += L"times";
		return result;
	}
	virtual bool	erase_empty_study() const { return true; }
};


void	FilterProtocolColumns(TomogramTaggingReport &protocol, const nodule_predicate &pred);

shared_ptr<nodule_predicate> GetNodulePredicate();


XRAD_END

#endif // ColumnPredicate_h__
