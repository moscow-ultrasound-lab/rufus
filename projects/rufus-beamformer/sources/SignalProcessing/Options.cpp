#include "Pre.h"
#include "Options.h"
#include "SimIOHeaders/SimIOHeaderProcessor.h"



int	Options :: Object_Count = 0;
int	Options :: numFiles = 1;
int	Options :: Path_Num = 0;



//char**	Options :: IP_File_Names;
//char**	Options :: OP_File_Names;
//AppFile* Options:: IP_Files;



Options :: Options()
	{

//	Init_XRAD();
//	Visible = 1;
//	focus_Algorithm = 0;


	sound_speed = cm_sec(1.54e5);
	omega0 = MHz(3);
	band_half_width = MHz(1);
	sample_rate = MHz(12);

	array_Pitch = cm(0.028);

	TX_Focus.realloc(0);
	RX_Focus.realloc(0);

// 	TX_Focus.dynamical = true;
// 	RX_Focus.dynamical = true;

//	Depth_Units = CreatePointer(char, 256);
//	Frequency_Units = CreatePointer(char, 256);
//	Angle_Units = CreatePointer(char, 256);
//	Speed_Units = CreatePointer(char, 256);

//	strcpy(Depth_Units, CENTIMETRES);
//	strcpy(Frequency_Units, M_HERZ);
//	strcpy(Angle_Units, DEGREES);
//	strcpy(Speed_Units, CM_SEC);

	if(!Object_Count)
		{
		Object_Count ++;
		}
	else	Object_Count ++;
	}

Options :: ~Options()
	{
	Object_Count --;
	}



//	'Geometry' means the globals which are
//	declared in "Subaperts data descriptor.h".


void	Options :: CopyFromSIMIO()
	{

//	focus_Algorithm = SIMIO_sub::_focus_Algorithm;
	n_rays = SIMIO_sub::_n_Rays;
	n_samples =SIMIO_sub::_n_Samples;
	n_frames = SIMIO_sub::_n_Subapertures;
	n_elements = SIMIO_sub::_n_Elements;
//	n_subaperture_elements_active = SIMIO_sub::_n_Subapert_Elements;
//	n_subaperture_elements_full = SIMIO_sub::_n_Subapert_Elements_Fact;

	if(SIMIO_sub::_n_Groups > 1) Error("Obsolete file: n_Groups are more than 1. Data will be displayed incorrectly");
	if(SIMIO_sub::_Group_Overlapping) Error("Obsolete file with 'group overlapping'. Data will be displayed incorrectly");


	if(!strcmp(SIMIO_sub::_Speed_Units, MM_SEC)) sound_speed = mm_sec(SIMIO_sub::_v_Tissue);
	else if(!strcmp(SIMIO_sub::_Speed_Units, MM_mkSEC)) sound_speed = mm_mksec(SIMIO_sub::_v_Tissue);
	else sound_speed = cm_sec(SIMIO_sub::_v_Tissue);
	// все в см/сек
	
	if(!strcmp(SIMIO_sub::_Frequency_Units, HERZ))
		{
		omega0 = Hz(SIMIO_sub::_omega0);
		band_half_width = Hz(SIMIO_sub::_half_Width);
		sample_rate = Hz(SIMIO_sub::_sample_Rate);
		}
	else if(!strcmp(SIMIO_sub::_Frequency_Units, M_HERZ))
		{
		omega0 = MHz(SIMIO_sub::_omega0);
		band_half_width = MHz(SIMIO_sub::_half_Width);
		sample_rate = MHz(SIMIO_sub::_sample_Rate);
		}
	else if(!strcmp(SIMIO_sub::_Frequency_Units, BACK_SECONDS))
		{
		omega0 = rad_sec(SIMIO_sub::_omega0);
		band_half_width = rad_sec(SIMIO_sub::_half_Width);
		sample_rate = rad_sec(SIMIO_sub::_sample_Rate);
		}
	

		{
		physical_angle	a0, a1;
		physical_length	r0, r1;	
		
		if(!strcmp(SIMIO_sub::_Angle_Units, RADIANS))
			{
			a0 = radians(SIMIO_sub::_start_Angle);
			a1 = radians(SIMIO_sub::_end_Angle);
			}
		
		else if(!strcmp(SIMIO_sub::_Angle_Units, DEGREES))
			{
			a0 = degrees(SIMIO_sub::_start_Angle);
			a1 = degrees(SIMIO_sub::_end_Angle);
			}
		
		if(!strcmp(SIMIO_sub::_Depth_Units, CENTIMETRES))
			{		
			array_Pitch = cm(SIMIO_sub::_array_Pitch);
			r0 = cm(SIMIO_sub::_r_Min);
			r1 = cm(SIMIO_sub::_r_Max);
			TX_Focus.realloc(1, cm(SIMIO_sub::_TX_Focus));
			RX_Focus.realloc(1, cm(SIMIO_sub::_RX_Focus));
			}
		
		else if(!strcmp(SIMIO_sub::_Depth_Units, MILLIMETRES))
			{		
			array_Pitch = mm(SIMIO_sub::_array_Pitch);
			r0 = mm(SIMIO_sub::_r_Min);
			r1 = mm(SIMIO_sub::_r_Max);
			TX_Focus.realloc(1, mm(SIMIO_sub::_TX_Focus));
			RX_Focus.realloc(1, mm(SIMIO_sub::_RX_Focus));
			}
		
		SetFrameSector(r0*(a1-a0).radians(), (r1-r0), a0, a1);
		}

	if(SIMIO_sub::_TX_Focus < 0) TX_Focus.realloc(0);
	if(SIMIO_sub::_RX_Focus < 0) RX_Focus.realloc(0);
	}

