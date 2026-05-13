#include "Pre.h"
#include "RFDataToSimIOImport.h"
#include "SimIOHeaders/SimIOHeaderProcessor.h"
#include "RFDataImport/S400_SpectromedRFDataImporter.h"
#include "RFDataImport/A4000RFDataImporter.h"

extern	char	this_process_comment[MAXSTRINGSIZE];

XRAD_BEGIN

RFDataToSimIOImport :: RFDataToSimIOImport()
	{
	if(Decide2("Choose RF source:", "A4000", "Sonomed", 0))
		importer = new SpectromedRFDataImporter;
	else
		importer = new A4000RFDataImporter;
	if(!importer->OpenRFData()) FatalError("File is not opened");
	int	frame_no = 0;
	do
		{
		if(importer->n_frames == 1)
			frame_no = 0;
		else
			frame_no = GetSigned("Frame no", frame_no+1, 1, importer->n_frames)-1;
			
		importer->GetFrame(frame_no);
		importer->SelectDepthWindow();
		importer->ApplyDepthWindow();
		importer->SelectFilter();
		importer->FilterFrame();
		
		downsample_ratio = GetSigned("Downsampling ratio",
			importer->AllowedDownsampleRatio(),
			1, 100);
		
		importer->DownsampleFrame(downsampled_data, downsample_ratio);
		DisplayMathFunction2D(downsampled_data, "Imported frame",
			//ScanFrameSector(importer->sp.pfd.scanning_trajectory_length, importer->sp.pfd.depth_range, importer->sp.pfd.start_angle, importer->sp.pfd.end_angle)
			importer->sp.pfd);
		}while(importer->n_frames == 1 ? false : YesOrNo("Read another frame?", false));

	
	strcpy(SIMIO::Process_Name,"RF importer");
	
// 	sprintf(Object_Comment, "Converted RF data set '%s', frame no %d of %d",
// 		importer->DataSourceName().c_str(),
// 		frame_no+1,
// 		importer->n_frames);
	
// 	_ftype = 'sbSw';
// 	_fcreator = 'usSw';

	sprintf(SIMIO::Original_RF_Data_File_Name, importer->DataSourceName().c_str());
	sprintf(SIMIO::Original_RF_Data_File_Date, "Unknown date");
	//sprintf(Original_RF_Data_Comment, "No comment");
	sprintf(SIMIO::Original_RF_Data_Comment, importer->GetComment().c_str());
	
	
	sprintf(SIMIO::Data_Species, SUBAPERT_DATA_SPECIES);
	sprintf(SIMIO::Data_Genesis, EXPT_GENESIS);
	sprintf(SIMIO::Process_Comment, "Converting RF data set");
	sprintf(SIMIO::Process_In_File_Name, importer->DataSourceName().c_str());
	sprintf(SIMIO::Process_Date_Time, "Unknown date");
	sprintf(this_process_comment, "Converting RF data ");

	
	n_samples = downsampled_data.hsize();

	n_rays = downsampled_data.vsize();
	
	if(importer->n_frames == 1)
		n_frames = 1;
	else
		n_frames = GetSigned("Frames/subapertures number", importer->n_frames, 1, importer->n_frames);
	
//	AllocateSectorData();
 	focused_data.realloc({n_frames, n_rays, n_samples}, complexF32(0));

	//SetAngleUnits(RADIANS);
//	SetFrequencyUnits(M_HERZ);
	//SetDepthUnits(CENTIMETRES);

	
	sound_speed = cm_sec(1.54e5);

	if(importer->sp.upo.ProbeType() == convex_2D_probe)
		{
		physical_angle	a0, a1;
		physical_length	r0, r1;	

		a0 = importer->sp.pfd.start_angle();
		a1 = importer->sp.pfd.end_angle();
//		r_min = importer->sp.upo.ScanningRadius() + importer->z0;
		r0 = importer->sp.upo.ScanningRadius() + importer->sp.calculate_min_depth();
//		r_max = importer->sp.upo.ScanningRadius() + importer->z0 + importer->sp.pfd.depth_range;
		r1 = importer->sp.upo.ScanningRadius() + importer->sp.calculate_max_depth();
		convexRadius = importer->sp.upo.ScanningRadius();

		SetFrameSector(r0*(a1-a0).radians(), (r1-r0), a0, a1);
		}
	else
		{
//		start_angle = radians(0);
//		end_angle = radians(0);
//		r_min = importer->sp.min_depth();
//		r_max = importer->sp.max_depth();
//		convexRadius = cm(0);
		
		SetFrameRectangle(importer->sp.pfd.scanning_trajectory_length(), importer->sp.pfd.depth_range());
		}
	
	sample_rate = importer->sp.sfd.sample_rate/downsample_ratio;

	omega0 = sample_rate/2;//importer->sp.carrier_frequency;
	band_half_width = omega0/2;
	

	
	//SetDepthUnits(CENTIMETRES);
	TX_Focus.realloc(1);
	RX_Focus.realloc(1);
//	TX_Focus.depth = RX_Focus.depth = cm(GetFloating("Focal point", importer->sp.mfo.focus_depths[0].cm(), 0, HUGE_VAL));
 	TX_Focus[0] = RX_Focus[0] = cm(GetFloating("Focal point", importer->sp.mfo[0].cm(), 0, HUGE_VAL));

//#pragma message ("следующие три строки нуждаютсЯ в серьезном уточнении")
	n_elements = importer->sp.upo.TotalElements();
#if 0
	n_subaperture_elements_active = importer->sp.upo.ApertureElements();
	n_subaperture_elements_full = importer->sp.upo.ApertureElements();//32;
#endif
	array_Pitch = importer->sp.upo.ElementSize();
	
	
//	n_elements = n_rays;
//	n_subaperture_elements_active = 64;//32;
//	n_subaperture_elements_full = 64;//32;
//	array_Pitch = importer->sp.pfd.scanning_trajectory_length/importer->sp.sfd.n_rays;
		
	printf("\n<<< %s >>>", this_process_comment);
	printf("\nOpened RF file name is '%s' created at %s",
		Signal_IP_File_Name.c_str() , SIMIO::Original_RF_Data_File_Date);
	printf("\nData comment : '%s'", SIMIO::Original_RF_Data_Comment);
	printf("\n%lu scanlines, %lu samples per waveform;", int(importer->sp.sfd.n_rays), int(importer->sp.sfd.n_samples));
	printf("\nDepth range from %.1f to %.1f cm;", r_min().cm(), r_max().cm());
	fflush(stdout);

	non_diffraction = YesOrNo("Don non-diffraction beam focusing?", true);
	if(non_diffraction) GetNonDiffractionParams();
	}

