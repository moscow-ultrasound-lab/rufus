/*
File : Std__Headers.h

Created B Robinson	18-Feb-91
Updated B Robinson	21-May-91
		Add conditional compilation of each data species (to avoid static
		storage getting real big
Updated	B Robinson	12-Jun-91
		Modify/Add focal point stuff
		Correct typo in Ray_Lateral_Displacment
Updated	B Robinson	24-Jun-91
		Correct SEER_Data_N_Params_Per_Descriptor (was EL_Data_N_P..... )
Updated	B Robinson	6-Apr-92
		Add SUMMED_DATA_SPECIES
Updated	B Robinson	6-May-92
		Add Interpolation_Factor to FRAME_DESCRIPTOR of SUMMED_DATA_SPECIES
Updated	B Robinson	13-May-92
		Add Interpolation_Factor to
		SF_Image_Params_Descriptor of SF_IMAGE_DATA_SPECIES and
		EL_Data_Frame_Descriptor of EL_DATA_SPECIES.
Updated B Robinson	29-Jul-92
		Add define for STD_HISTORY_DESCRIPTOR_NUM_PARAMS since Think C 5.0
		does not like taking sizeof(Standard_History) in the expression
		sizeof(Standard_History)/sizeof(param)
Updated B Robinson	30-Sept-92
		Correct bug in EL_Data_Num_Descriptors[] where
		EL_Data_Num_Descriptors[1] = 4 (due to mistake in adding
		Interpolation_Factor on 29-Jul-92 ??) but should have been = 3.
Updated	B Robinson	29-Oct-92
		Add #define for SYNTH_A_LINE
		Add Fractional_Start_Sample keyword phase to the EL_Data_Frame_Descriptor
		of the EL_DATA_SPECIES (since this is a valid (and more general) case
		than Start_Sample (which is long).
		NB: Start_Sample may be therefore be in error slightly.
Updated	B Robinson	22-Jan-93
		Add #define for SHORT_INT (as a 2-byte integer)
Updated	B Robinson	5-Feb-93
		Modify SF_IMAGE_DATA_SPECIES to have TX/RX focal point & TX/RX
		delay quantisation.
		add #define NS   "nano_seconds"
Updated	B Robinson	26-Feb-93
		Add TX & RX delay quantisation to EL_DATA_SPECIES & SUMMED_DATA_SPECIES



(1) FUNCTION AND PURPOSE :
	----------------------

A dictionary of standard SimIO style parameter record objects for use in RF
and processed data file headers.	The objects are global, (ie: can be
accessed by any function that #includes this header), and are either
variables, strings, or arrays of param structures (ie: the arguments of SimIO
functions such as SimPutHistory(), SimGetHistory(), etc).



(2) USAGE :
	-------

In this file, various objects are either :
(a) defined (ie: declared & have storage created) and (for arrays of
'keyword phrase' param structures) initialised,	or
(b) declared as exteral.
In addition, specific standard keyword phrase values are #defined.

Whether (1) or (2) occurs depends on if DEFINING_STD_FILE_HEADERS_0 is
#defined.   DEFINING_STD_FILE_HEADERS_0 must be #defined in one (and only one
file).	When this file is #included in a file in which
DEFINING_STD_FILE_HEADERS_0 is #defined, (1) will occur.	If additional
source files also want to have access to SimIO headers, then this file is
also #included, but without #defining DEFINING_STD_FILE_HEADERS_0, which will
result in extern declarations.


(3) DESIGN PHILOSOPHY :
	-------------------

Data of a particular "species" may come from a variety of processes. For
example, summed data can be acquired directly, ie: from a single process, or
it could come from a number of processes in cascade, eg: simulate "synthetic
focus" raw data, synthesize this into element by element data, then sum.
It is difficult to extract from the cascaded history exactly what the
parameters of the current data file are.	For example, the final process
might change the sample rate or the start depth but other history records
also might have different data rates.

Therefore, after the history records, there is a "GENERIC DATA DESCRIPTOR"
$DESC record (with std generic keyword phrases) followed by other descriptor
records (with data species specific keyword phrases) that unambiguously
identify what the characteristics of the data in the file are.	As an example,
an element by element data file contains the following records :

	[{STANDARD_HISTORY}] [...]
	[{GENERIC_DATA_DESCRIPTOR]
	[{GENERAL_DESCRIPTOR}]
	[{FRAME_DESCRIPTOR} [{TX_EVENT_DESCRIPTOR}{data}] [...]] [...]

The GENERIC_DATA_DESCRIPTOR is of a standard form & stores the "DataSpecies"
keyword.   The contents of the GENERAL_DESCRIPTOR, FRAME_DESCRIPTOR and
TX_EVENT_DESCRIPTOR are specific to the particular data species which in this
example of element by element data might be :
SF_DATA_SPECIES	"single TX, element by element RX, RF data" or
EL_DATA_SPECIES	"TX beamformed, element by element RX, RF data".
other data species are :
SUMMED_DATA_SPECIES "summed RF data" &
SF_IMAGE_SPECIES	"synthetic focus image raster data"
AB_DATA_SPECIES		"aberration data"
SEER_DATA_SPECIES	"Simulated Elevational Element Response data"

Note that these descriptor records may duplicate information contained in the
history records.	The distinction is that the history is intended to
be reviewed, whereas the descriptor records are intended to have parameters
extracted from them.



(4) UPDATING THE DESCRIPTORS :
	--------------------------

The format of the GENERIC_DATA_DESCRIPTOR may not change.	However, the
GENERIC_DATA_DESCRIPTOR stores the "DescriptorVersion" and the
"NumXtraDescriptors" keywords.	These provide a means to keep track of the
modifications to the species specific descriptor records (ie: all those
following the GENERIC_DATA_DESCRIPTOR $DESC record).

There are up to 16 (pairs) of keyword phases following the NumXtraDescriptors
keyword phase.	These provide information on the type and # keywords in the
descriptor record types (<= 16) that follow in the file.
The concept is that if additional keyword phases are needed in a particular
species specific descriptor, the version of species specific descriptors (as
a group) is incremented, and the additional keyword phases are added at the
end of the particular descriptor.	The other descriptors for the species are
unaltered.

The rules for modifying the species specific $DESC records following the
GENERIC_DATA_DESCRIPTOR record are :

(a) Can only add keyword phrases, and these must be added at the end of the
param array.	(ie: the SimIO routines will write or search down the
specified number of keyword phrases, so that new phrases must be added at
the end).

(b) The number of versions is unlimited.

(c) The number of keyword phases in each descriptor is unlimited.

(d) The max # species specific descriptors for any version is
MAX_NUM_DESCRIPTORS (= 16).	This is because a finite amount of storage is
set aside in GENERIC_DATA_DESCRIPTOR for the "XtraDescIdentifier_n" &
"NumXtraDescParams_n" keyword phases in each descriptor (ie: n <
MAX_NUM_DESCRIPTORS).

(e) Species specific descriptor records can be added or subtracted with
each version but must keep to the order specified in the enum statement.
???	<-- I think but not tested.

(f) To make a new version, need to modify the ?_Num_Descriptors[version],
?_N_Params_Per_Descriptor[version][param], and add extra keyword phrases
to all the appropriate ?_Descriptor[param] arrays.
In addition, if an additional descriptor is created, the enum statement
detailing the order of the descriptors in the XtraDescriptor keyword phrases
at the end of the GENERIC_DATA_DESCRIPTOR will need to modified, and
additional #defines for standard keyword values might be necessary.



(5) NOTES :
	_______

(a) With the exception of user supplied comment strings, the max string
size and comment size is limited to STDSTRINGSIZE & STDCOMMENTSIZE = 64 chars.
This is smaller than the SimIO size of 256 chars, to avoid using too much
static storage.   There is no checking of this linit, so be careful when
#defining new strings (the user should never create one of these smaller
strings interactively).


*/