void	Options :: CopyToSIMIO()
	{

	SIMIO_sub::_focus_Algorithm = 0;//focus_Algorithm;
	SIMIO_sub::_n_Rays = int(n_rays);
	SIMIO_sub::_n_Samples = int(n_samples);
	SIMIO_sub::_n_Subapertures = int(n_frames);
	SIMIO_sub::_n_Elements = int(n_elements);
//	SIMIO_sub::_n_Subapert_Elements = n_subaperture_elements_active;
//	SIMIO_sub::_n_Subapert_Elements_Fact = n_subaperture_elements_full;

	SIMIO_sub::_n_Groups = 1;
	SIMIO_sub::_n_Group_Rays = int(n_rays);
	SIMIO_sub::_Group_Overlapping = false;

	SIMIO_sub::_start_Angle = start_angle().radians();
	SIMIO_sub::_end_Angle = end_angle().radians();
	SIMIO_sub::_v_Tissue = sound_speed.cm_sec();

	SIMIO_sub::_omega0 = omega0.Hz();
	SIMIO_sub::_half_Width = band_half_width.Hz();
	SIMIO_sub::_sample_Rate  = sample_rate.Hz();
	
	SIMIO_sub::_array_Pitch = array_Pitch.cm();
	SIMIO_sub::_r_Min = r_min().cm();
	SIMIO_sub::_r_Max = r_max().cm();

	if(!TX_Focus.size())
		SIMIO_sub::_TX_Focus = -1;
	else
		SIMIO_sub::_TX_Focus = TX_Focus[0].cm();
	
	if(!RX_Focus.size())
		SIMIO_sub::_RX_Focus = -1;
	else
		SIMIO_sub::_RX_Focus = RX_Focus[0].cm();

	strcpy(SIMIO_sub::_Depth_Units, CENTIMETRES);
	strcpy(SIMIO_sub::_Frequency_Units, HERZ);
	strcpy(SIMIO_sub::_Angle_Units, RADIANS);
	strcpy(SIMIO_sub::_Speed_Units, CM_SEC);
	}

bool	Options :: CompareWithSIMIO()
	{
	//if(_focus_Algorithm != focus_Algorithm) return true;
	if(size_t(SIMIO_sub::_n_Rays) != n_rays) return true;
	if(size_t(SIMIO_sub::_n_Samples) != n_samples) return true;
	if(size_t(SIMIO_sub::_n_Subapertures) != n_frames) return true;
	if(size_t(SIMIO_sub::_n_Elements) != n_elements) return true;
//	if(size_t(SIMIO_sub::_n_Subapert_Elements) != n_subaperture_elements_active) return true;
//	if(size_t(SIMIO_sub::_n_Subapert_Elements_Fact) != n_subaperture_elements_full) return true;

	if(size_t(SIMIO_sub::_n_Groups) > 1) return true;
	if(size_t(SIMIO_sub::_n_Group_Rays) != n_rays) return true;

	if(SIMIO_sub::_Group_Overlapping) return true;

	return	false;
	}

void	Options :: DumpSIMIO(FILE *Output_File)
	{
//	//SetAngleUnits(DEGREES);
//	//SetDepthUnits(CENTIMETRES);
//	SetFrequencyUnits(M_HERZ);

	CopyToSIMIO();
	
	SimIOHeaderProcessor sim_header;
	sim_header.Attach(this);
	sim_header.WriteSubapertDataDescriptor(Output_File);
	}

