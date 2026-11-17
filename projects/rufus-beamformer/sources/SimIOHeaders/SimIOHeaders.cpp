#include "pre.h"
//-----------------------------------
//
//	этот файл представлЯет собой еще одно
//	неудобное наследство от атл. глобальные переменные
//	заданы ими в ашнике Std_Headers.h, а extern
//	включаетсЯ как опциЯ условной компилЯции SF_XTRN.
//	очень неуклюже, но переделывать их писаниЯ не
//	представлЯетсЯ возможным. сейчас выношу




#include <stdio.h>
#include "SimIO.h"
#include "SimIOHeaders/SimIOHeaders.h"
//#include "SubapertDataDescriptor.h"

void dummy_function_QM_Globals()
{
}

#define INCLUDE_SF_DATA_SPECIES
#define	DEFINING_STD_FILE_HEADERS_0	// force objects in

//#define INCLUDE_EL_DATA_SPECIES //?

/*
definitions and declarations that are common to two or more keyword phrases
note : use NO_COMMENT rather than "" or " ", since the SimIO does not like
to read "" or " ".
*/
//#define MAX_NUM_DESCRIPTORS 16
//#define NO_COMMENT			"no comment"
//#define	SIMIO::NULL_COMMENT()		(char *)NULL
//#define NA					"Not Applicable"
//#define	SHORT_INT			"short (2-byte) integer"
//#define BASE_0				"(base 0)"
//#define HZ					"Hz"
//#define	NS					"nano_seconds"
//#define MILLI_METRES		"milli-metres"
//#define MICRO_METRES		"micro-metres"
//#define	STD_FOCAL_COMMENT	"milli-metres (-1 is infinity, 0 is dynamic)"
//#define	FOCUSSED_AT_INFINITY	-1.0
//#define	DYNAMIC_FOCUS			0.0
//#define	STD_SPEED_OF_SOUND_COMMENT	"milli-metres/uSec"
//#define	STD_PERCENT_BW_COMMENT		"% (-6dB)"
//#define STD_RMS_COMMENT		"rms"
//#define	DEGREES				"degrees"
// //#define STDSTRINGSIZE		64
// //#define STDCOMMENTSIZE		64
//#define STDSTRINGSIZE		512
//#define STDCOMMENTSIZE		512


char SIMIO::Descriptor_Identifier[STDSTRINGSIZE];
char SIMIO::Xtra_Desc_Identifier[MAX_NUM_DESCRIPTORS][STDSTRINGSIZE];
//#define GENERIC_DATA_DESCRIPTOR		"Generic Data Descriptor"
//#define GENERAL_DESCRIPTOR	"General Descriptor (species specific, frame inspecific)"
//#define FRAME_DESCRIPTOR			"Frame Descriptor (species specific)"
//#define TX_EVENT_DESCRIPTOR 		"Transmit Event Descriptor (species specific)"
//#define	AB_PARAMS_DESCRIPTOR		"Aberration Parameters Descriptor"
//#define	AB_DATA_DESCRIPTOR			"Aberration Data Descriptor"
//#define	SF_IMAGE_PARAMS_DESCRIPTOR	"SF Image Parameters Descriptor"
//#define	SF_IMAGE_ROW_DATA_DESCRIPTOR	"SF Image Row Data Descriptor"
//#define SEER_GENERAL_DESCRIPTOR		"SEER General Parameters Descriptor"
//#define SEER_REFERENCE_DESCRIPTOR	"SEER Reference Waveform Descriptor"
//#define	SEER_RF_DATA_DESCRIPTOR		"SEER RF Data Descriptor (source coords specific)"
//#define SEER_DETECTED_DATA_DESCRIPTOR	"SEER detected Data Descriptor (range specific)"

char SIMIO::Process_Name[STDSTRINGSIZE];
//#define RF_ACQ				"rf_acq"
//#define SF_SYNTHESIZE		"SF_synthesize"
//#define	SYNTH_IMAGE			"synth_image"
//#define	SYNTH_A_LINE		"synth_A_line"
//#define SFFT_SYNTH			"SFFT_synth"
//#define SF_SIMULATE			"SF_simulate"
//#define	EL_SIMULATE			"EL_simulate"
//#define	HEADERS_ON_OFF		"headers_on_off"
//#define SEER				"SEER"
//#define SEER_skew			"SEER_skew"


