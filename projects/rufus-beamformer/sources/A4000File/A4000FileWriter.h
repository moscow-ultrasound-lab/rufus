//---------------------------------------------------------------------------

#ifndef __a4000_file_writer_h
#define __a4000_file_writer_h


#include <DataIO.h>
#include <DataIOEnum.h>

#include <DataArray2D.h>
#include <DataArray.h>
#include <A4000FileHeader.h>
#include <XRADIniFile.h>

XRAD_BEGIN



template<class M>
class	A4000DataWriter : public A4000DataHeader
	{
	FILE	*dat_file;
public:
	A4000DataWriter(const ScanParams &sp, int nfr)
		{
		scan_params = sp;
		n_frames_per_file = nfr;
		dat_file = NULL;
		data_signature = "A400";
		data_signature_size = strlen(data_signature);
		ray_header_size = 0;
		}

	void	SetDataFormat(ioNumberOptions df){data_format = df;};
	void	SetPatientInfo(const PatientInfo &pi){patient_info = pi;};

	void	SetDataFormat(ioNumberOptions df){data_format = df;};
	bool	SetDestination(const char* fn);
	bool	WriteHeaderFile();
	void	WriteFrame(const M &m);

	bool	BeginWriting();
	void	EndWriting();
	};



XRAD_END


#define __a4000_file_writer_cc
#include "A4000FileWriter.cc"
#undef __a4000_file_writer_cc

//---------------------------------------------------------------------------
#endif   // __a4000_file_writer_h