RFDataToSimIOImport	:: ~RFDataToSimIOImport()
	{
	}	
	
void	RFDataToSimIOImport :: InitWork()
	{
//	#pragma message ("то же и здесь, добавить размер буфера")

// 	const int fn_size = 256;
// 	char	fn[fn_size];
	string	fn = Signal_IP_File_Name;
	bool	index = false;
	
// 	strncpy(fn, Signal_IP_File_Name, fn_size-1);
// 	fn[fn_size-1] = 0;
	for(size_t i = 0; i < fn.size()-1; i ++) if(fn[i] == '.') fn[i] = 0, index = 1;
	
	if(index)SetOutputFileName(fn, "");
	else SetOutputFileName(fn, " (Converted)");
	
	ISignal_Write();
	fflush(stdout);
	}

void	RFDataToSimIOImport :: EndWork()
	{
	Write_Data();
	Display("Converted signal");
	}


void	RFDataToSimIOImport :: Batch()
	{
	GUIProgressBar	progress;
	progress.start("Exporting data", n_frames);
	for(size_t sub = 0; sub < n_frames; sub++)
		{
		ComplexFunction2D_F32 CurrentFrame;
		focused_data.GetSlice(CurrentFrame, {sub, slice_mask(0), slice_mask(1)});
		importer->GetFrame(sub);
		importer->ApplyDepthWindow();
		importer->FilterFrame();
		importer->DownsampleFrame(downsampled_data, downsample_ratio);

		if(non_diffraction) FocusNonDiffraction(downsampled_data, focusing_setup);

		CopyData(CurrentFrame, downsampled_data);
		++progress;
		}
	}


void	RFDataToSimIOImport :: GetNonDiffractionParams()
	{
	ComplexFunction2D_F32	untouched_signal(downsampled_data);
//	non_diffraction_setup s;

	//SetAngleUnits(RADIANS);
//	SetFrequencyUnits(M_HERZ);
	//SetDepthUnits(CENTIMETRES);

	focusing_setup.r_min = r_min() - convexRadius;
	focusing_setup.r_max = r_max() - convexRadius;
	focusing_setup.n_elements = n_rays;
//	focusing_setup.n_aperture_elements = n_subaperture_elements_active;
	focusing_setup.array_pitch = array_Pitch;
	focusing_setup.convex_radius = convexRadius;
//	focusing_setup.original_focus = TX_Focus.depth;
	focusing_setup.original_focuses = TX_Focus;
	focusing_setup.sample_rate = sample_rate; //MHz
	focusing_setup.sound_speed = cm_sec(1.54e5); //cm/sec;

	focusing_setup.tx_focusing = true;
	focusing_setup.rx_focusing = true;
	focusing_setup.zeta_factor = -1;
	focusing_setup.algorithm = fast_segment_algorithm;
//	focusing_setup.algorithm = slow_ft_algorithm;
	do
		{
		RX_Focus[0] = TX_Focus[0] = cm(GetFloating("Guess original focus", focusing_setup.original_focuses[0].cm(), 0, HUGE_VAL));
		focusing_setup.original_focuses = TX_Focus;
		focusing_setup.zeta_factor = GetFloating("Zeta factor is", focusing_setup.zeta_factor, -HUGE_VAL, HUGE_VAL);
		
		downsampled_data.CopyData(untouched_signal);
		FocusNonDiffraction(downsampled_data, focusing_setup);
		DisplayMathFunction2D(downsampled_data, "Focused",
			importer->sp.pfd
			//ScanFrameSector(importer->sp.pfd.scanning_trajectory_length, importer->sp.pfd.depth_range, importer->sp.pfd.start_angle, importer->sp.pfd.end_angle)
			);
		}while(!YesOrNo("Is focusing correct?", true));

	TX_Focus.realloc(0);
	TX_Focus.realloc(0);
	//	dynamical focusing
	}
	
XRAD_END
