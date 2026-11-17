#include "pre.h"
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

	FILE *par_file = fopen(par_filename.c_str(), "rb");
	if (!par_file) return;

	fscanf(par_file,"NumOfFrames=%d\n", &NumOfFrames);
	fscanf(par_file,"RawFrameSize=%d\n", &RawFrameSize);
	fscanf(par_file,"HeaderSize=%d\n", &HeaderSize);
	fscanf(par_file,"NumOfBBeams=%d\n", &NumOfBBeams);
	fscanf(par_file,"SizeofBBeamAtSamples=%d\n", &SizeofBBeamAtSamples);
	fscanf(par_file,"NumOfCFShots=%d\n", &NumOfCFShots);
	fscanf(par_file,"NumOfSweeps=%d\n", &NumOfSweeps);
	fscanf(par_file,"BeamsInSweep=%d\n", &BeamsInSweep);
	fscanf(par_file,"SizeofCFMBeamAtSamples=%d\n", &SizeofCFMBeamAtSamples);
	fscanf(par_file,"FirstScanCFMBeam=%d\n", &FirstScanCFMBeam);
	fscanf(par_file,"CFMDensity=%d\n", &CFMDensity);
	fscanf(par_file,"NumOfCFMBeams=%d\n", &NumOfCFMBeams);
	fscanf(par_file,"NumOfFirstCFMSample=%d\n", &NumOfFirstCFMSample);
	fscanf(par_file,"CFMFilterOrder=%d\n", &CFMFilterOrder);
	fclose(par_file);
	}



XRAD_END


