#include "pre.h"
#include "XRayRoiTaggingReport.h"
#include <FanTomCommons/Vocabulary/roi_vocabulary.h>
#include <XRADBasic/Sources/Utils/crayons.h>
#include <FanTomCommons/XRayRoiLabels.h>
/*!
	\file
	\date 2019/11/08 16:26
	\author kulberg

	\brief 
*/

XRAD_BEGIN



e_anatomical_location g_anatomical_location = e_anatomical_location::unknown;
bool	force_roi_errata = false;

void SetRoiErrataStatus(bool in_status)
{
	force_roi_errata = in_status;
}

bool	RoiErrataStatus()
{
	return force_roi_errata;
}


map<e_anatomical_location, wstring>	anatomic_location_names =
{
	make_pair(e_anatomical_location::mmg, L"mmg"),
	make_pair(e_anatomical_location::lung, L"lung")
};

map<e_anatomical_location, roi_type_vocabulary_t> vocabularies = 
{
	make_pair(e_anatomical_location::mmg, mmg_vocabulary()),
	make_pair(e_anatomical_location::lung, lung_vocabulary())

};


void SetAnatomicLocation(e_anatomical_location in_location)
{
	g_anatomical_location = in_location;
}

const wstring	XRayRoiTaggingReport::raw_filename_appendix_begin = L"-RoiSet_";
const wstring	XRayRoiTaggingReport::roi_shape = L"ellipse";


wstring	XRayRoiTaggingReport::legend() const
{
	return vocabularies[anatomical_location][roi_type_index].first;
//	return roi_type_vocabulary[roi_type_index].first;
}

ColorSampleUI8 XRayRoiTaggingReport::color() const
{
 	return vocabularies[anatomical_location][roi_type_index].second;
}

XRayRoiTaggingReport::XRayRoiTaggingReport() //: roi_shape(L"ellipse")
{
}


void	XRayRoiTaggingReport::Distort()
{
// 	if(force_csv_export_errata)
	tagging_system_id += L"+forced errata";
	roi_type_index += RandomUniformF64(-1, 1);
	double	ERR = std::hypot(loc.width(), loc.height()) / 8;
	double	ERR2 = ERR / 2;


	double	ex = RandomGaussian(ERR);
	double	ey = RandomGaussian(ERR);
	double	ewx = RandomUniformF64(-ERR2, ERR2);
	double	ewy = RandomUniformF64(-ERR2, ERR2);
	double	err_sum = hypot(hypot(ex, ey), hypot(ewx, ewy)) / hypot(loc.width(), loc.height());

// 	roi_confidence = range(roi_confidence - err_sum, 0, 1);
//	roi_confidence = RandomUniformF64(0, 1);
//	roi_confidence = sqrt(RandomUniformF64(0, 1));
	roi_confidence = range(RandomGaussian(0.9, 0.1), 0, 1);
	study_confidence = roi_confidence;

	loc.y1() += ey+ewy;
	loc.y2() += ey-ewy;
	loc.x1() += ex+ewx;
	loc.x2() += ex-ewx;
}


vector<wstring>	XRayRoiTaggingReport::export_csv_vector() const
{
	vector<wstring>	row(xray::n_csv_table_columns());

	xray::SetColumnValue(row, accession_number, xray::e_accession_no_column);
	xray::SetColumnValue(row, study_id, xray::e_study_id_column);
	xray::SetColumnValue(row, study_instance_uid, xray::e_study_instance_uid_column);
	xray::SetColumnValue(row, dcm_filename, xray::e_dcm_filename_column);
	xray::SetColumnValue(row, sop_instance_uid, xray::e_sop_instance_uid_column);
	xray::SetColumnValue(row, tagging_system_id, xray::e_tagging_system_id_column);


	xray::SetColumnValue(row, y(), xray::e_y_column);
	xray::SetColumnValue(row, x(), xray::e_x_column);
	xray::SetColumnValue(row, wy(), xray::e_wy_column);
	xray::SetColumnValue(row, wx(), xray::e_wx_column);

	xray::SetColumnValue(row, roi_type_index, xray::e_roi_type_column);
	xray::SetColumnValue(row, roi_confidence, xray::e_roi_confidence_column);

	xray::SetColumnValue(row, study_confidence, xray::e_study_confidence_column);

	return row;

}



