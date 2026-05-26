#include "pre.h"
#include "TomogramFinding.h"


/*!
	\file
	\date 2018/07/09 15:41
	\author kulberg

	\brief 
*/

XRAD_BEGIN

bool TomogramFinding::malignant() const
{
//	return true;
	if(!expert_decision().size()) return false;
	return expert_decision()[0].use_for_machine_learning;
}


bool TomogramFinding::rejected() const
{
	if(!expert_decision().size()) return false;

	switch(expert_decision()[0].decision)
	{
		case expert_protocol::confirmed:
		case expert_protocol::confirmed_partially:
			return false;

		case expert_protocol::rejected:
		case expert_protocol::doubt:
			return true;
	}

	throw logic_error("TomogramFinding::rejected, invalid expert decision");
}

bool TomogramFinding::wrong_size() const
{
	if(expert_decision().empty()) return false;
	return !expert_decision().front().proper_size;
}

bool xrad::TomogramFinding::operator==(const TomogramFinding &f) const
{
	return
		center==f.center &&
		z_type() == f.z_type() &&
		type()==f.type() &&
		version() == f.version() &&
		empty()==f.empty() &&
		doctor_id()==f.doctor_id() &&
		series_no() == f.series_no() &&
		diameter() == f.diameter() &&
		expert_decision() == f.expert_decision() &&
		confidence() == f.confidence();
}


bool quite_different(const TomogramFinding &f1, const TomogramFinding &f2)
{
	if(f1.empty() || f2.empty()) return true;
	if(f1.z_type() != f2.z_type())
	{
		return true;
	}
	if(!f1.expert_decision().empty() && !f2.expert_decision().empty())
	{
		if(f1.rejected() != f2.rejected())
			return true;
	}
	if(f1.wrong_size() || f2.wrong_size())
		return true;

	return false;
}



double distance(const TomogramFinding& f1, const TomogramFinding& f2)
{
	if(f1.empty() || f2.empty()) return max_double();
	return norma(f2.center - f1.center);
}

const vector<string8> &z_coordinate_legend() 
{
	static const vector<string8> result({"unknown", "mm", "slice"});
	XRAD_ASSERT_THROW_M(result.size() == size_t(TomogramFinding::z_type_t::n_possible_z_types), out_of_range, "z_coordinate_legend");
	return result; 
};

const vector<wstring> &z_coordinate_legend_ru()
{
	static const vector<wstring> result({L"?", L"мм", L"№ среза"});
	XRAD_ASSERT_THROW_M(result.size() == size_t(TomogramFinding::z_type_t::n_possible_z_types), out_of_range, "z_coordinate_legend_ru");
	return result;
};

const string8	&z_type_string(TomogramFinding::z_type_t c)
{
	return z_coordinate_legend()[size_t(c)];
}

const wstring	&z_type_string_ru(TomogramFinding::z_type_t c)
{
	return z_coordinate_legend_ru()[size_t(c)];
}

//---------------

const vector<string8> &expert_decision_legend()
{
	static const vector<string8> result({"unknown", "rejected", "doubt", "confirmed_partially", "confirmed"});
	XRAD_ASSERT_THROW_M(result.size() == expert_protocol::n_possible_decisions, out_of_range, "expert_decision_legend");
	return result;
}

const vector<wstring> &expert_decision_legend_ru()
{
	static const vector<wstring> result({L"Неизвестно", L"Не согласен", L"Есть сомнения", L"Частично согласен", L"Полностью согласен"});
	XRAD_ASSERT_THROW_M(result.size() == expert_protocol::n_possible_decisions, out_of_range, "expert_decision_legend_ru");
	return result;
}


const string8	&expert_decision_string(expert_protocol::decision_t c)
{
	return expert_decision_legend()[size_t(c)];
}

const wstring	&expert_decision_string_ru(expert_protocol::decision_t c)
{
	return expert_decision_legend_ru()[size_t(c)];
}

expert_protocol::decision_t expert_decision_enum(size_t e)
{
	XRAD_ASSERT_THROW_M(in_range(e, 0, expert_protocol::n_possible_decisions-1), out_of_range, "expert_decision_enum");
	return static_cast<expert_protocol::decision_t>(e);
}

TomogramFinding::z_type_t	z_type_enum(size_t e)
{
	XRAD_ASSERT_THROW_M(in_range(e, 0, size_t(TomogramFinding::z_type_t::n_possible_z_types)-1), out_of_range, "z_type_enum");
	return static_cast<TomogramFinding::z_type_t>(e);
}



namespace
{
string8 version_key(){ return u8"version"; }
string8 type_key(){ return u8"type"; }
string8 x_key(){ return u8"x"; }
string8 y_key(){ return u8"y"; }
string8 z_key(){ return u8"z"; }
string8 diameter_key(){ return u8"diameter (mm)"; }
string8 z_type_key(){ return u8"z type"; }
string8 series_no_key(){ return u8"series no"; }
string8 expert_decision_key(){ return u8"expert decision"; }
string8 confidence_key(){ return u8"confidence"; }


}//namespace