#ifndef __SimIOHeaders_h
#define __SimIOHeaders_h

#include <vector>
#include <string>
#include "SimIO.h"

#define INCLUDE_SF_DATA_SPECIES

class SIMIO
{
public:


/*
definitions and declarations that are common to two or more keyword phrases
note : use NO_COMMENT rather than "" or " ", since the SimIO does not like
to read "" or " ".
*/
	enum
	{
		MAX_NUM_DESCRIPTORS = 16,
		STDSTRINGSIZE	=	512,
		STDCOMMENTSIZE	=	512
	};

// #define MAX_NUM_DESCRIPTORS 16
//#define NO_COMMENT			"no comment"
	static char		*NULL_COMMENT(){ return NULL; }
	static std::string NO_COMMENT(){ return "no comment"; }
//#define	NULL_COMMENT		(char *)NULL
#define NA					"Not Applicable"
#define	SHORT_INT			"short (2-byte) integer"
#define BASE_0				"(base 0)"
#define HZ					"Hz"
#define	NS					"nano_seconds"
#define MILLI_METRES		"milli-metres"
#define MICRO_METRES		"micro-metres"
#define	STD_FOCAL_COMMENT	"milli-metres (-1 is infinity, 0 is dynamic)"
#define	FOCUSSED_AT_INFINITY	-1.0
#define	DYNAMIC_FOCUS			0.0
#define	STD_SPEED_OF_SOUND_COMMENT	"milli-metres/uSec"
#define	STD_PERCENT_BW_COMMENT		"% (-6dB)"
#define STD_RMS_COMMENT		"rms"
//#define	DEGREES				"degrees"