char SIMIO::Data_Species[STDSTRINGSIZE];
//#define SF_DATA_SPECIES		"single TX, element by element RX, RF data"
//#define EL_DATA_SPECIES		"TX beamformed, element by element RX, RF data"
//#define SUMMED_DATA_SPECIES "summed RF data"
//#define SF_IMAGE_SPECIES	"synthetic focus image raster data"
//#define	AB_DATA_SPECIES		"aberration data"
//#define	SEER_DATA_SPECIES	"Simulated Elevational Element Response data"

char SIMIO::Data_Genesis[STDSTRINGSIZE];
//#define EXPT_GENESIS		"experimental"
//#define SIM_GENESIS			"simulated"
//#define SYNTH_OF_EXPT_GENESIS	"synthetic focussing of experimental"
//#define SYNTH_OF_SIM_GENESIS	"synthetic focussing of simulated"
//#define	ANAL_OF_SEER_GENESIS	"analysis of SEER output data"
//#define	UNKNOWN_GENESIS			"too many processing steps to track genesis"


char SIMIO::ECG_Trigger[STDSTRINGSIZE];
//#define ECG_TRIGGER_ON		"On"
//#define ECG_TRIGGER_OFF		"Off"
//#define ECG_TRIGGER_NA		NA

char SIMIO::Acquisition_Method[STDSTRINGSIZE];
//#define SRAM				"SRAM"
//#define FIFO				"FIFO"
//#define	SYNTHESIZED			"synthesized"
//#define	SIMULATED			"simulated"

char SIMIO::Scanhead_Type[STDSTRINGSIZE];
//#define ATL_PA				"ATL UM-9 phased array"
//#define ECHO_PA				"Echo UM-9 Ultrasound phased array"
//#define AA					"ATL standard annular array"
//#define WAAA				"ATL wide aperture annular array"

char SIMIO::Data_Complexity[STDSTRINGSIZE];
//#define	REAL_COMPONENT		"real component only"
//#define IMAGINARY_COMPONENT	"imaginary component only"
//#define	ANALYTIC			"analytic components"


char SIMIO::Num_Xtra_Descriptors_Comment[STDCOMMENTSIZE];
//#define STD_NUM_XTRA_DESC_COMMENT	"# descriptor records for this species & version"

char SIMIO::Element_Pitch_Comment[STDCOMMENTSIZE];
//#define	ATL_3PA_PITCH		0.2794
char SIMIO::			Image_Depth_Comment[STDCOMMENTSIZE];

char SIMIO::Acqs_Per_Cal_Comment[STDCOMMENTSIZE];
//#define STD_ACQS_PER_CAL_COMMENT	"<--- 0 means no calibration pulses"

char SIMIO::TX_Events_Per_Frame_Comment[STDCOMMENTSIZE];	
//#define CAL_and_ACQ			"(calibration & acquisition)"
//#define	RAYS				"rays"

char SIMIO::Data_Count_Comment[STDCOMMENTSIZE];
//#define ITEMS	"items"

char SIMIO::Datum_Size_Comment[STDCOMMENTSIZE];
//#define BYTES	"bytes"

char SIMIO::Start_Sample_Comment[STDCOMMENTSIZE];
//#define STD_START_SAMPLE_COMMENT	"(relative to a TX from the array origin)"

char SIMIO::Fractional_Start_Sample_Comment[STDCOMMENTSIZE];
//#define STD_FRACTIONAL_START_SAMPLE_COMMENT	"(from the x, y, z origin)"


