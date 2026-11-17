#include "pre.h"
#include "SimIOHeaderProcessor.h"

XRAD_USING



char	this_process_comment[MAXSTRINGSIZE];

#define	SF_PROCESS_VERSION 6.0

void	SimIOHeaderProcessor::WriteGenericDataDescriptor(FILE *file_ptr_out)
{
	int	size_of_Generic_Data_Descriptor;

	SIMIO::Start_Sample = 0;
//	SIMIO::Image_Depth = (r_max - Options :: r_min).cm();
	SIMIO::Image_Depth = depth_range().cm();
	SIMIO::Sample_Rate = sample_rate.Hz();

	strcpy(SIMIO::Descriptor_Identifier, GENERIC_DATA_DESCRIPTOR);
	strcpy(SIMIO::Processor_Type, THIS_PROCESSOR_TYPE);	// <- generic.h

	SIMIO::Num_Xtra_Descriptors = 0;
	size_of_Generic_Data_Descriptor = GENERIC_DATA_DESCRIPTOR_BASE_NUM_PARAMS;
	SimPutDescriptor(size_of_Generic_Data_Descriptor, SIMIO::Generic_Data_Descriptor, file_ptr_out);
}

void	SimIOHeaderProcessor::WriteSubapertDataDescriptor(FILE *file_ptr_out)
{
	strcpy(SIMIO::Data_Type, "Complex");
	SIMIO::Data_Count = 0;
	SIMIO::Datum_Size = sizeof(complexF32);
	strcpy(SIMIO::Descriptor_Identifier, SUBAPERT_DATA_DESCRIPTOR);
	strcpy(SIMIO::Processor_Type, THIS_PROCESSOR_TYPE);	// <- generic.h

	SimPutDescriptor(NUM_PARAMS_IN_SUBAPERT_DATA_DESCRIPTOR,
					 SIMIO_sub::Subapert_Data_Descriptor, file_ptr_out);

}


void	SimIOHeaderProcessor::AppendHistoryRecord(FILE *file_ptr_out,
												  const char *input_file_name,
												  const char *output_file_name)
{

	time_t	present_time;

	SIMIO::Start_Sample = 0;
//	SIMIO::Image_Depth = (r_max - Options :: r_min).cm();
	SIMIO::Image_Depth = depth_range().cm();
	SIMIO::Sample_Rate = sample_rate.Hz();

	if(strlen(this_process_comment) == 0){// SimIO doesn't like zero length strings !
		strcpy(this_process_comment, SIMIO::NO_COMMENT().c_str());
	}

//	append a new history record:
	SIMIO::Process_Version = SF_PROCESS_VERSION;

//	if the file name includes a path, avoid overwriting STDSTRINGSIZE
//	buffers which are defined in Std_Headers

	strcpy(SIMIO::Process_In_File_Name, input_file_name);
	strcpy(SIMIO::Process_In_File_Date, SIMIO::Process_Date_Time);	// NB : input file
	strcpy(SIMIO::Process_In_File_Comment, SIMIO::Process_Comment);
	strcpy(SIMIO::Process_Out_File_Name, output_file_name);
	present_time = time(NULL);
	strcpy(SIMIO::Process_Date_Time, ctime(&present_time));	// NB : output file
	SIMIO::Process_Date_Time[strlen(SIMIO::Process_Date_Time) - 1] = '\0';// remove \n
	strcpy(SIMIO::Process_Comment, this_process_comment);

	if(strcmp(SIMIO::Data_Genesis, EXPT_GENESIS) == 0)
		strcpy(SIMIO::Data_Genesis, SYNTH_OF_EXPT_GENESIS);
	else strcpy(SIMIO::Data_Genesis, SYNTH_OF_SIM_GENESIS);




	strcpy(SIMIO::Image_Depth_Comment, MILLI_METRES);

	strcpy(SIMIO::Process_Description, "Synthetically focused complex subapertures signal");
	strcpy(SIMIO::Data_Species, SUBAPERT_DATA_SPECIES);
	SimPutHistory(STD_HISTORY_DESCRIPTOR_NUM_PARAMS,
				  SIMIO::Standard_History,
				  file_ptr_out);
}