	static std::string		DEGREES(){ return std::string("degrees"); }

// #define STDSTRINGSIZE		64
// #define STDCOMMENTSIZE		64
// #define STDSTRINGSIZE		512
// #define STDCOMMENTSIZE		512

	static char	Descriptor_Identifier[STDSTRINGSIZE];
	static char	Xtra_Desc_Identifier[MAX_NUM_DESCRIPTORS][STDSTRINGSIZE];
#define GENERIC_DATA_DESCRIPTOR		"Generic Data Descriptor"
#define GENERAL_DESCRIPTOR	"General Descriptor (species specific, frame inspecific)"
#define FRAME_DESCRIPTOR			"Frame Descriptor (species specific)"
#define TX_EVENT_DESCRIPTOR 		"Transmit Event Descriptor (species specific)"
// #define	AB_PARAMS_DESCRIPTOR		"Aberration Parameters Descriptor"
// #define	AB_DATA_DESCRIPTOR			"Aberration Data Descriptor"
// #define	SF_IMAGE_PARAMS_DESCRIPTOR	"SF Image Parameters Descriptor"
// #define	SF_IMAGE_ROW_DATA_DESCRIPTOR	"SF Image Row Data Descriptor"
// #define SEER_GENERAL_DESCRIPTOR		"SEER General Parameters Descriptor"
// #define SEER_REFERENCE_DESCRIPTOR	"SEER Reference Waveform Descriptor"
// #define	SEER_RF_DATA_DESCRIPTOR		"SEER RF Data Descriptor (source coords specific)"
// #define SEER_DETECTED_DATA_DESCRIPTOR	"SEER detected Data Descriptor (range specific)"

	static char	Process_Name[STDSTRINGSIZE];
#define RF_ACQ				"rf_acq"
//#define SF_SYNTHESIZE		"SF_synthesize"
// #define	SYNTH_IMAGE			"synth_image"
// #define	SYNTH_A_LINE		"synth_A_line"
// #define SFFT_SYNTH			"SFFT_synth"
// #define SF_SIMULATE			"SF_simulate"
// #define	EL_SIMULATE			"EL_simulate"
// #define	HEADERS_ON_OFF		"headers_on_off"
// #define SEER				"SEER"
// #define SEER_skew			"SEER_skew"


	static char	Data_Species[STDSTRINGSIZE];
#define SF_DATA_SPECIES		"single TX, element by element RX, RF data"
//#define EL_DATA_SPECIES		"TX beamformed, element by element RX, RF data"
// #define SUMMED_DATA_SPECIES "summed RF data"
// #define SF_IMAGE_SPECIES	"synthetic focus image raster data"
// #define	AB_DATA_SPECIES		"aberration data"
// #define	SEER_DATA_SPECIES	"Simulated Elevational Element Response data"