char SIMIO::Process_Description[MAXSTRINGSIZE];
char SIMIO::Process_Date_Time[STDSTRINGSIZE];
char SIMIO::Processor_Type[STDSTRINGSIZE];
char SIMIO::Process_In_File_Name[STDSTRINGSIZE];
char SIMIO::Process_In_File_Date[STDSTRINGSIZE];
char SIMIO::Process_In_File_Comment[MAXSTRINGSIZE];
char SIMIO::Process_Out_File_Name[STDSTRINGSIZE];
char SIMIO::Process_Comment[MAXSTRINGSIZE];
char SIMIO::Data_Type[STDSTRINGSIZE];
char SIMIO::How_Frames_Vary_Comment[MAXSTRINGSIZE];
char SIMIO::Original_RF_Data_File_Name[STDSTRINGSIZE];
char SIMIO::Original_RF_Data_File_Date[STDSTRINGSIZE];
char SIMIO::Original_RF_Data_Comment[MAXSTRINGSIZE];
char SIMIO::Correction_File_Name[STDSTRINGSIZE];
char SIMIO::Correction_File_Date[STDSTRINGSIZE];
char SIMIO::Correction_File_Comment[MAXSTRINGSIZE];

char SIMIO::Frame_Number_Comment[STDCOMMENTSIZE];
char SIMIO::Scanhead_Freq_Comment[STDCOMMENTSIZE];
char SIMIO::Calibration_Element_Comment[STDCOMMENTSIZE];
char SIMIO::Sample_Rate_Comment[STDCOMMENTSIZE];
char SIMIO::TX_Event_Number_Comment[STDCOMMENTSIZE];
char SIMIO::TX_Element_Number_Comment[STDCOMMENTSIZE];
char SIMIO::Start_Angle_Comment[STDCOMMENTSIZE];
char SIMIO::Delta_Angle_Comment[STDCOMMENTSIZE];
char SIMIO::Start_Lateral_Disp_Comment[STDCOMMENTSIZE];
char SIMIO::Delta_Lateral_Disp_Comment[STDCOMMENTSIZE];
char SIMIO::TX_Focal_Point_Comment[STDCOMMENTSIZE];
char SIMIO::RX_Focal_Point_Comment[STDCOMMENTSIZE];
char SIMIO::TX_Delay_Quant_Comment[STDCOMMENTSIZE];
char SIMIO::RX_Delay_Quant_Comment[STDCOMMENTSIZE];
char SIMIO::Ray_Angle_Comment[STDCOMMENTSIZE];
char SIMIO::Ray_Lateral_Disp_Comment[STDCOMMENTSIZE];

double SIMIO::Process_Version;
double SIMIO::Scanhead_Freq;
double SIMIO::Element_Pitch;
double SIMIO::Image_Depth;
double SIMIO::Sample_Rate;
double SIMIO::Start_Angle;
double SIMIO::Delta_Angle;
double SIMIO::Start_Lateral_Displacement;
double SIMIO::Delta_Lateral_Displacement;
double SIMIO::TX_Focal_Point;
double SIMIO::RX_Focal_Point;
double SIMIO::TX_Delay_Quantisation;
double SIMIO::RX_Delay_Quantisation;
double SIMIO::Ray_Angle;
double SIMIO::Ray_Lateral_Displacement;
double SIMIO::Fractional_Start_Sample;


long SIMIO::Zero_L = 0;
long SIMIO::N_Frames;		/* make these long to avoid later grief 	*/
long SIMIO::Frame_Number;	/* when using these as variables in			*/
long SIMIO::N_Scanhead_Els;	/* expressions to set sizes					*/
long SIMIO::Acqs_Per_Cal;
long SIMIO::Calibration_Element;
long SIMIO::TX_Events_Per_Frame;
long SIMIO::Start_Sample;
long SIMIO::Samps_Per_Waveform;
long SIMIO::Waveforms_Per_TX_Event;
long SIMIO::TX_Event_Number;
long SIMIO::TX_Element_Number;
long SIMIO::Data_Count;
long SIMIO::Datum_Size;
long SIMIO::Interpolation_Factor;

int SIMIO::Num_Xtra_Desc_Params[MAX_NUM_DESCRIPTORS];
int SIMIO::Descriptor_Version;







/*----------------------------------------------------------------------------
This $$HIST record is the standard form of the history record that is appended
to previous history record and immediately preceeds the $$DESC standard
"Data Descriptor" record.
*/ 

