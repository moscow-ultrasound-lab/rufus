#include "pre.h"

#ifndef __MWERKS__
#pragma hdrstop
#endif

#include <XRADSystem/Sources/IniFile/XRADIniFile.h>
#include "A4000FileReader.h"

XRAD_BEGIN

using namespace DataArrayIOAuxiliaries;

void	A4000DataReader :: OpenDataFiles()
	{
	dat_file_name.realloc(n_data_files);
	dat_file.realloc(n_data_files);
	
	for(size_t i = 0; i < n_data_files; ++i)
		{
		dat_file_name[i] = ssprintf(dat_file_pattern.c_str(), i);
		}
	
	for(size_t i = 0; i < n_data_files; ++i)
		{
		OpenDataFile(dat_file[i], dat_file_name[i]);
		}	
	}

void	A4000DataReader :: OpenDataFile(shared_cfile &datafile, const string& filename)
	{
	try
		{
		datafile.open(filename.c_str(), "rb");
		}

	
	catch(file_container_error &)
		{
		//	попытка найти файл в папке по умолчанию,
		//	если его нет по указанному пути
		size_t	fn_length = filename.length();
		size_t	dat_path_length = fn_length-1;
		while(dat_path_length>0 && filename[dat_path_length] != '\\'){--dat_path_length;}
		if(!dat_path_length)
			{
			throw runtime_error(ssprintf("A4000DataReader :: OpenDataFile, could not open file '%s'", filename.c_str()));
			}
		size_t dat_filename_size = fn_length-dat_path_length;
		string name_without_path(dat_filename_size, '\0');
		for(size_t i = 0; i < dat_filename_size - 1; i++)
			{
			name_without_path[i] = filename[i+dat_path_length+1];
			}		
		try
			{
			datafile.open(name_without_path.c_str(), "rb");
			}

		catch(file_container_error&)
			{
			//	попытка найти файл данных в той же папке, что
			//	и файл заголовка
			size_t	hdr_fn_length = hdr_file_name.length();
			size_t	hdr_path_size = hdr_fn_length-1;
			while(hdr_path_size > 0 && hdr_file_name[hdr_path_size] != '\\'){--hdr_path_size;}
			if(hdr_file_name[hdr_path_size] == '\\') ++hdr_path_size;
			string name_in_header_folder(hdr_path_size + dat_filename_size, '\0');
			for(size_t i = 0; i < hdr_path_size; i++)
				{
				name_in_header_folder[i] = hdr_file_name[i];
				}
			for(size_t i = 0; i <  dat_filename_size - 1; i++)
				{
				name_in_header_folder[hdr_path_size + i] = name_without_path[i];
				}
			datafile.open(name_in_header_folder.c_str(), "rb");
			}
		}	

	if(data_signature_size)
		{
		string	check_signature(data_signature_size+1, '\0');
//		const char *etalon_signature = data_signature.c_str();
		datafile.read(&check_signature[0], 1, data_signature_size);

		
		if(strcmp(check_signature.c_str(), data_signature.c_str()))
//		if(!(check_signature == data_signature))
			{
			throw logic_error(ssprintf("Invalid signature in data file: '%s' (must be '%s')",
				check_signature.c_str(),
				data_signature.c_str()));
			}

		}

	file_size_t proper_file_size = data_signature_size +
		scan_params.sfd.n_rays * scan_params.mfo.n_focuses*n_frames_per_file *
			(ray_header_size + scan_params.n_samples_total() * io_sample_size(data_format));
			//2;
	
	if(datafile.size() != proper_file_size)
		{
		throw logic_error(ssprintf("Invalid data file size: %d bytes (must be %d bytes)",
			datafile.size(),
			proper_file_size));
		}
	}



size_t A4000DataReader :: RayDataOffset(size_t frame_no, size_t ray_no) const
	{
	size_t frame_size_in_bytes = scan_params.sfd.n_rays * (ray_header_size + scan_params.n_samples_total()*io_sample_size(data_format));
	size_t ray_size_in_bytes = frame_size_in_bytes/scan_params.sfd.n_rays;
	size_t frame_in_file_no = frame_no % n_frames_per_file;
	
	return	data_signature_size +
		(frame_in_file_no/scan_params.mfo.n_focuses)*(frame_size_in_bytes*scan_params.mfo.n_focuses) + 
		
		(frame_in_file_no%scan_params.mfo.n_focuses)*ray_size_in_bytes +
		ray_no*ray_size_in_bytes*scan_params.mfo.n_focuses +
		ray_header_size;
	}




XRAD_END