	static char	Data_Genesis[STDSTRINGSIZE];
#define EXPT_GENESIS		"experimental"
#define SIM_GENESIS			"simulated"
#define SYNTH_OF_EXPT_GENESIS	"synthetic focussing of experimental"
#define SYNTH_OF_SIM_GENESIS	"synthetic focussing of simulated"
// #define	ANAL_OF_SEER_GENESIS	"analysis of SEER output data"
// #define	UNKNOWN_GENESIS			"too many processing steps to track genesis"


	static char	ECG_Trigger[STDSTRINGSIZE];
#define ECG_TRIGGER_ON		"On"
#define ECG_TRIGGER_OFF		"Off"
#define ECG_TRIGGER_NA		NA

	static char	Acquisition_Method[STDSTRINGSIZE];
#define SRAM				"SRAM"
#define FIFO				"FIFO"
#define	SYNTHESIZED			"synthesized"
#define	SIMULATED			"simulated"

	static char	Scanhead_Type[STDSTRINGSIZE];
#define ATL_PA				"ATL UM-9 phased array"
#define ECHO_PA				"Echo UM-9 Ultrasound phased array"
#define AA					"ATL standard annular array"
#define WAAA				"ATL wide aperture annular array"

	static	char	Data_Complexity[STDSTRINGSIZE];
#define	REAL_COMPONENT		"real component only"
#define IMAGINARY_COMPONENT	"imaginary component only"
#define	ANALYTIC			"analytic components"


	static char	Num_Xtra_Descriptors_Comment[STDCOMMENTSIZE];
#define STD_NUM_XTRA_DESC_COMMENT	"# descriptor records for this species & version"

	static	char	Element_Pitch_Comment[STDCOMMENTSIZE];
#define	ATL_3PA_PITCH		0.2794
	static char				Image_Depth_Comment[STDCOMMENTSIZE];

	static char	Acqs_Per_Cal_Comment[STDCOMMENTSIZE];
#define STD_ACQS_PER_CAL_COMMENT	"<--- 0 means no calibration pulses"

	static char	TX_Events_Per_Frame_Comment[STDCOMMENTSIZE];
#define CAL_and_ACQ			"(calibration & acquisition)"
#define	RAYS				"rays"

	static char	Data_Count_Comment[STDCOMMENTSIZE];
#define ITEMS	"items"

	static char	Datum_Size_Comment[STDCOMMENTSIZE];
#define BYTES	"bytes"

	static char	Start_Sample_Comment[STDCOMMENTSIZE];
#define STD_START_SAMPLE_COMMENT	"(relative to a TX from the array origin)"

	static char	Fractional_Start_Sample_Comment[STDCOMMENTSIZE];
#define STD_FRACTIONAL_START_SAMPLE_COMMENT	"(from the x, y, z origin)"


	static char	Process_Description[MAXSTRINGSIZE];
	static char	Process_Date_Time[STDSTRINGSIZE];
	static char	Processor_Type[STDSTRINGSIZE];
	static char	Process_In_File_Name[STDSTRINGSIZE];
	static char	Process_In_File_Date[STDSTRINGSIZE];
	static char	Process_In_File_Comment[MAXSTRINGSIZE];
	static char	Process_Out_File_Name[STDSTRINGSIZE];
	static char	Process_Comment[MAXSTRINGSIZE];
	static char	Data_Type[STDSTRINGSIZE];
	static char	How_Frames_Vary_Comment[MAXSTRINGSIZE];
	static char	Original_RF_Data_File_Name[STDSTRINGSIZE];
	static char	Original_RF_Data_File_Date[STDSTRINGSIZE];
	static char	Original_RF_Data_Comment[MAXSTRINGSIZE];
	static char	Correction_File_Name[STDSTRINGSIZE];
	static char	Correction_File_Date[STDSTRINGSIZE];
	static char	Correction_File_Comment[MAXSTRINGSIZE];