std::vector<param> SIMIO::Standard_History =	
	{
	{"ProcessName", Process_Name, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"ProcessDescription", Process_Description, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"ProcessVersion", &Process_Version, SIMIO::NULL_COMMENT(), ADOUBLE, 0},
	{"ProcessInFileName", Process_In_File_Name, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"ProcessInFileDate", Process_In_File_Date, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"ProcessInFileComment", Process_In_File_Comment, SIMIO::NULL_COMMENT(), 
																ASTRING, 0},
	{"ProcessOutFileName", Process_Out_File_Name, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"ProcessDateTime", Process_Date_Time, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"ProcessComment", Process_Comment, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"DataSpecies", Data_Species, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"DataGenesis", Data_Genesis, SIMIO::NULL_COMMENT(), ASTRING, 0}
	};

//#define	STD_HISTORY_DESCRIPTOR_NUM_PARAMS	(11)


/*----------------------------------------------------------------------------
This $$DESC record is the "Generic Data Descriptor" record.

It immediately follows the $$HIST records.

Note that this repeats much of the information in the most recent standard 
history record, but this is intentional for the reasons described above, and
by repeating various STD_HISTORY keyword phrases, this std will tend to be
enforced by default.
*/ 
//#define GENERIC_DATA_DESCRIPTOR_BASE_NUM_PARAMS	20

long SIMIO::Num_Xtra_Descriptors;

#if 0
char SIMIO::string_void[STDSTRINGSIZE];
#else
char *SIMIO::string_void = "void";
#endif