void	Options :: LoadSIMIO(FILE *Input_File)
	{
	if(SimGetDescriptor(	NUM_PARAMS_IN_SUBAPERT_DATA_DESCRIPTOR,
				SIMIO_sub::Subapert_Data_Descriptor,
				Input_File) != SUCCESS)
		{
		ShowString("Old version of the file header. Some additional values in",
			SIMIO_sub::_Depth_Units);
		SIMIO_sub::_TX_Focus = GetFloating("TX Focus (0 - infinity, < 0 - dynamic)",
			SIMIO_sub::_TX_Focus, -1, HUGE_VAL);
		SIMIO_sub::_RX_Focus = GetFloating("RX Focus (0 - infinity, < 0 - dynamic)",
			SIMIO_sub::_RX_Focus, -1, HUGE_VAL);
		SIMIO_sub::_Group_Overlapping = false;
		}
	CopyFromSIMIO();

//	SetFrequencyUnits(M_HERZ);

//	SetAngleUnits(RADIANS);
//	SetDepthUnits(CENTIMETRES);
//	SetFrequencyUnits(BACK_SECONDS);
	}
/*
void	Options :: SetAngleUnits(const char *unit){//	Default is Radian
	float	factor = 1.0;
	int	Index = 0;

	if(!strcmp(unit, DEGREES))
		{
		factor *= (180/PI);
		}
	if(!strcmp(Angle_Units, DEGREES))
		{
		factor /= (180/PI);
		}
//	start_angle *= factor;
//	end_angle *= factor;
	strcpy(Angle_Units, unit);
	}

void	Options :: SetDepthUnits(const char *unit){//	Default is cm
	float	factor = 1.0;
	int	Index = 0;

	if(!strcmp(unit, MILLIMETRES))
		{
		factor *= 10;
		}
	if(!strcmp(Depth_Units, MILLIMETRES))
		{
		factor /= 10;
		}
//	r_min *= factor;
//	r_max *= factor;
//	array_Pitch *= factor;

//	TX_Focus *= factor;
//	RX_Focus *= factor;

	strcpy(Depth_Units, unit);
	}


void	Options :: SetFrequencyUnits(const char *unit){ //	Default is MHz
	float	factor = 1.0;
	int	Index = 0;

	if(!strcmp(unit, BACK_SECONDS))
		{
		factor *= (two_pi()*1.0E6);
		}
	if(!strcmp(Frequency_Units, BACK_SECONDS))
		{
		factor /= (two_pi()*1.0E6);
		}

	if(!strcmp(unit, HERZ))
		{
		factor *= 1.0E6;
		}
	if(!strcmp(Frequency_Units, HERZ))
		{
		factor /= 1.0E6;
		}


	omega0 *= factor;
	band_half_width *= factor;
	sample_Rate *= factor;
	strcpy(Frequency_Units, unit);
	}

void	Options :: SetSpeedUnits(const char *unit){//	Default is nm/sec
	float	factor = 1.0;

	if(!strcmp(unit, MM_SEC))
		{
		factor *= 1.0E6;
		}
	if(!strcmp(Speed_Units, MM_SEC))
		{
		factor /= 1.0E6;
		}

	if(!strcmp(unit, CM_SEC))
		{
		factor *= 1.0E5;
		}
	if(!strcmp(Speed_Units, CM_SEC))
		{
		factor /= 1.0E5;
		}


	sound_speed *= factor;
	strcpy(Speed_Units, unit);
	}
*/
void	Options :: Attach(Options *x)
	{

//	SetDepthUnits(x->Depth_Units);
//	SetFrequencyUnits(x->Frequency_Units);
//	SetAngleUnits(x->Angle_Units);

//	focus_Algorithm = x -> focus_Algorithm;
//	Visible = x -> Visible;

	n_rays = x -> n_rays;
	n_samples = x -> n_samples;

//	n_intervals = x -> n_intervals;
//	interval_increment = x -> interval_increment;


	n_frames = x -> n_frames;
	n_elements = x -> n_elements;
//	n_subaperture_elements_active = x -> n_subaperture_elements_active;
//	n_subaperture_elements_full = x -> n_subaperture_elements_full;
	
	PhysicalFrameDimensions::operator=(*x);
//	start_angle = x -> start_angle;
//	end_angle = x -> end_angle;
//	depth_range = x->depth_range;
//	scanning_trajectory_length = x->scanning_trajectory_length;
//	steering_angle = x->steering_angle;

//	r_min = x -> r_min;
//	r_max = x -> r_max;
	
	sound_speed = x -> sound_speed;
	omega0 = x -> omega0;
	band_half_width = x -> band_half_width;
	sample_rate = x -> sample_rate;
	array_Pitch = x -> array_Pitch;

	TX_Focus.MakeCopy(x -> TX_Focus);
	RX_Focus.MakeCopy(x -> RX_Focus);
	}
	


void	Options :: ExportScanConverterOptions(ScanConverterOptions &sco)
	{
	static_cast<PhysicalFrameDimensions&>(sco) = *this;
/*
	sco.depth_range = depth_range;
	sco.start_angle = start_angle;
	sco.end_angle = end_angle;
	
	if((end_angle-start_angle).radians())
		sco.scanning_trajectory_length = r_min()*(sco.end_angle-sco.start_angle).radians();
	else sco.scanning_trajectory_length = array_Pitch * n_rays;
*/
	}

