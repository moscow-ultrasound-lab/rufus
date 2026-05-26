#ifndef __a4000_file_writer_cc
#error "This file should be included only through A4000FileWriter.h"
#endif

#include <DataIO.h>

XRAD_BEGIN



template<class M>
bool	A4000DataWriter<M> :: SetDestination(const char *filename)
	{
	static	char	file_name_buffer[1024];
	int	n = 0;
	FILE	*temp_file = NULL;

	do
		{
		++n;
		if(temp_file) fclose(temp_file);
		sprintf(file_name_buffer, "%s_%.3d.hdr", filename, n);
		temp_file = fopen(file_name_buffer, "rb");
		}while(temp_file && n < 1000);
	if(n >= 1000) return false;


	sprintf(file_name_buffer, "%s_%.3d", filename, n);
	hdr_file_name = string(file_name_buffer) + ".hdr";
	dat_file_name = string(file_name_buffer) + ".dat";

	return true;
	}


#error ("формат файла сильно менЯетсЯ, всю функцию ниже переписать по ридеру")

template<class M>
bool	A4000DataWriter<M> :: WriteHeaderFile()
	{
	QMIniWriter	iw;
	if(!iw.Open(hdr_file_name.c_str())) return false;

	iw.WriteSection("DATA_FILE_INFO");
	iw.WriteString("data_file_name", dat_file_name.c_str());
	iw.WriteString("data_format", /*iot_to_str(data_format)*/"PC_SIGNED_INT_16");
//	iw.WriteString("data_signature", "A400");
	iw.WriteString("data_signature", data_signature.c_str());
	iw.WriteInt("n_frames_per_file", n_frames_per_file);
	iw.WriteInt("ray_header_size", ray_header_size);

	iw.WriteSection("SCAN_PARAMS");
	iw.WriteInt("probe_id", scan_params.probeID);
	iw.WriteInt("n_focuses", scan_params.n_focuses);
	iw.WriteInt("n_rays", scan_params.n_rays);

	iw.WriteInt("n_samples", scan_params.n_samples);
	iw.WriteInt("start_sample", scan_params.start_sample);
	iw.WriteInt("end_sample", scan_params.end_sample);

	iw.WriteDouble("scanning_trajectory_length", scan_params.array_length);
	iw.WriteDouble("start_angle", scan_params.start_angle*180/PI);
	iw.WriteDouble("end_angle", scan_params.end_angle*180/PI);

	iw.WriteDouble("sample_rate_mhz", scan_params.sample_rate.MHz());
	iw.WriteDouble("carrier_frequency_mhz", scan_params.carrier_frequency.MHz());
	iw.WriteDouble("carrier_offset_mhz", scan_params.carrier_offset.MHz());
	iw.WriteDouble("sound_speed_cm_sec", scan_params.sound_speed.cm_sec());

	for(int i = 0; i < scan_params.n_focuses && i < 4; i++)
		{
		AnsiString	param_name = AnsiString("focus_") + IntToStr(i);
		iw.WriteDouble(param_name.c_str(), scan_params.focus_depths[i]);
		}

	iw.WriteSection("ANAMNESIS");
	iw.WriteString("ID", patient_info.ID.c_str());
	iw.WriteString("surname", patient_info.surname.c_str());
	iw.WriteString("name", patient_info.name.c_str());
	iw.WriteString("patronymic", patient_info.patronymic.c_str());
	iw.WriteInt("birth_year", patient_info.birth_year);
	iw.WriteInt("birth_month", patient_info.birth_month);
	iw.WriteInt("birth_day", patient_info.birth_day);

	iw.WriteString("comment", patient_info.comment.c_str());

	iw.WriteSection("EXAMINATION_DATE");

	iw.WriteInt("year", year);
	iw.WriteInt("month", month);
	iw.WriteInt("day", day);
	iw.WriteInt("hour", hour);
	iw.WriteInt("minute", minute);


	iw.Close();
	return true;
	}


template<class M>
bool	A4000DataWriter<M> :: BeginWriting()
	{
	dat_file = fopen(dat_file_name.c_str(), "wb");
	if(!dat_file)
		{
		return false;
		}
//	fwrite("A400", 4, 1, dat_file);
	fwrite(data_signature.c_str(), data_signature_size, 1, dat_file);
	return true;
	}


template<class M>
void	A4000DataWriter<M> :: WriteFrame(const M &m)
	{
	// here check if m.vsize m.hsize == n_rays, n_samples
	DataArray<char>	ray_header(ray_header_size, 0);
	for(int i = 0; i < m.vsize(); i++)
		{
		if(ray_header_size)
			{
			fwrite_numbers<char>(ray_header, dat_file, ioI8);
			}
		fwrite_numbers<sample_type>(m[i], dat_file, data_format);
//		fwrite_numbers(m[i], dat_file, data_format);
		}
	}

template<class M>
void	A4000DataWriter<M> :: EndWriting()
	{
	fclose(dat_file);
	dat_file = NULL;
	}






XRAD_END
