#include "pre.h"
#include "S400_SpectromedRFDataImporter.h"
#include "A4000RFDataImporter.h"
#include <XRADBasic/Math.h>
#include <XRADSystem/Sources/IniFile/XRADIniFile.h>

//--------------------------------------------------------
//
//	original header conversion
//
//--------------------------------------------------------

XRAD_BEGIN


A4000RFDataImporter :: A4000RFDataImporter()
	{
	sp.sfd.sample_rate = MHz(10);
	}


bool	A4000RFDataImporter :: OpenRFData()
	{
	string file_name;
	GetFileNameRead(file_name, "A4000 RF data");
	
	bool	header_file_open_successful(false);
	
	while(!header_file_open_successful)
		{
		try
			{
			data_reader.ReadHeaderFile(file_name);
			data_reader.OpenDataFiles();

			header_file_open_successful = true;
			}
		catch(ini_file_error &ex)
			{
			Error(ssprintf("%s. You can edit file right now and try again. Add/rename parameters according to the rename table (see comments in ReadScanParamsV2() function)", ex.what()));
			}
		}
	
	n_focuses = data_reader.scan_params.mfo.n_focuses;
	n_frames = data_reader.n_frames_per_file * data_reader.n_data_files * n_focuses;
	sp = data_reader.scan_params;
	
//	dz = sp.depth_range()/sp.sfd.n_samples;
//	dz = sp.depth_step();
	dz = sp.pfd.depth_range()/sp.sfd.n_samples;
	z_window_center = sp.pfd.depth_range()/2;
	z_window_width = sp.pfd.depth_range();

	
	frame_buffer.realloc(sp.sfd.n_rays, sp.sfd.n_samples);	
	return true;
	}



string	A4000RFDataImporter :: GetComment()
	{
	static	char	birth_date[256];
	
	if(data_reader.patient_info.birth_day && data_reader.patient_info.birth_month && data_reader.patient_info.birth_year)
		sprintf(birth_date, " %.2d.%.2d.%.4d г. р.\n", 
			data_reader.patient_info.birth_day,
			data_reader.patient_info.birth_month,
			data_reader.patient_info.birth_year);
	else sprintf(birth_date, "\n");
		
	return	data_reader.patient_info.surname + " " +
		data_reader.patient_info.name + " " +
		data_reader.patient_info.patronymic +
		string(birth_date) +
		data_reader.patient_info.comment;
	};




void	A4000DataReader :: GetA4000Frame(ComplexFunction2D_F32 &container, size_t frame_no)
	{

	shared_cfile	&frame_file = GetFrameFile(frame_no);
	
	ComplexFunctionF32	read_buffer(scan_params.n_samples_total());

	for(size_t i = 0; i < container.vsize(); ++i)
		{
		frame_file.seek(RayDataOffset(frame_no, i), SEEK_SET);
//		frame_file.fread_numbers(container.row(i), data_format);
		frame_file.read_numbers(read_buffer, data_format);
		std::copy(read_buffer.begin() + scan_params.sfroi.n_samples_to_skip_at_start, read_buffer.end() - scan_params.sfroi.n_samples_to_skip_at_end, container.row(i).begin());
		}
	}



void	A4000RFDataImporter :: GetFrame(size_t frame_no)
	{
	data_reader.GetA4000Frame(frame_buffer, frame_no);
	if(sp.sfroi.carrier_offset != MHz(0))
		{
		double	carrier_roll_factor = (sp.sfroi.carrier_offset/sp.sfd.sample_rate)*two_pi();
		ComplexFunctionF32	roll_function(sp.sfd.n_samples);
		for(size_t i = 0; i < sp.sfd.n_samples; ++i)
			{
			roll_function[i] = polar(1, double(i)*carrier_roll_factor);
			}
#pragma message "алгоритм работает только при считывании данных посли гильберта, нужно потом доработать"
		for(size_t i = 0; i < sp.sfd.n_rays; ++i)
			{
			frame_buffer.row(i) %= roll_function;
			}
		}
	}



XRAD_END
