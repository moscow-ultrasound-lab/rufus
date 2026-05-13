#include "pre.h"
#include "SyntheticFocusingOptions.h"
#include "ATL_src_reduced/atl_lut_interpolator.h"

#include "RawSFDataSourceSimIO.h"
#include "RawSFDataSourceTransEcho.h"
#include "RawSFDataSourceSimulation.h"
#include "RawSFDataSourceINM.h"

/*
//for reading txt files with aberration profiles
#include <iostream>
#include <fstream>
#include <string>
//#include <charconv>
//#include <system_error>
#include <sstream>
*/

//#include <iomanip>

extern	char	this_process_comment[MAXSTRINGSIZE];

// по всему материалу была серьезнаЯ ошибка: смешаны понЯтиЯ частоты входных и выходных отсчетов. сейчас это исправлено, но следы могли остатьсЯ


XRAD_BEGIN




//physical_frequency	new_sample_rate = MHz(6);	//MHz
void dummy()
{
/*
	string	filename = GetFileNameRead("Aberration profile file", SavedGUIValue("*.txt"), "*.txt");
	std::ifstream file(filename, std::ios::in);
	std::string line;
	float x(0), y(0), z(0), i(0);
	RealFunctionF32 aberration(64,0);
	if (file.is_open())
	{
		std::cout << "File: " << filename << "\n\n";
		while (std::getline(file, line)) {
			std::cout << line << "\n\n";
			std::istringstream in(line);      //make a stream for the line itself	
			for (int i = 0; i < size(aberration); i++)
			{
				in >> aberration[i];			 //now read the whitespace-separated floats
			}
		}
	}
	else {
		std::cout << "Failed to open the file";
	}
	file.close();

	std::cout << "\n\nFloats\n";
	for (int i = 0; i < size(aberration); i++)
	{
		std::cout << aberration[i] << " ";
	}	
	*/
}

SyntheticFocusingOptions::SyntheticFocusingOptions() :SectorData()
{
	size_t	answer = 1;

//	dummy();

	//answer = Decide("Data source", {"Sim IO", "TransEcho", "INM"}, SavedGUIValue(answer));
	answer = Decide("Data source", { "Sim IO", "TransEcho", "INM", "Simulation" }, SavedGUIValue(answer));

//	answer = 1;
	switch(answer)
	{
		case 0:
			data_source.reset(new RawSFDataSourceSimIO);
			break;
		case 1:
			data_source.reset(new RawSFDataSourceTransEcho);
			break;
		case 2:
			data_source.reset(new RawSFDataSourceINM);
			break;
		case 3:
			data_source.reset(new RawSFDataSourceSimulation);
			break;
	}

	data_source->Init();
	data_source->LoadData();
	
	

	Set_Default_Options();
	ShowOptions();
}



SyntheticFocusingOptions :: ~SyntheticFocusingOptions()
{
}


void	SyntheticFocusingOptions::Set_Default_Options()
{
	Signal_IP_File_Name = data_source->GetFileName();

	// параметры сырых данных:
	sound_speed = cm_sec(1.54e5);

	n_frames = 1;
	n_subaperture_elements_active = data_source->n_tx_elements/n_frames;
	n_subaperture_elements_full = n_subaperture_elements_active;

//	r_min = 0.5 * SIMIO::Start_Sample*sound_speed/sample_rate;
//	r_max = 0.5 * (SIMIO::Start_Sample + SIMIO::Samps_Per_Waveform)*sound_speed/sample_rate;

	// параметры фокусируемых данных. пока задаютсЯ 
	n_rays = data_source->recommended_n_rays;//128;

	n_samples = data_source->hilbert_samples;
	sample_rate = data_source->hilbert_signal_sample_rate;//Hz(SIMIO::Sample_Rate)/2;

//	n_samples = SIMIO::Samps_Per_Waveform/2;
//	sample_rate = Hz(SIMIO::Sample_Rate)/2;

// 	first_sample = double(SIMIO::Start_Sample) * sample_rate/data_source->raw_signal_sample_rate;
// 	last_sample = double(SIMIO::Start_Sample + SIMIO::Samps_Per_Waveform)  * sample_rate/data_source->raw_signal_sample_rate;

	first_sample = double(data_source->first_raw_sample) * sample_rate/data_source->raw_signal_sample_rate;
	last_sample = double(data_source->last_raw_sample)  * sample_rate/data_source->raw_signal_sample_rate;


// 		{
	double	n_samps_per_mm_1_way = sample_rate.MHz() / data_source->sound_speed.mm_mksec();
	physical_angle a0 = -data_source->recommended_angle;-degrees(45);
	physical_angle a1 = data_source->recommended_angle;//degrees(45);
	physical_length r0 = mm(0.5 * double(first_sample) / n_samps_per_mm_1_way);
	physical_length r1 = mm(0.5 * double(last_sample) / n_samps_per_mm_1_way);
// 		
	SetFrameSector(r0*(a1-a0).radians(), (r1-r0), a0, a1);
//		}

	omega0 = data_source->probe_carrier;//MHz(3)
	band_half_width = data_source->probe_bandwidth/2;//MHz(0.5);

//	array_Pitch = mm(SIMIO::Element_Pitch);
	array_Pitch = data_source->array_pitch;


	TX_Focus.realloc(0);
	RX_Focus.realloc(0);

	TX_Focus.static_direction = false;
	sector_centre_angle = radians(0);

	apodization_tx = false;
	apodization_rx = false;

	n_merged_elements = 1;

	cancel_dc_offset_correction = false;
}