	static char	Frame_Number_Comment[STDCOMMENTSIZE];
	static char	Scanhead_Freq_Comment[STDCOMMENTSIZE];
	static char	Calibration_Element_Comment[STDCOMMENTSIZE];
	static char	Sample_Rate_Comment[STDCOMMENTSIZE];
	static char	TX_Event_Number_Comment[STDCOMMENTSIZE];
	static char	TX_Element_Number_Comment[STDCOMMENTSIZE];
	static	char	Start_Angle_Comment[STDCOMMENTSIZE];
	static	char	Delta_Angle_Comment[STDCOMMENTSIZE];
	static	char	Start_Lateral_Disp_Comment[STDCOMMENTSIZE];
	static	char	Delta_Lateral_Disp_Comment[STDCOMMENTSIZE];
	static	char	TX_Focal_Point_Comment[STDCOMMENTSIZE];
	static	char	RX_Focal_Point_Comment[STDCOMMENTSIZE];
	static	char	TX_Delay_Quant_Comment[STDCOMMENTSIZE];
	static	char	RX_Delay_Quant_Comment[STDCOMMENTSIZE];
	static	char	Ray_Angle_Comment[STDCOMMENTSIZE];
	static	char	Ray_Lateral_Disp_Comment[STDCOMMENTSIZE];

	static double	Process_Version;
	static double	Scanhead_Freq;
	static	double	Element_Pitch;
	static double	Image_Depth;
	static double	Sample_Rate;
	static	double	Start_Angle;
	static	double	Delta_Angle;
	static	double	Start_Lateral_Displacement;
	static	double	Delta_Lateral_Displacement;
	static	double	TX_Focal_Point;
	static	double	RX_Focal_Point;
	static	double	TX_Delay_Quantisation;
	static	double	RX_Delay_Quantisation;
	static	double	Ray_Angle;
	static	double	Ray_Lateral_Displacement;
	static double	Fractional_Start_Sample;


	static long	Zero_L;
	static long	N_Frames;		/* make these long to avoid later grief 	*/
	static long	Frame_Number;	/* when using these as variables in			*/
	static long	N_Scanhead_Els;	/* expressions to set sizes					*/
	static long	Acqs_Per_Cal;
	static long	Calibration_Element;
	static long	TX_Events_Per_Frame;
	static long	Start_Sample;
	static long	Samps_Per_Waveform;
	static long	Waveforms_Per_TX_Event;
	static long	TX_Event_Number;
	static long	TX_Element_Number;
	static long	Data_Count;
	static long	Datum_Size;
	static long	Interpolation_Factor;

	static int		Num_Xtra_Desc_Params[MAX_NUM_DESCRIPTORS];
	static int		Descriptor_Version;







	/*----------------------------------------------------------------------------
	This $$HIST record is the standard form of the history record that is appended
	to previous history record and immediately preceeds the $$DESC standard
	"Data Descriptor" record.
	*/

	static std::vector<param> Standard_History;

#define	STD_HISTORY_DESCRIPTOR_NUM_PARAMS	(11)


/*----------------------------------------------------------------------------
This $$DESC record is the "Generic Data Descriptor" record.

It immediately follows the $$HIST records.

Note that this repeats much of the information in the most recent standard
history record, but this is intentional for the reasons described above, and
by repeating various STD_HISTORY keyword phrases, this std will tend to be
enforced by default.
*/
#define GENERIC_DATA_DESCRIPTOR_BASE_NUM_PARAMS	20

	static long	Num_Xtra_Descriptors;

#if 0
	static char	string_void[STDSTRINGSIZE];
#else
	static char *string_void;
#endif

	static std::vector<param> Generic_Data_Descriptor;			/* GENERIC_DATA_DESCRIPTOR	*/



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
#define	SF_VERSION_0	0

	enum	{/* this is the order that the xtra	*/
		SF_Data_General_Descriptor_Num,	/* descriptor records names/sizes are 	*/
		SF_Data_Frame_Descriptor_Num,	/* listed in the GENERIC_DATA_DESCRIPTOR*/
		SF_Data_TX_Event_Descriptor_Num
	};

	static long	SF_Data_Num_Descriptors[];		/* indexed by version #		*/

