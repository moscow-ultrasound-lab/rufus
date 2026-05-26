#include "pre.h"


#include <A4000File/A4000FileHeader.h>
//#include <XRADGUI.h>


XRAD_BEGIN

using namespace DataArrayIOAuxiliaries;

void	A4000DataHeader :: ReadHeaderFile(string in_hdr_filename)
	{	
	hdr_file_name = in_hdr_filename;
		
	IniFileReader	ir;
	ir.open(hdr_file_name.c_str());
	
	double	header_file_version(0);
	
	try
		{
		ir.set_section("HEADER_FILE_INFO");
		header_file_version = ir.read_double("header_file_version", 1.0);
		}
	catch(ini_file_error &)
		{
		header_file_version = 1.0;
		}
	
	
	if(header_file_version == 1.0)
		{
		ReadDataFilesInfo_V1(ir);
		ReadScanParamsV1(ir);
		}
	else if(header_file_version == 2.0)
		{
		ReadDataFilesInfo_V2(ir);
		ReadScanParamsV2(ir);
		}
	else FatalError(ssprintf("A4000DataHeader::ReadHeaderFile -- Unknown A4000 header file version (%.2g)", header_file_version));

	ReadAnamnesis_V1_V2(ir);
	}



void	A4000DataHeader :: ReadDataFilesInfo_V1(IniFileReader &ir)
	{
	ir.set_section("DATA_FILE_INFO");
	n_data_files = ir.read_int("n_data_files", 1);
	dat_file_pattern = ir.read_string("data_file_name");

	data_format = str_to_io(ir.read_string("data_format").c_str());
	data_signature = ir.read_string("data_signature", "A400");
	
	if(data_signature == string("A4000")) data_signature = string("A400");
	// запоздалаЯ заплатка: почему-то в многих заголовках вписано ошибочное
	// значение; проще это игнорировать сразу
	
	data_signature_size = strlen(data_signature.c_str());
	
	ray_header_size = ir.read_int("ray_header_size", 0);
	n_frames_per_file = ir.read_int("n_frames");
	}


void	A4000DataHeader :: ReadDataFilesInfo_V2(IniFileReader &ir)
	{
	ir.set_section("DATA_FILE_INFO");
	n_data_files = ir.read_int("n_data_files", 1);
	dat_file_pattern = ir.read_string("data_file_name");
	
	data_format = str_to_io(ir.read_string("data_format").c_str());
	data_signature = ir.read_string("data_signature", "A400");
		
	data_signature_size = strlen(data_signature.c_str());
	
	ray_header_size = ir.read_int("ray_header_size", 0);
	n_frames_per_file = ir.read_int("n_frames_per_file");
	}


void	A4000DataHeader :: ReadScanParamsV1(IniFileReader &ir)
	{
	ir.set_section("SCAN_PARAMS");

	scan_params.mfo.realloc(ir.read_int("n_focuses"));
	scan_params.sfd.n_rays = ir.read_int("n_rays");

//	scan_params.pfd.start_angle = degrees(ir.read_double("start_angle"));//*PI/180;
//	scan_params.pfd.end_angle = degrees(ir.read_double("end_angle"));//*PI/180;
	
	physical_angle a0 = degrees(ir.read_double("start_angle"));//*PI/180;
	physical_angle a1 = degrees(ir.read_double("end_angle"));//*PI/180;

	scan_params.sfd.sample_rate = MHz(ir.read_double("sample_rate_mhz"));
//	scan_params.carrier_frequency = MHz(ir.read_double("carrier_frequency_mhz"));
	scan_params.sfroi.carrier_offset = MHz(0);
	scan_params.sound_speed = cm_sec(ir.read_double("sound_sped_cm_sec", 1.54e5));

	scan_params.sfd.n_samples = ir.read_int("n_samples");
	scan_params.sfroi.n_samples_skipped = ir.read_int("start_sample");
	scan_params.sfroi.n_samples_to_skip_at_start = 0;
	scan_params.sfroi.n_samples_to_skip_at_end = positive(ir.read_int("end_sample") - scan_params.sfd.n_samples);


	physical_length	probe_radius = cm(ir.read_double("array_length_cm"))/(a1-a0).radians();	
	scan_params.upo.InitProbeConvex(probe_radius, a1-a0, scan_params.sfd.n_rays, 32);

//	scan_params.pfd.scanning_trajectory_length = (scan_params.upo.ScanningRadius() + scan_params.min_depth()) * (a1-a0).radians();
//	scan_params.pfd.depth_range = cm(scan_params.sfd.n_samples)*scan_params.sound_speed.cm_sec()/(2.*scan_params.sfd.sample_rate.Hz());
	
	scan_params.pfd.SetFrameSector(
		(scan_params.upo.ScanningRadius() + scan_params.calculate_min_depth()) * (a1-a0).radians(),
		cm(scan_params.sfd.n_samples)*scan_params.sound_speed.cm_sec()/(2.*scan_params.sfd.sample_rate.Hz()),
		a0,
		a1
		);
	
	for(size_t i = 0; i < scan_params.mfo.n_focuses && i < 4; i++)
		{
		static char	param_name[256];// = AnsiString("focus_") + IntToStr(i);
		sprintf(param_name, "focus_%d", int(i));
		scan_params.mfo[i] = cm(ir.read_double(param_name));
		}	
	}