void	SyntheticFocusingOptions::UpdateFocusingSectorBounds()
{
}



void	SyntheticFocusingOptions::Modify_Options()
{
	static bool	vary_sound_speed(false);
	static bool	merge_elements(false);
	static bool	display_raw_data(false);
	static bool	change_centre_angle(false);
	static physical_angle centre_angle(degrees(0));

	/*
	GetCheckboxDecision("Focusing options",
		{"Display raw data",	"TX apodization",	"RX apodization",	"Vary sound speed",	"Cancel DC offset correction",	"Merge elements",	"Centre angle offset"},
		{&display_raw_data,		&apodization_tx,	&apodization_rx,	&vary_sound_speed,	&cancel_dc_offset_correction,	&merge_elements,	&change_centre_angle});
	*/

	if(display_raw_data) data_source->Display();
	if(merge_elements) n_merged_elements = GetSigned("How many elements to merge?", 1, 1, data_source->n_tx_elements);

	sound_speed = data_source->sound_speed;

	if(n_frames==1) frame_sense = fs_frame;
	else frame_sense = fs_subaperture;

	sector_centre_angle = centre_angle = change_centre_angle ? degrees(GetFloating("Sector center angle", centre_angle.degrees(), -90, 90)) : degrees(0);

	if(frame_sense == fs_frame)
	{
		if(vary_sound_speed)
		{
			frame_sense = fs_focus;
			n_frames = GetSigned("How many hypotheses?", 5, 3, 48);
		}
	}

	physical_length	cm_per_raw_sample_2_way;
	physical_length	min_possible_depth, max_possible_depth;
	cm_per_raw_sample_2_way = cm(0.5*data_source->sound_speed.cm_sec() / data_source->raw_signal_sample_rate.Hz());

	physical_angle	a0, a1;
	physical_length r0, r1;

	a0 = start_angle();
	a1 = end_angle();

	min_possible_depth = data_source->first_raw_sample*cm_per_raw_sample_2_way;
	max_possible_depth = data_source->last_raw_sample*cm_per_raw_sample_2_way;

	r0 = min_possible_depth;
	r1 = max_possible_depth;

	physical_length	cm_per_sample_1_way = cm(data_source->sound_speed.cm_sec() / sample_rate.Hz());

	first_sample = r0/cm_per_raw_sample_2_way + 1.0;// round up
	r0 = first_sample*cm_per_raw_sample_2_way;
	last_sample = r1/cm_per_raw_sample_2_way;	// round down
	r1 = last_sample*cm_per_raw_sample_2_way;

	SetFrameSector(r0*(a1-a0).radians(), (r1-r0), a0, a1);

	sprintf(this_process_comment, "No comment");

//	CreateComment();
}




// void	SyntheticFocusingOptions :: CreateComment()
// 	{
// 	if(!TX_Focus.size()) sprintf(Object_Comment,"%s dynamic focus on TX aperture,\n", Object_Comment);
// 	else	sprintf(Object_Comment,"%s TX aperture focused at ***infinity,\n", Object_Comment);
// 
// 	if(!RX_Focus.size()) sprintf(Object_Comment,"%sdynamic focus on RX aperture,", Object_Comment);
// 	else	sprintf(Object_Comment,"%s RX aperture focused at ***infinity,", Object_Comment);
// 	
// 	switch(focus_Algorithm)
// 		{
// 		case MEMORY_TEST:
// 			sprintf(Object_Comment,"Memory test is done. Delete OP file.");
// 			break;
// 		case FILTRED_APERTURE:	
// 			sprintf(Object_Comment,"%s, filtred aperture", Object_Comment);
// 			break;
// 		case SIMULATE_ABERRATION:
// 			sprintf(Object_Comment,"%s, simulated aberration", Object_Comment);
// 			break;
// 		case CORRECT_ABERRATION:
// 			sprintf(Object_Comment,"%s, aberrations corrected", Object_Comment);
// 			break;
// 		}
// 
// 	if(focus_Algorithm)
// 		{
// 		if(apodization_tx || apodization_rx) sprintf(Object_Comment,"%s Apodization.", Object_Comment);
// 		else sprintf(Object_Comment,"%s no Apodization.", Object_Comment);
// 		}
//	}

void	SyntheticFocusingOptions::ShowOptions()
{
	string comment = ssprintf("\nOriginal file name was '%s'", data_source->GetFileName().c_str());

	comment += ssprintf("\n\r%ld element array", data_source->n_tx_elements);
	comment += ssprintf(", %ld samples per waveform", data_source->samples_per_element);
	comment += ssprintf(" @ %.1f MHz", data_source->raw_signal_sample_rate.MHz());

	double	n_samps_per_mm_1_way = sample_rate.MHz() / data_source->sound_speed.mm_mksec();

	comment += ssprintf("\n\rDepth : %.2lf to %.2lf cm",
						0.5 * 0.1 * data_source->first_raw_sample/n_samps_per_mm_1_way,
						0.5 * 0.1 * (data_source->last_raw_sample)/n_samps_per_mm_1_way);
			//ShowText("Synthetic focusing options", comment);
}

XRAD_END
