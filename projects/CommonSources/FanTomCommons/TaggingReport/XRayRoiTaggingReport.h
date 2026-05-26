#ifndef roi_file_info_h__
#define roi_file_info_h__

/*!
	\file
	\date 2019/11/08 16:26
	\author kulberg

	\brief  
*/

#include <XRADBasic/ThirdParty/nlohmann/json.hpp>
#include <FanTomCommons/Identity.h>
#include "Q:\projects\AutoStudiesSort\QualityMetrics\StudyLoadSettings.h"//TODO плохое включение


XRAD_BEGIN


struct	XRayRoiTaggingReport : public XRayRoiIdentity
{
	int	roi_type_index;
	int	roi_no;
	e_anatomical_location	anatomical_location;

	//! \brief На ноябрь 2019 никаких форм кроме эллипса не предусмотрено. Используется const для предотвращения случайного использования других значений
	static const wstring roi_shape;
	range2_I32	loc;
	bool	roi_correspondence_applied = false;


	//!	\brief Имя файла roi начинается с имени дайкома.
	//!	MG_1.2.276.0.7230010.3.1.4.50143260.1368.1571935787.113-RoiSet_01_01
	//!	Признаком окончания служит данная константа
	static const wstring	raw_filename_appendix_begin;

	XRayRoiTaggingReport();
	XRayRoiTaggingReport(const XRayRoiTaggingReport &other) = default;

	void	Distort();

	void	init_from_primary_roi_file(const wstring &filename);
	void	import_csv_row(const wstring &filename, const vector<wstring> &table_row);
	vector<wstring>	export_csv_vector() const;

	nlohmann::json	export_json() const;

	int	x() const { return loc.center().x(); }
	int	y() const { return loc.center().y(); }
	int	wx() const { return loc.width(); }
	int	wy() const { return loc.height(); }


	wstring	legend() const;
	ColorSampleUI8 color() const;
};

//static	bool	force_roi_errata;
void	SetRoiErrataStatus(bool in_status);
bool	RoiErrataStatus();


XRAD_END

#endif // roi_file_info_h__
