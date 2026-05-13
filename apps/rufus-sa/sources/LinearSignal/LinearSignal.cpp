#include "pre.h"
#include <StatisticUtils.h>
#include "LinearSignal.h"
#include <MathFunctionTypes.h>
#include <S400_SpectromedRFDataImporter.h>

//--------------------------------------------------------
//
//	LinearSignalOptions
//
//---------------------------------------------------------



physical_speed	LinearSignalOptions :: soundSpeed = cm_sec(1.5e5); 	//cm/sec
physical_frequency	LinearSignalOptions :: sampleRate = MHz(15);		//MHz


double	LinearSignalOptions :: arrayQuotient = 1.5;


LinearSignalOptions :: LinearSignalOptions()
	{
	}


LinearSignalOptions :: ~LinearSignalOptions()
	{
	}

struct	log_cabs_functor
	{
	template<class T1, class T2>\
		T1 &operator()(T1 &x, const T2 &y) const {return x=T1(log(cabs(y)+1));}
	};

void LinearSignal::Display(char *title, double scalefactor)
	{
	if(n_frames > 1)
		{
		DisplayFrameSet(title, scalefactor);
		}
	else
		{
		DisplayFrame(0, title, scalefactor);
		}
	}

void LinearSignal::SetFrame(int frame_no)
	{
	data.GetSlice(*this, quick_iv(frame_no, slice_mask(0), slice_mask(1)));
	}

void	LinearSignalOptions :: WriteHeaderFile(const char *data_fle_name) const{
//	запись файла-заголовка, содержащего все параметры данных.
//	сами данные пишутся файле "data_file_name".
//	имя файла-заголовка "data_file_name.note"

	char	hdrFileName[256];
	sprintf(hdrFileName, "%s.note", data_fle_name);

	LinearSignalHeader gHdr;
	areaHeader	aHdr;

	gHdr.fileName = data_fle_name;
//	sprintf(gHdr.fileName, "%s", data_fle_name);
	gHdr.soundSpeed = soundSpeed.cm_sec();
// 	gHdr.omega0 = omega0.MHz();
// 	gHdr.halfWidth = halfWidth.MHz();
	gHdr.sampleRate = sampleRate.MHz();
	gHdr.nElements = n_array_elements;
	gHdr.nApertElements = n_aperture_elements;
	gHdr.arrayPitch = arrayPitch.cm();
	gHdr.TX_Focusing = TX_Focusing;
	gHdr.RX_Focusing = RX_Focusing;

	gHdr.n_frames = n_frames;
	gHdr.nRays = n_rays;
	gHdr.nSamples = n_samples;
	gHdr.rMin = rMin.cm();
	gHdr.rMax = rMax.cm();
	gHdr.TX_Focus = TX_Focus.cm();
	gHdr.RX_Focus = RX_Focus.cm();
	gHdr.convexRadius = convexRadius.cm();

	aHdr.firstRay = 0;
	aHdr.nRays = n_rays;
	aHdr.firstSample = 0;
	aHdr.nSamples = n_samples;

//	sprintf(aHdr.comment, "%s", comment);
	aHdr.comment = comment;

	FILE	*hdrFile = fopen(hdrFileName, "w");
	gHdr.print(stdout);
	aHdr.print(stdout);
	gHdr.print(hdrFile);
	aHdr.print(hdrFile);
	fclose(hdrFile);
	}

//--------------------------------------------------------
//
//	вспомогательная функция: автоматическое создание имени файла
//
//---------------------------------------------------------

char	import_filename[] = "c:\\temp\\cfm_0001";

//extern	char	import_filename[];
//	strcpy(import_filename, "c:\\temp\\cfm_0001");

