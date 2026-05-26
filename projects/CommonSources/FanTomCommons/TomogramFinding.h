#ifndef Finding_h__
#define Finding_h__

/*!
	\file
	\date 2018/07/09 15:42
	\author kulberg

	\brief  Описание одного очага в формате (x,y,z,r).
*/

#include <FanTomCommons/json_utils.h>



XRAD_BEGIN



struct expert_protocol
{
	using self = expert_protocol;
	enum decision_t
	{
		unknown = 0,
		rejected = 1,
		doubt = 2,
		confirmed_partially = 3,
		confirmed = 4,

		n_possible_decisions
	};

	bool	use_for_machine_learning;

	decision_t decision;
	wstring comment;
	wstring	id;
	wstring	type;
	bool	proper_size;

	expert_protocol() : decision(unknown), proper_size(false), use_for_machine_learning(false){}
	bool operator==(const self &e) const
	{
		return 
			decision == e.decision && 
			comment==e.comment && 
			id==e.id && 
			type==e.type && 
			proper_size==e.proper_size;
	}
};


class TomogramFinding
{
	// одна находка в каком-то исследовании

public:
	using self = TomogramFinding ;


	enum class z_type_t
	{
		unknown = 0,
		mm = 1,
		slice = 2,

		n_possible_z_types
		// Находки могут быть обозначены в срезах или в мм. В общем случае неясно, какой врач как обозначал, поэтому по умолчанию у всех будет unknown.
		// Одна из задач экспертов эти вещи приводить к нужной координате
	};

private:
	wstring	m_type;
	wstring	m_version;
	wstring	m_doctor_id;
	wstring	m_series_no;
	double	m_nodule_confidence; // Мнение лица, поставившего отметку. Имеет смысл для систем ИИ. Для врачей по умолчанию считается 1
	double	m_study_confidence;

	bool	m_empty;
	physical_length	m_diameter;// ранее было обозначено как радиус, но вообще говоря, это диаметр
	z_type_t	m_z_type;
	vector<expert_protocol> m_expert;

public:

	point3_F64	center;

	TomogramFinding(const wstring &in_doctor_id,
					double in_x, double in_y, double in_z, 
					physical_length in_d, const wstring &in_type,
					double in_nodule_confidence, double in_study_confidence, 
					const wstring &in_series_no = L"?") :
		center(in_z, in_y, in_x),
		m_diameter(in_d),
		m_type(in_type),
		m_empty(false),
		m_z_type(z_type_t::unknown),
//		m_version(L"4.0"), //версии очагов 1-3 были в таблицах декабря 2017 г.
		m_version(L"4.1"), // по сравнению с 4.0 добавлена запись о confidence находки
		m_doctor_id(in_doctor_id),
		m_series_no(in_series_no),
		m_nodule_confidence(in_nodule_confidence),
		m_study_confidence(in_study_confidence)
		{}

	TomogramFinding(const wstring &in_doctor_id) : m_doctor_id(in_doctor_id), center(-1, -1, -1), m_diameter(mm(0)), m_empty(true), m_nodule_confidence(0), m_study_confidence(0)
	{
	};

	const physical_length	&diameter() const { return m_diameter; }
	physical_length	&diameter() { return m_diameter; }

	const bool empty() const{ return m_empty; }
	void	SetEmptiness(bool in_empty) { m_empty = in_empty; }
	const double	&confidence() const { return m_nodule_confidence; }
	double	&confidence() { return m_nodule_confidence; }

	bool	operator == (const TomogramFinding &f) const;
	static physical_length min_diameter(){ return mm(4); }

	z_type_t	&z_type() { return m_z_type; }
	const z_type_t	&z_type() const { return m_z_type; }

	vector<expert_protocol>	&expert_decision() { return m_expert; }
	const vector<expert_protocol>	&expert_decision() const { return m_expert; }

	wstring	&version(){ return m_version; }
	const wstring	&version() const { return m_version; }

	wstring	&type(){ return m_type; }
	const wstring	&type() const { return m_type; }

	wstring	&doctor_id(){ return m_doctor_id; }
	const wstring	&doctor_id() const { return m_doctor_id; }

	wstring	&series_no(){ return m_series_no; }
	const wstring	&series_no() const { return m_series_no; }

	bool	malignant() const;
	bool	rejected() const;

	bool	wrong_size() const;

};



bool	quite_different(const TomogramFinding &f1, const TomogramFinding &f2);
double distance(const TomogramFinding& f1, const TomogramFinding& f2);


json	finding_to_json(const TomogramFinding &nodule);
TomogramFinding json_to_finding(const wstring &doctor_id, const json &j);

const vector<string8> &expert_decision_legend();
const vector<wstring> &expert_decision_legend_ru();
const string8	&expert_decision_string(expert_protocol::decision_t c);
const wstring	&expert_decision_string_ru(expert_protocol::decision_t c);
expert_protocol::decision_t expert_decision_enum(size_t e);

const vector<string8> &z_coordinate_legend();
const vector<wstring> &z_coordinate_legend_ru();
const string8	&z_type_string(TomogramFinding::z_type_t c);
const wstring	&z_type_string_ru(TomogramFinding::z_type_t c);
TomogramFinding::z_type_t	z_type_enum(size_t e);

XRAD_END

#endif // Finding_h__
