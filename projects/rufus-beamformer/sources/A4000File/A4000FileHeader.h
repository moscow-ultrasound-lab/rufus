#ifndef __a4000_file_header_h
#define __a4000_file_header_h


#include "ProbeOptions/ScanParams.h"
#include <XRADSystem/Sources/IniFile/XRADIniFile.h>

//#include <DataArray2D.h>
#include <XRADBasic/Containers.h>

XRAD_BEGIN

//----------------------------------------------------------------------
//
//	формат A4000 состоит из одного или нескольких бинарных
//	файлов и текстового файла в формате *.ini с описанием структуры файла,
//	а также (возможно) с данными анамнеза пациента
//	от заголовка наследуютсЯ классы A4000DataReader и A4000DataWriter
//


struct	PatientInfo
	{

	string 	ID;

	string	surname;
	string	name;
	string	patronymic;

	int	birth_year;
	int	birth_month;
	int	birth_day;

	string	comment;
	};

class	A4000DataHeader
	{
	protected:
	
		void	ReadScanParamsV1(IniFileReader &ir);
		void	ReadScanParamsV2(IniFileReader &ir);
		void	ReadAnamnesis_V1_V2(IniFileReader &ir);

		void	ReadDataFilesInfo_V1(IniFileReader &ir);
		void	ReadDataFilesInfo_V2(IniFileReader &ir);


	public:
		size_t n_data_files;
		size_t n_frames_per_file;

		string	hdr_file_name;
		
		DataArray<string> dat_file_name;
		string	dat_file_pattern;
		
		//	сигнатура данных: несколько байт, которые могут быть добавлены
		//	в начало бинарного файла. по умолчанию отсутствует
		string	data_signature;
		size_t	data_signature_size;
		
		// 	фиксированное количество байт, которое добавлЯетсЯ в начало каждой
		//	ак. строки. по умолчанию 0
		size_t	ray_header_size;

		ScanParams scan_params;
		PatientInfo patient_info;
		ioNumberOptions	data_format;
		
		A4000DataHeader() : n_frames_per_file(0), n_data_files(1) {}
		
		void	ReadHeaderFile(string fn);

	public:
		int	year, day, month;
		int	hour, minute;
	};



XRAD_END

#endif   // __a4000_file_header_h