std::vector<param> SIMIO::Generic_Data_Descriptor		/* GENERIC_DATA_DESCRIPTOR	*/
 =	{
	{"DescriptorIdentifier", Descriptor_Identifier, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"DataType", string_void, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"DataCount", &Zero_L, SIMIO::NULL_COMMENT(), ALONG, 0},
	{"ProcessorType", Processor_Type, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"OriginalRfDataFileName", Original_RF_Data_File_Name, SIMIO::NULL_COMMENT(), 
																ASTRING, 0},
	{"OriginalRfDataFileDate", Original_RF_Data_File_Date, SIMIO::NULL_COMMENT(), 
																ASTRING, 0},
	{"OriginalRfDataComment", Original_RF_Data_Comment, SIMIO::NULL_COMMENT(), 
																ASTRING, 0},
	{"ProcessInFileName", Process_In_File_Name, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"ProcessInFileDate", Process_In_File_Date, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"ProcessInFileComment", Process_In_File_Comment, SIMIO::NULL_COMMENT(), 
																ASTRING, 0},
	{"ProcessName", Process_Name, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"ProcessDescription", Process_Description, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"ProcessVersion", &Process_Version, SIMIO::NULL_COMMENT(), ADOUBLE, 0},
	{"ProcessOutFileName", Process_Out_File_Name, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"ProcessDateTime", Process_Date_Time, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"CurrentDataComment", Process_Comment, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"DataSpecies", Data_Species, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"DataGenesis", Data_Genesis, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"DescriptorVersion", &Descriptor_Version, SIMIO::NULL_COMMENT(), AINT, 0},
	{"NumXtraDescriptors", &Num_Xtra_Descriptors, 
				Num_Xtra_Descriptors_Comment, ALONG, 0},

	{"XtraDescIdentifier0", &Xtra_Desc_Identifier[0][0], SIMIO::NULL_COMMENT(), 
																ASTRING, 0},
	{"NumXtraDescParams0", &Num_Xtra_Desc_Params[0], SIMIO::NULL_COMMENT(), AINT, 0},
	{"XtraDescIdentifier1", &Xtra_Desc_Identifier[1][0], SIMIO::NULL_COMMENT(), 
																ASTRING, 0},
	{"NumXtraDescParams1", &Num_Xtra_Desc_Params[1], SIMIO::NULL_COMMENT(), AINT, 0},
	{"XtraDescIdentifier2", &Xtra_Desc_Identifier[2][0], SIMIO::NULL_COMMENT(), 
																ASTRING, 0},
	{"NumXtraDescParams2", &Num_Xtra_Desc_Params[2], SIMIO::NULL_COMMENT(), AINT, 0},
	{"XtraDescIdentifier3", &Xtra_Desc_Identifier[3][0], SIMIO::NULL_COMMENT(), 
																ASTRING, 0},
	{"NumXtraDescParams3", &Num_Xtra_Desc_Params[3], SIMIO::NULL_COMMENT(), AINT, 0},
	{"XtraDescIdentifier4", &Xtra_Desc_Identifier[4][0], SIMIO::NULL_COMMENT(), 
																ASTRING, 0},
	{"NumXtraDescParams4", &Num_Xtra_Desc_Params[4], SIMIO::NULL_COMMENT(), AINT, 0},
	{"XtraDescIdentifier5", &Xtra_Desc_Identifier[5][0], SIMIO::NULL_COMMENT(), 
																ASTRING, 0},
	{"NumXtraDescParams5", &Num_Xtra_Desc_Params[5], SIMIO::NULL_COMMENT(), AINT, 0},
	{"XtraDescIdentifier6", &Xtra_Desc_Identifier[6][0], SIMIO::NULL_COMMENT(), 
																ASTRING, 0},
	{"NumXtraDescParams6", &Num_Xtra_Desc_Params[6], SIMIO::NULL_COMMENT(), AINT, 0},
	{"XtraDescIdentifier7", &Xtra_Desc_Identifier[7][0], SIMIO::NULL_COMMENT(), 
																ASTRING, 0},
	{"NumXtraDescParams7", &Num_Xtra_Desc_Params[7], SIMIO::NULL_COMMENT(), AINT, 0},
	{"XtraDescIdentifier8", &Xtra_Desc_Identifier[8][0], SIMIO::NULL_COMMENT(), 
																ASTRING, 0},
	{"NumXtraDescParams8", &Num_Xtra_Desc_Params[8], SIMIO::NULL_COMMENT(), AINT, 0},
	{"XtraDescIdentifier9", &Xtra_Desc_Identifier[9][0], SIMIO::NULL_COMMENT(), 
																ASTRING, 0},
	{"NumXtraDescParams9", &Num_Xtra_Desc_Params[9], SIMIO::NULL_COMMENT(), AINT, 0},
	{"XtraDescIdentifier10", &Xtra_Desc_Identifier[10][0], SIMIO::NULL_COMMENT(), 
																ASTRING, 0},
	{"NumXtraDescParams10", &Num_Xtra_Desc_Params[10], SIMIO::NULL_COMMENT(), AINT, 0},
	{"XtraDescIdentifier11", &Xtra_Desc_Identifier[11][0], SIMIO::NULL_COMMENT(), 
																ASTRING, 0},
	{"NumXtraDescParams11", &Num_Xtra_Desc_Params[11], SIMIO::NULL_COMMENT(), AINT, 0},
	{"XtraDescIdentifier12", &Xtra_Desc_Identifier[12][0], SIMIO::NULL_COMMENT(), 
																ASTRING, 0},
	{"NumXtraDescParams12", &Num_Xtra_Desc_Params[12], SIMIO::NULL_COMMENT(), AINT, 0},
	{"XtraDescIdentifier13", &Xtra_Desc_Identifier[13][0], SIMIO::NULL_COMMENT(), 
																ASTRING, 0},
	{"NumXtraDescParams13", &Num_Xtra_Desc_Params[13], SIMIO::NULL_COMMENT(), AINT, 0},
	{"XtraDescIdentifier14", &Xtra_Desc_Identifier[14][0], SIMIO::NULL_COMMENT(), 
																ASTRING, 0},
	{"NumXtraDescParams14", &Num_Xtra_Desc_Params[14], SIMIO::NULL_COMMENT(), AINT, 0},
	{"XtraDescIdentifier15", &Xtra_Desc_Identifier[15][0], SIMIO::NULL_COMMENT(), 
																ASTRING, 0},
	{"NumXtraDescParams15", &Num_Xtra_Desc_Params[15], SIMIO::NULL_COMMENT(), AINT, 0}
	};



