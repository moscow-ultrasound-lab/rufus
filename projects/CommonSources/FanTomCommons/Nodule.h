#ifndef StudyFindings_h__
#define StudyFindings_h__

/*!
	\file
	\date 2018/01/16 10:42
	\author kulberg

	\brief  
*/

#include <vector>
#include <list>
#include <map>
#include <XRADDicom/XRADDicom.h>
#include "TomogramFinding.h"


XRAD_BEGIN

//! Тип "столбец находок", когда разные доктора указали на один и тот же объект
//!	Первый параметр шаблона wstring это идентификатор врача
struct nodule_t : public map<wstring, TomogramFinding>
{
	PARENT(map<wstring, TomogramFinding>);

	physical_length	diameter() const
	{
		physical_length	d(mm(0));
		size_t n(0);
		for(auto &finding : *this) if(!finding.second.empty()) d+=finding.second.diameter(), ++n;
		if(n) return d/n;
		else return mm(0);
	}

	bool finding_exists(const wstring &doctor_id) const {return find(doctor_id) != end();}

	bool finding_empty(const wstring &doctor_id) const
	{
		if(!finding_exists(doctor_id)) return true;
		return at(doctor_id).empty();
	}

	bool empty() const
	{
		if(parent::empty()) return true;
		bool	result(true);
		for(auto &nodule: *this) if(!nodule.second.empty()) result = false;
		return result;
	}

	nodule_t() = default;


	// Костыль очередной
	TomogramFinding at(const wstring &doctor_id) const
	{
		auto	it = find(doctor_id);
		if(it==end()) return TomogramFinding(doctor_id);// пустая находка равносильна сообщению, что этот доктор ничего в этом месте не обнаружил
		else return it->second;
	}

	TomogramFinding &at(const wstring &doctor_id)
	{
		auto	it = find(doctor_id);
		if(it==end()) throw invalid_argument("TomogramFinding::at(wstring), item not found");
		// в не-const версии исключение, т.к. возвращать пустую nonconst ссылку никак нельзя
		else return it->second;
	}

	void add_finding(const TomogramFinding &finding)
	{
		insert(make_pair(finding.doctor_id(), finding));
	}

	void	rename_doctor(const wstring &new_doctor_id, const wstring &old_doctor_id)
	{
		if(find(new_doctor_id) != end())
		{
			throw invalid_argument("TomogramFinding::rename_doctor, new doctor's name already exists");
		}
		auto	it = find(old_doctor_id);
		if(it==end()) return;
		it->second.doctor_id() = new_doctor_id;
		insert(make_pair(new_doctor_id, move(it->second)));
		erase(find(old_doctor_id));		
	}

	set<wstring> doctor_ids() const
	{
		set<wstring>	result;
		for(auto &finding: *this)
		{
			result.insert(finding.first);
		}
		return result;
	}

};



struct	one_doctor_one_study_findings_t : public vector<TomogramFinding> // этот тип -- строка таблицы
{
	// список находок одного доктора в одном исследовании
	// Может быть непустой список, но его элементы не привязаны ни к какому пространственному объекту.
	// Это происходит, если составляется синхронный список находок разных докторов. Тогда размер списка
	// у всех докторов будет одинаков, но если какая-то находка не подтверждена конкретным доктором,
	// у него она будет помечена флагом "пусто"
	bool	empty() const
	{
		bool result(true); // Изначально считаем контейнер пустым.
		for(auto &f: *this) result &= !f.empty(); // любая непустая находка обнуляет результат
		return result;
	}

	void	SetDoctorID(const wstring &in_doctor_id)
	{
		doctor_id = in_doctor_id;
		for(auto &finding: *this) finding.doctor_id() = in_doctor_id;
	}

	Dicom::complete_study_id_t	ids;
	wstring	doctor_id;
	wstring	comment;
};

struct reports_heap_t : public list<one_doctor_one_study_findings_t>
{
	// несортированная куча отчетов разных докторов и разных серий
};


struct one_study_findings_t : public list<one_doctor_one_study_findings_t>
{
	Dicom::complete_study_id_t	ids;
	one_study_findings_t(const Dicom::complete_study_id_t &in_ids) : ids(in_ids){}
	// находки, сделанные разными докторами в одной серии. пока не отличается от protocol_t, но должна будет отличаться
};



//typedef list<doctor_study_findings_t> protocol_t;

XRAD_END

#endif // StudyFindings_h__