void	A4000DataHeader :: ReadAnamnesis_V1_V2(IniFileReader &ir)
	{
	ir.set_section("ANAMNESIS");
	patient_info.ID = ir.read_string("ID");
	patient_info.surname = ir.read_string("surname");
	patient_info.name = ir.read_string("name");
	patient_info.patronymic = ir.read_string("patronymic");
	patient_info.birth_year = ir.read_int("birth_year", 0);
	patient_info.birth_month = ir.read_int("birth_month", 0);
	patient_info.birth_day = ir.read_int("birth_day", 0);

	patient_info.comment = ir.read_string("comment");

	ir.set_section("EXAMINATION_DATE");

	ir.read_int("year", year);
	ir.read_int("month", month);
	ir.read_int("day", day);
	ir.read_int("hour", hour);
	ir.read_int("minute", minute);
	}

void	A4000DataHeader :: ReadScanParamsV2(IniFileReader &ir)
	{	
	scan_params.upo.LoadProbeOptions(ir);
	
	ir.set_section("SCAN_PARAMS");

	scan_params.mfo.realloc(ir.read_int("n_focuses"));
	scan_params.sfd.n_rays = ir.read_int("n_rays");

//	scan_params.pfd.start_angle = degrees(ir.read_double("start_angle"));//*PI/180;
//	scan_params.pfd.end_angle = degrees(ir.read_double("end_angle"));//*PI/180;

	physical_angle a0 = degrees(ir.read_double("start_angle"));//*PI/180;
	physical_angle a1 = degrees(ir.read_double("end_angle"));//*PI/180;
	
	scan_params.sfd.sample_rate = MHz(ir.read_double("sample_rate_mhz"));
//	scan_params.carrier_frequency = MHz(ir.read_double("carrier_frequency_mhz"));
	scan_params.sfroi.carrier_offset = MHz(ir.read_double("carrier_offset_mhz", 0));
	scan_params.sound_speed = cm_sec(ir.read_double("sound_sped_cm_sec", 1.54e5));

	scan_params.sfroi.n_samples_skipped = ir.read_int("n_samples_skipped");
	scan_params.sfroi.n_samples_to_skip_at_start = ir.read_int("n_samples_to_skip_at_start");
	scan_params.sfroi.n_samples_to_skip_at_end = ir.read_int("n_samples_to_skip_at_end");
	scan_params.sfd.n_samples = ir.read_int("n_samples_total") - (scan_params.sfroi.n_samples_to_skip_at_start + scan_params.sfroi.n_samples_to_skip_at_end);
	
	switch(scan_params.upo.ProbeType())
		{
		case convex_2D_probe:
		case phased_2D_probe:
		case single_element_scanning_2D_probe:
		case annular_scanning_2D_probe:
			{
			//scan_params.pfd.scanning_trajectory_length = (scan_params.upo.ScanningRadius() + scan_params.min_depth()) * scan_params.pfd.angle_range().radians();
			scan_params.pfd.SetFrameSector(
				(scan_params.upo.ScanningRadius() + scan_params.calculate_min_depth()) * (a1-a0).radians(),
				cm(scan_params.sfd.n_samples)*scan_params.sound_speed.cm_sec()/(2.*scan_params.sfd.sample_rate.Hz()),
				a0,
				a1
				);
			break;
			}
		case linear_2D_probe:
			{
			//scan_params.pfd.scanning_trajectory_length = scan_params.upo.ElementSize() * scan_params.sfd.n_rays;
			scan_params.pfd.SetFrameRectangle(
				scan_params.upo.ElementSize() * scan_params.sfd.n_rays,
				cm(scan_params.sfd.n_samples)*scan_params.sound_speed.cm_sec()/(2.*scan_params.sfd.sample_rate.Hz()));
			break;
			}
		default:
			throw logic_error("Formula not written yet");
			break;
		}
	
//	scan_params.pfd.depth_range = cm(scan_params.sfd.n_samples)*scan_params.sound_speed.cm_sec()/(2.*scan_params.sfd.sample_rate.Hz());
	
	for(size_t i = 0; i < scan_params.mfo.n_focuses && i < 4; i++)
		{
		static char	param_name[256];// = AnsiString("focus_") + IntToStr(i);
		sprintf(param_name, "focus_%d", int(i));
		scan_params.mfo[i] = cm(ir.read_double(param_name));
		}
	}




XRAD_END