#if defined( INCLUDE_SF_DATA_SPECIES )
/***************************** SF_DATA_SPECIES *******************************

The SF_DATA_SPECIES is used to store records of element by element RX data 
where each record arises from a TX from a single element.

The following $$DESC records are defined :
SF_Data_General_Descriptor		frame inspecific parameters
SF_Data_Frame_Descriptor		(possibly) frame specific parameters
SF_Data_TX_Event_Descriptor		one of these to introduce each $$DATA record
*/



/*----------------------------------------------------------------------------
This $$DESC record is the "General Descriptor" record for the SF_DATA_SPECIES
data (ie: single TX, element by element data").	
It immediately follows the Data Descriptor record, and there is only one since 
the data is not frame specific.

Descriptor version = 0	18-Feb-91	Original
*/

enum	{/* this is the order that the xtra	*/
	SF_Data_General_Descriptor_Num,	/* descriptor records names/sizes are 	*/
	SF_Data_Frame_Descriptor_Num,	/* listed in the GENERIC_DATA_DESCRIPTOR*/
	SF_Data_TX_Event_Descriptor_Num
	};
	
long SIMIO::SF_Data_Num_Descriptors[]		/* indexed by version #		*/
=	{
	3
	};

/* indexed : SF_Data_N_Params_Per_Descriptor[version][param]	*/
int SIMIO::SF_Data_N_Params_Per_Descriptor[][MAX_NUM_DESCRIPTORS]	
 =	{
	{11,	/* version 0	GENERAL_DESCRIPTOR	*/
	17,		/* version 0	FRAME_DESCRIPTOR	*/
	8}		/* version 0	TX_EVENT_DESCRIPTOR	*/
	};
	
std::vector<param> SIMIO::SF_Data_General_Descriptor	/* GENERAL_DESCRIPTOR	*/
 =	{
	{"DescriptorIdentifier", Descriptor_Identifier, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"DataType", string_void, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"DataCount", &Zero_L, SIMIO::NULL_COMMENT(), ALONG, 0},
	{"ProcessorType", Processor_Type, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"#Frames", &N_Frames, SIMIO::NULL_COMMENT(), ALONG, 0},
	{"HowFramesVaryComment", How_Frames_Vary_Comment, SIMIO::NULL_COMMENT(), 
																ASTRING, 0},
	{"AcquisitionMethod", Acquisition_Method, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"ScanheadType", Scanhead_Type, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"ScanheadFreq", &Scanhead_Freq, Scanhead_Freq_Comment, ADOUBLE, 0},
	{"#ScanheadElements", &N_Scanhead_Els, SIMIO::NULL_COMMENT(), ALONG, 0},
	{"ElementPitch", &Element_Pitch, Element_Pitch_Comment, ADOUBLE, 0}
	};





/*----------------------------------------------------------------------------
This $$DESC record is the "Frame Descriptor" record for the SF_DATA_SPECIES
data (ie: single TX, element by element data").
These follow the General Descriptor record, and there is one of these at the 
beginning of each frames worth of data.	It contains information that is 
(potentially) frame specific.	
*/ 

std::vector<param>  SIMIO::SF_Data_Frame_Descriptor	/* FRAME_DESCRIPTOR	*/
 =	{
	{"DescriptorIdentifier", Descriptor_Identifier, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"DataType", string_void, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"DataCount", &Zero_L, SIMIO::NULL_COMMENT(), ALONG, 0},
	{"ProcessorType", Processor_Type, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"Frame#", &Frame_Number, Frame_Number_Comment, ALONG, 0},
	{"ImageDepth", &Image_Depth, Image_Depth_Comment, ADOUBLE, 0},
	{"CorrectionFileName", Correction_File_Name, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"CorrectionFileDate", Correction_File_Date, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"CorrectionFileComment", Correction_File_Comment, SIMIO::NULL_COMMENT(), 
																ASTRING, 0},
	{"ECGtrigger", ECG_Trigger, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"AcqsPerCal", &Acqs_Per_Cal, Acqs_Per_Cal_Comment, ALONG, 0},
	{"CalibrationElement", &Calibration_Element, Calibration_Element_Comment, ALONG, 0},
	{"TxEventsPerFrame", &TX_Events_Per_Frame, TX_Events_Per_Frame_Comment, ALONG, 0},
	{"WaveformsPerTxEvent", &Waveforms_Per_TX_Event, SIMIO::NULL_COMMENT(), ALONG, 0},
	{"SampsPerWaveform", &Samps_Per_Waveform, SIMIO::NULL_COMMENT(), ALONG, 0},
	{"SampleRate", &Sample_Rate, Sample_Rate_Comment, ADOUBLE, 0},
	{"StartSample", &Start_Sample, Start_Sample_Comment, ALONG, 0}
	}