	/* indexed : SF_Data_N_Params_Per_Descriptor[version][param]	*/
	static int SF_Data_N_Params_Per_Descriptor[][MAX_NUM_DESCRIPTORS];

	static std::vector<param> SF_Data_General_Descriptor;	/* GENERAL_DESCRIPTOR	*/


	/*----------------------------------------------------------------------------
	This $$DESC record is the "Frame Descriptor" record for the SF_DATA_SPECIES
	data (ie: single TX, element by element data").
	These follow the General Descriptor record, and there is one of these at the
	beginning of each frames worth of data.	It contains information that is
	(potentially) frame specific.
	*/

	static std::vector<param> SF_Data_Frame_Descriptor;	/* FRAME_DESCRIPTOR	*/




	/*----------------------------------------------------------------------------
	This $$DESC record is the "Transmit Event Descriptor" record for the
	SF_DATA_SPECIES data (ie: single TX, element by element data").
	These follow the Frame Descriptor record, and there is one of these for the
	data corresponding to every transmit event.	After each Transmit Event
	Descriptor record there is a $$DATA record with the binary data.	By
	convention, the data is a two dimensional array, Waveforms_Per_TX_Event(ie:
	receive elements)*Samps_Per_Waveform, where the sample index varies fastest
	*/

	static char	TX_Event_Type[STDSTRINGSIZE];
#define ACQUISITION	"acquisition"
#define CALIBRATION	"calibration"

	static std::vector<param> SF_Data_TX_Event_Descriptor;	/* TX_EVENT_DESCRIPTOR	*/



};//class SIMIO

//Subapert data descriptors

class SIMIO_sub : public SIMIO
{
	public:
	#define	SUBAPERT_DATA_DESCRIPTOR		"Subapert Data Descriptor"
	#define	ABERRATION_DESCRIPTOR			"Aberration Descriptor"
	#define	RAY_GROUP_DESCRIPTOR			"Ray Group Descriptor"
	#define	NUM_PARAMS_IN_SUBAPERT_DATA_DESCRIPTOR	26
	#define	NUM_PARAMS_IN_RAY_GROUP_DESCRIPTOR	7
	#define	NUM_PARAMS_IN_ABERRATION_DESCRIPTOR	6

	#define	CENTIMETRES	"cm"
	#define	MILLIMETRES	"mm"
	#define	HERZ		"Hz"
	#define	M_HERZ		"MHz"
	#define	BACK_SECONDS	"sec-1"
	#define	DEGREES		"degrees"
	#define	RADIANS		"radians"
	#define	MM_SEC		"mm/sec"
	#define	CM_SEC		"cm/sec"
	#define	MM_mkSEC	"mm/µsec"


		static	int	_focus_Algorithm;
		static	int	_n_Rays;
		static	int	_n_Samples;
		static	int	_n_Subapertures;
		static	int	_n_Elements;
		static	int	_n_Subapert_Elements;
		static	int	_n_Subapert_Elements_Fact;
		static	int	_n_Intervals;

		static	int	_Group_Overlapping;

		static	int	_n_Groups;
		static	int	_n_Group_Rays;
		static	int	_from_Ray;

		static	double	_start_Angle;
		static	double	_end_Angle;
		static	double	_v_Tissue;
		static	double	_omega0;
		static	double	_half_Width;
		static	double	_sample_Rate;
		static	double	_array_Pitch;
		static	double	_r_Min;
		static	double	_r_Max;

		static	double	_TX_Focus;
		static	double	_RX_Focus;


		static	char	_Depth_Units[MAXSTRINGSIZE];
		static	char	_Frequency_Units[MAXSTRINGSIZE];
		static	char	_Angle_Units[MAXSTRINGSIZE];
		static	char	_Speed_Units[MAXSTRINGSIZE];

		static	std::vector<param> Subapert_Data_Descriptor;

		static	std::vector<param> Ray_Group_Descriptor;


		static	std::vector<param> Aberration_Descriptor;

}; // namespace SIMIO -> class


#ifndef THIS_PROCESSOR_TYPE
#define THIS_PROCESSOR_TYPE "Generic processor type"
#endif


#endif //__SimIOHeaders_h
