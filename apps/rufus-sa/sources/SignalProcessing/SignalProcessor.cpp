#include "Pre.h"
#include "SignalProcessor.h"
//#include "NonUniformMotionAnalysis.h"

XRAD_BEGIN


SignalProcessor :: SignalProcessor()
	{
	ISignal_Read();
	ShowDataFileInfo();
	}

SignalProcessor :: ~SignalProcessor()
	{
	}

void	SignalProcessor :: ShowDataFileInfo()
	{
	printf("\n\r'%s' created by process '%s' on %s", 
		Signal_IP_File_Name.c_str(), SIMIO::Process_Name,
		SIMIO::Process_Date_Time);
	printf("\n\r Original file name was '%s' created on '%s'\n\r'%s'", 
		SIMIO::Original_RF_Data_File_Name,
		SIMIO::Original_RF_Data_File_Date,
		SIMIO::Original_RF_Data_Comment);
	printf("\n\r'%s'", SIMIO::Process_Comment);
	printf("\n\rData genesis : %s", SIMIO::Data_Genesis);

	printf("\n\r%ld subapertures focused to", int(n_frames));
	//SetAngleUnits(DEGREES);
	printf(" a sector between %.1f", start_angle().degrees());
	printf(" and %.1f degrees,", end_angle().degrees());
	//SetAngleUnits(RADIANS);
	printf(" %ld rays", int(n_rays));
	printf(", %ld samples per waveform", int(n_samples));
//	SetFrequencyUnits(M_HERZ);
	printf(" @ %.1f MHz", sample_rate.MHz());
	printf("\nCarrier frequency is %.1f MHz,", omega0.MHz());
	printf(" Band width is %.1f MHz; ", 2. * band_half_width.MHz());
//	SetFrequencyUnits(BACK_SECONDS);
//	printf("\nSubaperture contains %ld elements", int(n_subaperture_elements_active));
	printf("\n\rDepth : %.2lf to %.2lf cm", 
				r_min().cm(),  r_max().cm());
	printf("\n\n\r");
	fflush(stdout);
	}

size_t Task = 2;
	
void	SignalProcessor :: InitWork()
	{
	}

void	SignalProcessor :: Batch(){}

void	SignalProcessor :: EndWork()
	{
	if(Task == 2) Display("Processed");
	}
	
void	SignalProcessor :: Display(const char *Title)
	{
	size_t answer = 0;
	
	do
		{
// 		if(focus_Algorithm == CORRECT_ABERRATION)
// 			{
// 			answer = GetButtonDecision(Title, //3,
// 			{
// 					"Signals",
// 					"Aberrations",
// 					"Exit display"
// 			});
// 			switch(answer)
// 				{
// 				case 0:	SectorData :: Display(Title);
// 					break;
// 				case 1:	//Aberrations_Map -> Display("Aberrations");
// 					break;
// 				}
// 			}	
// 
// 		else
			{
			SectorData :: Display(Title);
			answer = 2;
			}
		}while(answer != 2);
	}


//------------------------------------------------------------------------------
//
//	стабилизациЯ
//
//------------------------------------------------------------------------------


namespace{

//	вспомогательнаЯ утилита отображениЯ

double	abs_f(double x){return fabs(x);}
double	abs_f(complexF32 x){return cabs(x);}

template<class data_t>
void	DisplayUtil(GrayScanConverter &SC, const data_t &in_data, bool log_compress, const char *title)
	{
	SC.fill(0);
	for(size_t i = 0; i < SC.vsize(); i++)
		{
		for(size_t j = 0; j < SC.hsize(); j++)
			{
//			for(size_t sub = 0; sub < n_frames; sub ++)
				{
				SC.at(i,j) += abs_f(in_data.at(i,j));
				}
			}
		}
	if(log_compress) LogCompress(SC);	
	CutHistogramEdges(SC, range1_F64(0.001, 0.999));
	NormalizeImage(SC, 0,255);
	DisplayMathFunction2D(SC.GetConvertedImage(), title);
	}


} // namespace


void	StabilizeSignalComponents(SignalProcessor &sp)
	{
	size_t n_frames = sp.n_frames;
	size_t n_rays = sp.n_rays;
	size_t n_samples = sp.n_samples;
	
	
	DataArray<ComplexFunction2D_F32> process_buffer;
	GrayScanConverter	sample_brightness;	
	sp.ExportScanConverterOptions(sample_brightness);
	sample_brightness.SetFlip(true);
	sample_brightness.InitScanConverter(362);
	sample_brightness.realloc(n_rays, n_samples);
//	sample_brightness.SetImageTitle("stabilization_details.pct");

	process_buffer.realloc(n_frames);
	for(size_t sub = 0; sub < n_frames; sub++)
		{
		process_buffer[sub].realloc(n_rays, n_samples);
//		sp.SetCurrentFrame(sub);
		for(size_t ray = 0; ray < n_rays; ray++)
			{
			ComplexFunctionF32	CurrentRay;
			sp.focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
//			sp.SetCurrentRay(ray);
			process_buffer[sub].row(ray).CopyData(CurrentRay);
			}
		}
	

	DisplayUtil(sample_brightness, process_buffer[0], true, "subapert 0");	
	bool	display_intermediate_results = YesOrNo("Display intermediate results?", false);
	
	GUIProgressBar	progress;
	progress.start("Stabilizing subapertures", n_frames-1);
	RealFunction2D_F32	total_motion_correction(n_rays, n_samples);
	for(size_t sub = 1; sub < n_frames; ++sub)
		{
		ComplexFunction2D_F32	current_motion_correction(n_rays, n_samples, complexF32(0));// неактуально
// 		EvaluateCorrelationShift(
// 			process_buffer[sub],
// 			process_buffer[0],
// 			current_motion_correction);		

		if(display_intermediate_results) DisplayUtil(sample_brightness, process_buffer[sub], true, "before correction");

		//	коррекцию сдвигов делать в другом месте: при выборе интервалов длЯ свертки с функциЯми Љ-‹
// 		ApplyCorrelationShift(
// 			process_buffer[sub],
// 			current_motion_correction);
			
		if(display_intermediate_results)
			{
			DisplayUtil(sample_brightness, process_buffer[sub], true, "after correction");
			DisplayUtil(sample_brightness, current_motion_correction, false, "motion_correction");
			}
		for(size_t i = 0; i < n_rays; ++i)
			{
			for(size_t j = 0; j < n_samples; ++j)
				{
				total_motion_correction.at(i,j) += cabs(current_motion_correction.at(i,j));
				}
			}
		++progress;
		}
	progress.end();
	DisplayUtil(sample_brightness, total_motion_correction, false, "total_motion_correction");

	sample_brightness.fill(0);
	for(size_t i = 0; i < n_samples; i++)
		{
		for(size_t j = 0; j < n_rays; j++)
			{
			for(size_t sub = 0; sub < n_frames; sub ++)
				{
				sample_brightness.at(j,i) += cabs(process_buffer[sub].at(j,i));
				}
			}
		}

	LogCompress(sample_brightness);
	CutHistogramEdges(sample_brightness, range1_F64(0.001, 0.999));
	NormalizeImage(sample_brightness, 0,255);
	DisplayMathFunction2D(sample_brightness.GetConvertedImage(), "Accumulated B-image after stabilization");
	}
	
	
XRAD_END
