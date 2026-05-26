#include "pre.h"
#include "XRayRoiLabels.h"
/*!
	\file
	\date 2019/12/10 20:38
	\author kulberg

	\brief 
*/

XRAD_BEGIN

namespace xray
{

const vector<wstring> &csv_header()
{
	static const vector<wstring> result =
	{
		L"no",
		L"nodule_id",
		convert_to_wstring(accession_number_tag()),
		convert_to_wstring(study_id_tag()),
		convert_to_wstring(study_instance_uid_tag()),
		convert_to_wstring(dcm_filename_tag()),
		convert_to_wstring(sop_instance_uid_tag()),

		convert_to_wstring(y_tag()),
		convert_to_wstring(x_tag()),
		convert_to_wstring(y_size_tag()),
		convert_to_wstring(x_size_tag()),
		convert_to_wstring(roi_type_index_tag()),
		convert_to_wstring(roi_confidence_tag()),
		convert_to_wstring(study_confidence_tag()),
		convert_to_wstring(tagging_system_id_tag()),
	};

	XRAD_ASSERT_THROW(result.size() == n_csv_table_columns());

	return result;
}


template<>
void	SetColumnValue<wstring>(vector<wstring> &row, const wstring &value, int no)
{
	if(!value.empty()) row[no-1] = value;
}

template<>
void	SetColumnValue<double>(vector<wstring> &row, const double &value, int no)
{
	row[no-1] = ssprintf(L"%g", value);
}

template<>
void	SetColumnValue<int>(vector<wstring> &row, const int &value, int no)
{
	row[no-1] = ssprintf(L"%d", value);
}


template<>
void	GetColumnValue<wstring>(wstring &value, const vector<wstring> &row, int no)
{
	value = row[no-1];
}

template<>
void	GetColumnValue<int>(int &value, const vector<wstring> &row, int no)
{
	if(!row[no-1].empty())
	{
		value = wcstol(row[no-1].c_str(), nullptr, 10);
	}
	else
	{
		value = 0;
	}
}

template<>
void	GetColumnValue<double>(double &value, const vector<wstring> &row, int no)
{
	if(!row[no-1].empty())
	{
		value = wcstod(row[no-1].c_str(), nullptr);
	}
	else
	{
		value = 0;
	}
}

}//namespace xray

XRAD_END