;




/*----------------------------------------------------------------------------
This $$DESC record is the "Transmit Event Descriptor" record for the 
SF_DATA_SPECIES data (ie: single TX, element by element data").
These follow the Frame Descriptor record, and there is one of these for the 
data corresponding to every transmit event.	After each Transmit Event 
Descriptor record there is a $$DATA record with the binary data.	By 
convention, the data is a two dimensional array, Waveforms_Per_TX_Event(ie: 
receive elements)*Samps_Per_Waveform, where the sample index varies fastest
*/ 

char SIMIO::TX_Event_Type[STDSTRINGSIZE];
//#define ACQUISITION	"acquisition"
//#define CALIBRATION	"calibration"

std::vector<param> SIMIO::SF_Data_TX_Event_Descriptor	/* TX_EVENT_DESCRIPTOR	*/
 =	{
	{"DescriptorIdentifier", Descriptor_Identifier, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"DataType", Data_Type, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"DataCount", &Data_Count, Data_Count_Comment, ALONG, 0},
	{"ProcessorType", Processor_Type, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"DatumSize", &Datum_Size, Datum_Size_Comment, ALONG, 0},
	{"TxEvent#", &TX_Event_Number, TX_Event_Number_Comment, ALONG, 0},
	{"TxEventType", TX_Event_Type, SIMIO::NULL_COMMENT(), ASTRING, 0},
	{"TxElement#", &TX_Element_Number, TX_Element_Number_Comment, ALONG, 0}
	}
;
#endif								/* SF_DATA_SPECIES						*/








//Subapert data descriptors


int SIMIO_sub::_focus_Algorithm;
int SIMIO_sub::_n_Rays;
int SIMIO_sub::_n_Samples;
int SIMIO_sub::_n_Subapertures;
int SIMIO_sub::_n_Elements;
int SIMIO_sub::_n_Subapert_Elements;
int SIMIO_sub::_n_Subapert_Elements_Fact;
int SIMIO_sub::_n_Intervals;

int SIMIO_sub::_Group_Overlapping;

int SIMIO_sub::_n_Groups;
int SIMIO_sub::_n_Group_Rays;
int SIMIO_sub::_from_Ray;

double SIMIO_sub::_start_Angle;
double SIMIO_sub::_end_Angle;
double SIMIO_sub::_v_Tissue;
double SIMIO_sub::_omega0;
double SIMIO_sub::_half_Width;
double SIMIO_sub::_sample_Rate;
double SIMIO_sub::_array_Pitch;
double SIMIO_sub::_r_Min;
double SIMIO_sub::_r_Max;

double SIMIO_sub::_TX_Focus;
double SIMIO_sub::_RX_Focus;


char SIMIO_sub::_Depth_Units[] = CENTIMETRES;
char SIMIO_sub::_Frequency_Units[] = BACK_SECONDS;
char SIMIO_sub::_Angle_Units[] = RADIANS;
char SIMIO_sub::_Speed_Units[]  = CM_SEC;