namespace{
	bool	GenerateFileName(char *filename)
		{
		int	count = 0;
		char	result_file_name[256];
		FILE	*dummy_file = NULL;
		char	default_filename[256];
		bool	use_generic_filename = false;

		if(use_generic_filename)strcpy(default_filename, "c:\\CFM_dump\\cfm");
		else strcpy(default_filename, import_filename);

		sprintf(result_file_name, "%s", default_filename);
		dummy_file = fopen(result_file_name, "r");

		while(count < 1024 && dummy_file)
			{
			fclose(dummy_file);
			sprintf(result_file_name, "%s_%04d", default_filename, count++);
			//sprintf(result_file_name, "c:\\temp\\cfm_%04d", count++);
			dummy_file = fopen(result_file_name, "r");
			}

		if(!dummy_file)
			{
			strcpy(filename, result_file_name);
			return true;
			}
		else{
			fclose(dummy_file);
			return false;
			}
		}
	}//namespace



//--------------------------------------------------------
//
//	LinearSignal
//
//---------------------------------------------------------


PhysicalFrameDimensions LinearSignal::GetFrameDimensions(double scalefactor)
{
	PhysicalFrameDimensions	display_area;
	if(convexRadius.cm())
		{
		physical_angle scan_angle = radians(arrayPitch*n_rays/convexRadius);
		display_area = ScanFrameSector(rMin*scan_angle.radians(), rMax-rMin, -scan_angle/2, scan_angle/2);
		}
	else
		{
		display_area = ScanFrameRectangle(arrayPitch*n_array_elements, rMax-rMin);
		}
//	display_area.pixels_per_cm *= scalefactor;
	return display_area;
}


void	LinearSignal :: DisplayFrame(int frame_no, char *Title, double scalefactor)
	{
	ComplexFunction2D_F32	frame;
	data.GetSlice(frame, quick_iv(frame_no, slice_mask(0), slice_mask(1)));
	DisplayMathFunction2D(frame, Title, GetFrameDimensions(scalefactor));
	}

void	LinearSignal :: DisplayFrameSet(char *Title, double scalefactor)
	{	
	DisplayMathFunction3D(data, Title, GetFrameDimensions(scalefactor));
	}






void	LinearSignal :: Save()
	{
	try
		{
		string	data_fle_name;
		GetFileNameWrite(data_fle_name, "Save linear array signal");
		Save(data_fle_name, create_new_file);
		}
	catch(canceled_operation &){}
	}


void	LinearSignal :: Save (string file_name, signal_save_option op)
	{
	static	char	data_fle_name[512];
	static	char	dummy_header[0x147];
	FILE	*data_file;
	
	
	switch(op)
		{
		case continue_existing_file:
			data_file = fopen(data_fle_name, "ab+");
			if(data_file) break;
			else op = create_new_file;

		case create_new_file:
			if(!file_name.empty()) sprintf(data_fle_name, "%s", file_name.c_str());
			else /*if(!GenerateFileName(data_fle_name))*/ return;

			WriteHeaderFile(data_fle_name);
			data_file = fopen(data_fle_name, "ab+");
			break;
		default:
			throw logic_error("LinearSignal :: Save, Invalid file option");
		};
	
 	for(int i = 0; i < n_frames; ++i)
		{
		SetFrame(i);
		fwrite(dummy_header, 0x147, 1, data_file);
		for(int j = 0; j < n_rays; ++j)
			{
			fwrite_numbers(row(j), data_file, ioComplexF32_LE);
			}
		}
	fclose(data_file);
	}


void	LinearSignal :: Open()
	{
	LinearSignalHeader	hdr = data_file.OpenLC_File();
	
//	realloc(hdr.nRays, hdr.nSamples);

	soundSpeed = cm_sec(hdr.soundSpeed);

// 	omega0 = MHz(hdr.omega0);
// 	halfWidth = MHz(hdr.halfWidth);
	sampleRate = MHz(hdr.sampleRate);
	n_array_elements = hdr.nElements;
	n_aperture_elements = hdr.nApertElements;
	arrayPitch = cm(hdr.arrayPitch);
	TX_Focusing = hdr.TX_Focusing;
	RX_Focusing = hdr.RX_Focusing;

	n_rays = hdr.nRays;
	n_samples = hdr.nSamples;
	rMin = cm(hdr.rMin);
	rMax = cm(hdr.rMax);
	TX_Focus = cm(hdr.TX_Focus);
	RX_Focus = cm(hdr.RX_Focus);
	convexRadius = cm(hdr.convexRadius);

	nRepeats = n_rays/n_array_elements;
//	comment = hdr.comment;
	
	int	first_frame = 1;
	int	last_frame = hdr.n_frames;
	
	first_frame = GetLong("First frame to read", first_frame, 1, hdr.n_frames);
	last_frame = GetLong("Last frame to read", last_frame, first_frame, hdr.n_frames);
	
	n_frames = last_frame-first_frame + 1;
	data.realloc(quick_iv(n_frames, n_rays, n_samples));
	
	for(int i = first_frame-1; i < last_frame; ++i)
		{
		SetFrame(i-(first_frame-1));
		data_file.GetFrame(*this,i);
		}
	}