void	XRayRoiTaggingReport::import_csv_row(const wstring &roi_table_filename_with_path, const vector<wstring> &row)
{
	roi_correspondence_applied = true;
	int	x, y, wx, wy;	
	static bool	irym_correction = YesOrNo("Apply IRYM correction", SavedGUIValue<bool>(true));

	XRAD_ASSERT_THROW(row.size() == xray::n_csv_table_columns());

	//	извлекает из имени файла идентификатор системы разметки. Также записывает в структуру имя файла без пути
	SplitFilename(roi_table_filename_with_path, nullptr, &roi_filename_with_extension, &tagging_system_id, nullptr);

	xray::GetColumnValue(accession_number, row, xray::e_accession_no_column);
	xray::GetColumnValue(study_id, row, xray::e_study_id_column);
	xray::GetColumnValue(study_instance_uid, row, xray::e_study_instance_uid_column);
	xray::GetColumnValue(dcm_filename, row, xray::e_dcm_filename_column);
	xray::GetColumnValue(sop_instance_uid, row, xray::e_sop_instance_uid_column);

//TODO: Наскоро для ИИ баттла заменено. Потом сделать лучше
// 	xray::GetColumnValue(tagging_system_id, row, xray::e_tagging_system_id_column);
	tagging_system_id = filename_without_extension(roi_table_filename_with_path);

	xray::GetColumnValue(y, row, xray::e_y_column);
	xray::GetColumnValue(x, row, xray::e_x_column);
	xray::GetColumnValue(wy, row, xray::e_wy_column);
	xray::GetColumnValue(wx, row, xray::e_wx_column);



	xray::GetColumnValue(roi_type_index, row, xray::e_roi_type_column);
	xray::GetColumnValue(roi_confidence, row, xray::e_roi_confidence_column);
	xray::GetColumnValue(study_confidence, row, xray::e_study_confidence_column);


	if(irym_correction)
	{
	//	#pragma message "поправка ради IRYM"
		x += wx/2; // они вместо центра указывали левый верхний угол.
		y += wy/2;
		roi_type_index = 1; // Находки были указаны без категорий. Принудительно вводим ЗНО, чтобы получить какое-то сравнение
	}

// 
// #pragma message see comment
// 	//заплата костыльная для некорректных данных на этапе подготовки к баттлу. Убрана после исправления разработчиками
// 	if(study_confidence > 1) study_confidence /= 100;
// 	if(roi_confidence > 1) roi_confidence /= 100;

	automatic_tagging = true;						//таблицы в этом формате поступают только от автоматических систем разметки
	anatomical_location = g_anatomical_location;	//во время одного сеанса используется разметка только для одного типа анатомического региона

	loc.x1() = x-wx/2;
	loc.x2() = x+wx/2;
	loc.y1() = y-wy/2;
	loc.y2() = y+wy/2;

}


void	XRayRoiTaggingReport::init_from_primary_roi_file(const wstring &filename_with_path)
{
	XRAD_ASSERT_THROW_M(g_anatomical_location != e_anatomical_location::unknown, invalid_argument, "ROI vocabubary not initialized");

	automatic_tagging = false;
	tagging_system_id = L"doctor";
	anatomical_location = g_anatomical_location;
	//roi_correspondence_applied = false;
	
	wstring	roi_filename_without_extension;

	SplitFilename(filename_with_path, nullptr, &roi_filename_with_extension, &roi_filename_without_extension, nullptr);
	auto found = roi_filename_without_extension.find(raw_filename_appendix_begin);
	XRAD_ASSERT_THROW_EX(found!=std::string::npos, invalid_argument);
	dcm_filename = wstring(roi_filename_without_extension.begin(), roi_filename_without_extension.begin() + found);
	wstring	digits = wstring(roi_filename_without_extension.begin() + found + raw_filename_appendix_begin.size(), roi_filename_without_extension.end());

	int counter1 = swscanf(digits.c_str(), L"%d_%d", &roi_type_index, &roi_no);
	int counter2 = swscanf(digits.c_str(), L"%d-%d", &roi_type_index, &roi_no);

	XRAD_ASSERT_THROW_EX(counter1==2 || counter2==2, invalid_argument);

	roi_confidence = 1;// в первичных файлах разметка врачей, которая считается надежной
}

//TODO загрузка из json не сделана

nlohmann::json XRayRoiTaggingReport::export_json() const
{
	nlohmann::json	result;

	result[xray::anatomical_location_tag()] = convert_to_string8(anatomic_location_names[anatomical_location]);
	result[xray::roi_shape_tag()] = convert_to_string8(roi_shape);
	result[xray::roi_type_tag()] = convert_to_string8(legend());
	result[xray::roi_type_index_tag()] = roi_type_index;
	result[xray::roi_filename_tag()] = convert_to_string8(roi_filename_with_extension);
	result[xray::tagging_system_id_tag()] = convert_to_string8(tagging_system_id);

	result[xray::x_tag()] = loc.center().x();
	result[xray::y_tag()] = loc.center().y();

	result[xray::x_size_tag()] = loc.width();
	result[xray::y_size_tag()] = loc.height();
	result[xray::roi_confidence_tag()] = roi_confidence;
	result[xray::study_confidence_tag()] = study_confidence;

	return result;
}


XRAD_END