std::vector<param> SIMIO_sub::Subapert_Data_Descriptor
={
   {"DescriptorIdentifier", Descriptor_Identifier, SIMIO::NULL_COMMENT(), ASTRING, 0},
   {"DataType", Data_Type, SIMIO::NULL_COMMENT(), ASTRING, 0},
   {"DataCount", &Data_Count, Data_Count_Comment, ALONG, 0},
   {"ProcessorType", Processor_Type, SIMIO::NULL_COMMENT(), ASTRING, 0},
   {"DatumSize", &Datum_Size, Datum_Size_Comment, ALONG, 0},

   {"focus_Algorithm",	&_focus_Algorithm, SIMIO::NULL_COMMENT(), ALONG, 0},
   {"n_Rays",		&_n_Rays, SIMIO::NULL_COMMENT(), ALONG, 0},
   {"n_Samples",		&_n_Samples, SIMIO::NULL_COMMENT(), ALONG, 0},
   {"n_Subapertures",		&_n_Subapertures, SIMIO::NULL_COMMENT(), ALONG, 0},
   {"n_Elements",		&_n_Elements, SIMIO::NULL_COMMENT(), ALONG, 0},
   {"n_Subapert_Elements",	&_n_Subapert_Elements, SIMIO::NULL_COMMENT(), ALONG, 0},
   {"n_Subapert_Elements_Fact", &_n_Subapert_Elements_Fact, SIMIO::NULL_COMMENT(), ALONG, 0},

   {"n_Groups", &_n_Groups, SIMIO::NULL_COMMENT(), ALONG, 0},		// for compatibility with old files only  
   {"n_Group_Rays", &_n_Group_Rays, SIMIO::NULL_COMMENT(), ALONG, 0},	// (really removed after 14 jan 2005. KNS). now always
								   // n_Groups = 1, n_Group_Rays = n_Rays

   {"start_Angle",	&_start_Angle,	_Angle_Units, ADOUBLE, 0},
   {"end_Angle",	&_end_Angle,	_Angle_Units, ADOUBLE, 0},
   {"v_Tissue",	&_v_Tissue,	_Speed_Units, ADOUBLE, 0},
   {"omega0",	&_omega0,	_Frequency_Units, ADOUBLE, 0},
   {"half_Width",	&_half_Width,	_Frequency_Units, ADOUBLE, 0},
   {"sample_Rate", 	&_sample_Rate,	_Frequency_Units, ADOUBLE, 0},
   {"array_Pitch",	&_array_Pitch,	_Depth_Units, ADOUBLE, 0},
   {"r_Min",		&_r_Min,		_Depth_Units, ADOUBLE, 0},
   {"r_Max",	&_r_Max,	_Depth_Units, ADOUBLE, 0},

   {"TX_Focus",	&_TX_Focus,	_Depth_Units, ADOUBLE, 0},
   {"RX_Focus",	&_RX_Focus,	_Depth_Units, ADOUBLE, 0},
   {"Group_Overlapping", &_Group_Overlapping, SIMIO::NULL_COMMENT(), ALONG, 0}	// for compatibility with old files only  
									   // (really removed after 14 jan 2005. KNS). now always false
};

std::vector<param> SIMIO_sub::Ray_Group_Descriptor
={
   {"DescriptorIdentifier", Descriptor_Identifier, SIMIO::NULL_COMMENT(), ASTRING, 0},
   {"DataType", Data_Type, SIMIO::NULL_COMMENT(), ASTRING, 0},
   {"DataCount", &Data_Count, Data_Count_Comment, ALONG, 0},
   {"ProcessorType", Processor_Type, SIMIO::NULL_COMMENT(), ASTRING, 0},
   {"DatumSize", &Datum_Size, Datum_Size_Comment, ALONG, 0},

   {"n_Group_Rays", &_n_Group_Rays, SIMIO::NULL_COMMENT(), ALONG, 0},
   {"from_Ray", &_from_Ray, SIMIO::NULL_COMMENT(), ALONG, 0},
};


std::vector<param> SIMIO_sub::Aberration_Descriptor
={
   {"DescriptorIdentifier", Descriptor_Identifier, SIMIO::NULL_COMMENT(), ASTRING, 0},
   {"DataType", Data_Type, SIMIO::NULL_COMMENT(), ASTRING, 0},
   {"DataCount", &Data_Count, Data_Count_Comment, ALONG, 0},
   {"ProcessorType", Processor_Type, SIMIO::NULL_COMMENT(), ASTRING, 0},
   {"DatumSize", &Datum_Size, Datum_Size_Comment, ALONG, 0},

   {"n_Intervals",		&_n_Intervals, SIMIO::NULL_COMMENT(), ALONG, 0},
};