void	LinearSignal :: OpenSpectromed()
	{
	SpectromedRFDataImporter	sm_data;
	double	downsample_ratio = 10;
	ComplexFunction2D	downsampled_data;
	if(!sm_data.OpenRFData()) return;

	int	frame_no(0);

	do
		{
		frame_no = GetLong("Frame no", frame_no+1, 1, sm_data.n_frames)-1;
		sm_data.GetFrame(frame_no);
		sm_data.SelectDepthWindow();
		sm_data.ApplyDepthWindow();
		sm_data.SelectFilter();
		sm_data.FilterFrame();

		downsample_ratio = 10;//sm_data.sample_rate/(2*sm_data.static_filter.filter_f0);

		sm_data.DownsampleFrame(downsampled_data, downsample_ratio);
		
		DisplayMathFunction2D(downsampled_data, "Loaded data", sm_data.sp.pfd);
		}while(YesOrNo("Select another frame?", YES));

	comment = sm_data.GetComment();

	soundSpeed = cm_sec(1.54e5);

// 	omega0 = sm_data.static_filter.filter_f0;
// 	halfWidth = sm_data.static_filter.filter_bandwidth/2;

	sampleRate = sm_data.sp.sfd.sample_rate/downsample_ratio;
// 	omega0 = sampleRate/2;
// 	halfWidth = sampleRate/4;

	n_array_elements = sm_data.sp.sfd.n_rays;
	n_rays = sm_data.sp.sfd.n_rays;
	n_aperture_elements = 32;

	if(sm_data.sp.upo.ProbeType() == linear_2D_probe)
		{
// 		arrayPitch = sm_data.sp.pfd.width/sm_data.n_rays;
		arrayPitch = sm_data.sp.upo.ElementSize();
		convexRadius = cm(0);
		}
	else if(sm_data.sp.upo.ProbeType() == convex_2D_probe)
		{
		arrayPitch = sm_data.sp.upo.ElementSize();
// 		arrayPitch = sm_data.radius*sm_data.angle/sm_data.n_rays;
		convexRadius = sm_data.sp.upo.ScanningRadius();
		}
	else FatalError("Invalid array type");


	TX_Focusing = true;
	RX_Focusing = true;


	n_samples = downsampled_data.hsize();
	rMin = sm_data.sp.pfd.r_min();
//	rMax = sm_data.z0 + sm_data.z_range;
	rMax = rMin + sm_data.sp.pfd.depth_range();
	TX_Focus = cm(GetDouble("Focus distance", sm_data.sp.mfo[0].cm(), -inf(), inf()));
//	TX_Focus = GetDouble("Focus distance", (rMax+rMin)/2, rMin, rMax);
	RX_Focus = TX_Focus;

	nRepeats = 1;
	
	n_frames = sm_data.n_frames;
	
	data.realloc(quick_iv(n_frames, n_rays, n_samples));

	StartProgress("Converting frames", n_frames, 2);
	for(int i = 0; i < n_frames; ++i)
		{
		sm_data.GetFrame(i);
		sm_data.ApplyDepthWindow();
		sm_data.FilterFrame();

		sm_data.DownsampleFrame(downsampled_data, downsample_ratio);

		SetFrame(i);

		CopyData(downsampled_data);
		NextProgress();
		}
	EndProgress();
	}

