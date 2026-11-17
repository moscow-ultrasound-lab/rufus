#ifndef XRayRoiLabels_h__
#define XRayRoiLabels_h__

/*!
	\file
	\date 2019/12/10 19:45
	\author kulberg

	\brief  
*/

XRAD_BEGIN

namespace xray
{

enum csv_colulmns
{
	e_row_no_column =		1,//машина не использует, может потребоваться для навигации экспертов по таблице
	e_finding_id_column =	2,//машина не использует, может потребоваться для обратной связи с разработчиком ИИ
	e_accession_no_column = 3,
	e_study_id_column =		4,
	e_study_instance_uid_column = 5,
	e_dcm_filename_column = 6,
	e_sop_instance_uid_column =	7,

	e_y_column =	8,
	e_x_column =	9,
	e_wy_column =	10,
	e_wx_column =	11,
	e_roi_type_column = 12,
	e_roi_confidence_column = 13,
	e_study_confidence_column = 14,
	e_tagging_system_id_column = 15
};

const vector<wstring> &csv_header();

inline size_t	n_csv_table_columns() { return 15; }


template<class T>
void	GetColumnValue(T &value, const vector<wstring> &row, int no);

template<class T>
void	SetColumnValue(vector<wstring> &row, const T &value, int no);


//	тэги для записи в json
// inline	string	_tag(){ return ""; }

inline	string	study_id_tag(){ return "study_id"; }
inline	string	accession_number_tag(){ return "accession_number"; }
inline	string	study_instance_uid_tag(){ return "study_instance_uid"; }
inline	string	instances_tag(){ return "instances"; }


inline	string	diagnosis_tag(){ return "diagnosis"; }
inline	string	study_confidence_tag(){ return "study_confidence"; }
inline	string	tagging_system_id_tag(){ return "tagging_system_id"; }
inline	string	roi_filename_tag(){ return "roi_filename"; }
inline	string	automatic_tagging_tag(){ return "automatic_tagging"; }
inline	string	roi_type_index_tag(){ return "roi_type_index"; }
inline	string	dcm_filename_tag(){ return "dcm_filename"; }
inline	string	acquisition_number_tag(){ return "acquisition_number"; }
inline	string	series_number_tag(){ return "series_number"; }
inline	string	instance_number_tag(){ return "instance_number"; }
inline	string	sop_instance_uid_tag(){ return "sop_instance_uid"; }
inline	string	rois_tag(){ return "rois"; }
inline	string	anatomical_location_tag(){ return "anatomical_location"; }

inline	string	y_tag(){ return "y"; }
inline	string	x_tag(){ return "x"; }
inline	string	y_size_tag(){ return "y_size"; }
inline	string	x_size_tag(){ return "x_size"; }
inline	string	roi_confidence_tag(){ return "confidence"; }//строка без префикса, т.к. используется внутри объекта roi
inline	string	roi_shape_tag(){ return "roi_shape"; }
inline	string	roi_type_tag(){ return "roi_type"; }
// inline	string	_tag(){ return ""; }
// inline	string	_tag(){ return ""; }


}//namespace xray


XRAD_END

#endif // XRayRoiLabels_h__