json	expert_to_json(const expert_protocol &e)
{
	json	j;
	j["decision"] = expert_decision_string(e.decision);
	j["id"] = convert_to_string8(e.id);
	j["comment"] = convert_to_string8(e.comment);
	j["type"] = convert_to_string8(e.type);
	j["proper size"] = e.proper_size;
	j["machine learning"] = e.use_for_machine_learning;
	return j;
}



json	finding_to_json(const TomogramFinding &finding)
{
	if(finding.empty())
	{
		return json();
	}

	json	result;

	double	version = round_n(wcstod(finding.version().c_str(), NULL), 2);

	if(version >= 4.1)
	{
		result[confidence_key()] = finding.confidence();
		result[version_key()] = convert_to_string8(finding.version());
	}
	else
	{
		result[confidence_key()] = 1;
		result[version_key()] = convert_to_string8(L"4.1");
	}

	result[type_key()] = convert_to_string8(finding.type());


	result[x_key()] = finding.center.x();
	result[y_key()] = finding.center.y();
	result[z_key()] = finding.center.z();
	result[diameter_key()] = finding.diameter().mm();

	result[z_type_key()] = z_type_string(finding.z_type());
	if(finding.series_no().size()) result[series_no_key()] = convert_to_string8(finding.series_no());


	result[expert_decision_key()];//добавляем нулевую экспертную оценку
	for(auto ex: finding.expert_decision())
	{
		result[expert_decision_key()].push_back(expert_to_json(ex));
	}

	return result;
}


template<>
bool	try_get(physical_length &value, string8 key, const json &j)
{
	double	buffer;
	if(!try_get(buffer, key, j)) return false;
	value = mm(buffer);
	return true;
}

template<>
bool	try_get(TomogramFinding::z_type_t &value, string8 key, const json &j)
{
	string8	buffer;
	if(!try_get(buffer, key, j)) return false;
	auto	found = std::find(z_coordinate_legend().begin(), z_coordinate_legend().end(), buffer);
	value = z_type_enum(found - z_coordinate_legend().begin());
	return true;
}

template<>
bool	try_get(expert_protocol::decision_t &e, string8 key, const json &j)
{
	string8	buffer;
	if(!try_get(buffer, key, j)) return false;
	auto	found = std::find(expert_decision_legend().begin(), expert_decision_legend().end(), buffer);
	if(found==expert_decision_legend().end()) return false;
	
	e = expert_decision_enum(found - expert_decision_legend().begin());

	return true;
}

const expert_protocol json_to_expert(const json	&j)
{
	expert_protocol	e;

	try_get(e.decision, "decision", j);
	try_get(e.id, "id", j);
	try_get(e.comment, "comment", j);
	try_get(e.type, "type", j);
	try_get(e.proper_size, "proper size", j);
	try_get(e.use_for_machine_learning, "machine learning", j);

	return e;
}


TomogramFinding json_to_finding(const wstring &doctor_id, const json &j)
{
	TomogramFinding finding(doctor_id);

 	if(j.empty())	return finding;

	bool	not_empty(true);// считаем находку непустой изначально, но по отсутствию любого из четырех ключевых признаков сбрасываем в 0
 
	try_get(finding.version(), version_key(), j);

	try_get(finding.type(), type_key(), j);
	not_empty &= try_get(finding.center.x(), x_key(), j);
	not_empty &= try_get(finding.center.y(), y_key(), j);
	not_empty &= try_get(finding.center.z(), z_key(), j);
 	not_empty &= try_get(finding.diameter(), diameter_key(), j);
 	try_get(finding.z_type(), "z type", j);
	try_get(finding.series_no(), series_no_key(), j);
	
	finding.SetEmptiness(!not_empty);

	json	expert;
	if(try_get(expert, expert_decision_key(), j))
	{
		for(auto &ex: expert)
		{
			finding.expert_decision().push_back(json_to_expert(ex));
		}
	}

	double	version = round_n(wcstod(finding.version().c_str(), NULL), 2);

	if(version >= 4.1)
	{
		try_get(finding.confidence(), confidence_key(), j);
	}
	else
	{
		if(finding.type()==L"с" || finding.type()==L"п" || finding.type()==L"м") 
		{
			finding.confidence() = 1;
		}
		else
		{
			finding.version() = L"4.1";
			double	palliative_confidence = wcstod(finding.type().c_str(), NULL);
			if(palliative_confidence)
			{
				finding.confidence()=palliative_confidence;
				finding.type() = L"?";
			}
			else
			{
				finding.confidence()=1;
			}
		}

	}


	return finding;
}

XRAD_END

