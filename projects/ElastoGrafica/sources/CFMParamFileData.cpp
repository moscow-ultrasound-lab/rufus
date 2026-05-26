#include "pre.h"
#include <CFileContainer.h>
#include "CFMParamFileData.h"


XRAD_BEGIN

void	CFMParamFileData::Init(const string &filename)
	{
	char	par_fn_buffer[512];
	char	dat_fn_buffer[512];

	strcpy(dat_fn_buffer, filename.c_str());
	strcpy(par_fn_buffer, filename.c_str());

	int	n = strlen(dat_fn_buffer)-4;
	
	strcpy(dat_fn_buffer+n, ".dat");
	strcpy(par_fn_buffer+n, ".par");

	dat_filename = string(dat_fn_buffer);
	par_filename = string(par_fn_buffer);

	CFileContainer par_file;
	par_file.open(par_filename.c_str(), "rb");

	fscanf(par_file.c_file(),"NumOfFrames=%d\n", &NumOfFrames);
	fscanf(par_file.c_file(),"RawFrameSize=%d\n", &RawFrameSize);
	fscanf(par_file.c_file(),"HeaderSize=%d\n", &HeaderSize);
	fscanf(par_file.c_file(),"NumOfBBeams=%d\n", &NumOfBBeams);
	fscanf(par_file.c_file(),"SizeofBBeamAtSamples=%d\n", &SizeofBBeamAtSamples);
	fscanf(par_file.c_file(),"NumOfCFShots=%d\n", &NumOfCFShots);
	fscanf(par_file.c_file(),"NumOfSweeps=%d\n", &NumOfSweeps);
	fscanf(par_file.c_file(),"BeamsInSweep=%d\n", &BeamsInSweep);
	fscanf(par_file.c_file(),"SizeofCFMBeamAtSamples=%d\n", &SizeofCFMBeamAtSamples);
	fscanf(par_file.c_file(),"FirstScanCFMBeam=%d\n", &FirstScanCFMBeam);
	fscanf(par_file.c_file(),"CFMDensity=%d\n", &CFMDensity);
	fscanf(par_file.c_file(),"NumOfCFMBeams=%d\n", &NumOfCFMBeams);
	fscanf(par_file.c_file(),"NumOfFirstCFMSample=%d\n", &NumOfFirstCFMSample);
	fscanf(par_file.c_file(),"CFMFilterOrder=%d\n", &CFMFilterOrder);	
	}



XRAD_END


