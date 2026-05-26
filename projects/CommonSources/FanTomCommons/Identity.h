#ifndef Identity_h__
#define Identity_h__

/*!
	\file
	\date 2019/11/21 13:38
	\author kulberg

	\brief  
*/

XRAD_BEGIN


//!	Информация о системе, разметившей одно исследование. Содержит идентификатор и (опционально) комментарий
struct	tagging_system_identity : private pair<wstring, wstring>
{
	PARENT(pair<wstring, wstring>);

public:

	wstring	&id(){ return first; }
	wstring	&comment(){ return second; }

	const wstring	&id() const { return first; }
	const wstring	&comment() const { return second; }

	tagging_system_identity(){}
	tagging_system_identity(const wstring &id, const wstring &comment) : parent(id, comment){}
};


struct study_identity
{
	wstring	accession_number;
	wstring	study_id;
	wstring	study_instance_uid;

	double	study_confidence = 0;

//!	Часть информации об исследовании дублируется (например, study_id встречается в информации об исследовании, о кадре и о roi).
//	При сборке roi одного исследования проверяет, соответствуют ли идентификаторы добавляемого объекта идентификаторам
//	всего исследования. Для пустых идентификаторов копирует текущее значение
	void	adjust_common_fields(study_identity &other);
	void	adjust_common_fields(const study_identity &other);

	bool	same_study(const study_identity &other){ return other.study_id == study_id && other.accession_number==accession_number && other.study_instance_uid == study_instance_uid; }
};


struct acquisition_identity : public study_identity
{
	PARENT(study_identity);
	// возможно, для рентгенограмм эти поля не очень нужны:
	int	acquisition_number;
	int series_number;

	using parent::adjust_common_fields;
	void	adjust_common_fields(acquisition_identity &other);
	void	adjust_common_fields(const acquisition_identity &other);
};

struct XRayFrameIdentity : public acquisition_identity
{
	PARENT(acquisition_identity);

	wstring	dcm_filename;
	wstring	sop_instance_uid;

	// поля, которые, возможно, не понадобятся
	size_t	instance_number = 0;
	size_t	frame_number = 0;

	using parent::adjust_common_fields;
	void	adjust_common_fields(XRayFrameIdentity &other);
	void	adjust_common_fields(const XRayFrameIdentity &other);
};

struct	XRayRoiIdentity : public XRayFrameIdentity 
{
	PARENT(XRayFrameIdentity);
	wstring	roi_filename_with_extension;
	
	//! \brief Для разметки врачей false, для ИИ систем true
	bool	automatic_tagging;

	//! \brief Идентификатор системы разметки. Для первичных протоколов от врачей doctor. Для протоколов автоматической разметки берется из roi_filename (без расширения)
	wstring	tagging_system_id;

	double	roi_confidence = 0;

	using parent::adjust_common_fields;
	void	adjust_common_fields(XRayRoiIdentity &other);
	void	adjust_common_fields(const XRayRoiIdentity &other);
};

XRAD_END

#endif // Identity_h__
