#include "pre.h"
#include "Identity.h"
/*!
	\file
	\date 2019/11/21 13:46
	\author kulberg

	\brief 
*/

XRAD_BEGIN

void check_or_copy(wstring &self, wstring &other)
{
	if(self.empty()) self = other;
	if(other.empty()) other = self;

//	XRAD_ASSERT_THROW_M(self == other, invalid_argument, convert_to_string(ssprintf(L"inconsistent indentifiers '%s'!='%s'", self.c_str(), other.c_str())));
}

void check_or_copy(wstring &self, const wstring &other)
{
	if(self.empty()) self = other;
}


void check_or_copy(double &self, double &other)
{
	if(self < other) self = other;
	if(other < self) other = self;

	XRAD_ASSERT_THROW_M(self == other, invalid_argument, convert_to_string(ssprintf(L"inconsistent values '%g'!='%g'", self, other)));
}


void check_or_copy(double &self, const double &other)
{
	if(self < other) self = other;
}


void check_or_copy(int &self, int &other)
{
	if(self < other) self = other;
	if(other < self) other = self;

	XRAD_ASSERT_THROW_M(self == other, invalid_argument, convert_to_string(ssprintf(L"inconsistent values '%g'!='%g'", self, other)));
}


void check_or_copy(int &self, const int &other)
{
	if(self < other) self = other;
}



#define coc(n) check_or_copy(n, other.n)


void study_identity::adjust_common_fields(study_identity &other)
{
	coc(accession_number);
	coc(study_id);
	coc(study_instance_uid);
	coc(study_confidence);
}


void study_identity::adjust_common_fields(const study_identity &other)
{
	coc(accession_number);
	coc(study_id);
	coc(study_instance_uid);
	coc(study_confidence);
}


void	XRayFrameIdentity::adjust_common_fields(XRayFrameIdentity &other)
{
	parent::adjust_common_fields(other);

	coc(dcm_filename);
	coc(sop_instance_uid);
}

void	XRayFrameIdentity::adjust_common_fields(const XRayFrameIdentity &other)
{
	parent::adjust_common_fields(other);

	coc(dcm_filename);
	coc(sop_instance_uid);
}


void XRayRoiIdentity::adjust_common_fields(XRayRoiIdentity &other)
{
	parent::adjust_common_fields(other);

	coc(roi_filename_with_extension);
	coc(roi_confidence);
}

void XRayRoiIdentity::adjust_common_fields(const XRayRoiIdentity &other)
{
	parent::adjust_common_fields(other);

	coc(roi_filename_with_extension);
	coc(roi_confidence);
}

void acquisition_identity::adjust_common_fields(acquisition_identity &other)
{
	parent::adjust_common_fields(other);

	coc(acquisition_number);
	coc(series_number);
}


void xrad::acquisition_identity::adjust_common_fields(const acquisition_identity &other)
{
	parent::adjust_common_fields(other);

	coc(acquisition_number);
	coc(series_number);
}


XRAD_END

