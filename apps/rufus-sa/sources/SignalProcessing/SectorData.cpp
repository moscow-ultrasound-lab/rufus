#include "pre.h"
#include <string.h>
#include "SectorData.h"
#include "SimIOHeaders/SimIOHeaderProcessor.h"
#include "SimIOHeaders/SimIO.h"
#include <XRADBasic/DataArrayIO.h>

//-------------------------------------------------------------
//	вспомогательный класс ввода-вывода, на деле, старая 
//	неуклюжая конструкция, собираю вместе, чтобы потом
//	уничтожить
//-------------------------------------------------------------
XRAD_BEGIN

using namespace DataArrayIOAuxiliaries;

SectorDataIO_temp::SectorDataIO_temp()
{
	Signal_IP_File_Name = "none";
	Signal_OP_File_Name = "none";
	Signal_IP_File = NULL;
	Signal_OP_File = NULL;
}

SectorDataIO_temp :: ~SectorDataIO_temp()
{
	if(Signal_IP_File)
		SimClose(Signal_IP_File), Signal_IP_File = NULL;
	if(Signal_OP_File)
		SimClose(Signal_OP_File), Signal_OP_File = NULL;
}


void	SectorDataIO_temp::SetOutputFileName(string name_base, string name_appendix)
{
	DefaultOPFileName = name_base;
	for(int i = int(name_base.length()-1); i >=0; --i)
	{
		if(DefaultOPFileName[i] == '/' || DefaultOPFileName[i] == '\\') DefaultOPFileName[i] = 0, i=-1;
		else if(DefaultOPFileName[i] == '.') DefaultOPFileName[i] = 0, i=-1;
	}
	DefaultOPFileName += name_appendix;
}

void	SectorData::ISignal_Read()
{
	static	char	message[256], error_message[256];
	static	int	num_params_in_Generic_Data_Descriptor;

	printf("\n\r<<< Synthetic Focus RF data, Complex subapertures signal >>>\n\n\r");
	fflush(stdout);
	SimErrorMode(RETURN, stderr);


	Signal_IP_File_Name = GetFileNameRead("Subapertures signal", "*.sbsw");

	XRAD_ASSERT_THROW_M(SimOpen(Signal_IP_File_Name.c_str(), "rb", &Signal_IP_File) == SUCCESS, runtime_error, ssprintf(error_message, "Error opening input file  (%s)", Signal_IP_File_Name.c_str()));
	SimCheckSumCheckEnable(true);		// always check the checksum initially
	XRAD_ASSERT_THROW_M(SimSkipHistory(Signal_IP_File) == SUCCESS, invalid_argument, "This is not a SimIO style file");

//	read the standard "Generic_Data_Descriptor" record.   Only read some of the 
//	parameters to begin with to establish how many data-species/descriptor-version 
//	descriptor records there are.   Then read info regarding the number of keyword 
//	phrases (param structures) there are in each of these descriptor records.

	SimGetDescriptor(GENERIC_DATA_DESCRIPTOR_BASE_NUM_PARAMS,
					 SIMIO::Generic_Data_Descriptor,
					 Signal_IP_File);

	num_params_in_Generic_Data_Descriptor = GENERIC_DATA_DESCRIPTOR_BASE_NUM_PARAMS +
		2*SIMIO::Num_Xtra_Descriptors;

	SimGetDescriptor(num_params_in_Generic_Data_Descriptor,
					 SIMIO::Generic_Data_Descriptor,
					 Signal_IP_File);

	XRAD_ASSERT_THROW_M(!strcmp(SIMIO::Data_Species, SUBAPERT_DATA_SPECIES), invalid_argument, ssprintf("Invalid data species: %s", SIMIO::Data_Species));

	XRAD_ASSERT_THROW_M(SimSkipData(Signal_IP_File) == SUCCESS, runtime_error, "Problem skipping Generic_Data_Descriptor data");

	LoadSIMIO(Signal_IP_File);
//	sprintf(Object_Comment, "Work with pre-computed data from the file '%s'", Signal_IP_File_Name.c_str());
	focused_data.realloc({n_frames, n_rays, n_samples}, complexF32(0));

//	AllocateSectorData();
	SetOutputFileName(Signal_IP_File_Name, " (processed)");


	// ниже идет перенос куска из Start_Signal_IO;

	SimErrorMode(RETURN, stderr);	// enable automatic error handling
	SimSkipHistory(Signal_IP_File);	// rewind to 1st descriptor record
	SimSkipData(Signal_IP_File);	// skip Generic_Data_Descriptor record
	SimSkipData(Signal_IP_File);	// skip Subaperts_Data_Descriptor record
	SimCheckSumCheckEnable(false);	// turn off since takes too long !

	Read_Data();
}

string	extension(const string &filename)
{
	auto	it = filename.rbegin(), ie = filename.rend();
	size_t	extension_length = std::find(it, ie, '.') - it;
	if(extension_length == filename.size()) return string("");
	else return string(filename.end()-extension_length, filename.end());
}

void	SectorData::ISignal_Write()
{
	string	File_Name;

	try
	{
		string	ext = "sbsw";
		string	ext_filter = "*." + ext;
		GUIValue<string>	default_filename = (string(DefaultOPFileName) + "." + ext).c_str();
		Signal_OP_File_Name = GetFileNameWrite("Result file name", default_filename, ext_filter);
		if(extension(Signal_OP_File_Name) != ext) Signal_OP_File_Name += ("."+ext);
	}
	catch(canceled_operation &)
	{
		Signal_OP_File_Name = "SFTempFile.sbsw";
	}

	File_Name = Signal_OP_File_Name;

	if(SimOpen(File_Name.c_str(), "w+b", &Signal_OP_File) != SUCCESS)
	{
		FatalError(ssprintf("Error opening output file (%s)", File_Name.c_str()));
	}

	printf("\nOpened output file : %s", Signal_OP_File_Name.c_str());
	fflush(stdout);

	SimIOHeaderProcessor sim_header;

	if(Signal_IP_File)
	{
		SimCopyHistory(1, &Signal_IP_File, 1, &Signal_OP_File); // rewind to 1st descriptor record
	}

	sim_header.Attach(this);
	sim_header.AppendHistoryRecord(Signal_OP_File, Signal_IP_File_Name.c_str(), Signal_OP_File_Name.c_str());
	sim_header.WriteGenericDataDescriptor(Signal_OP_File);
	DumpSIMIO(Signal_OP_File);
	//Zero_Data();
	focused_data.fill(complexF32(0));

//	SetCurrentRay(0);
}



//-------------------------------------------------------------
//
//-------------------------------------------------------------



void	SectorData::ProcessInitDialog()
{
}

SectorData::SectorData()
{
	SetOutputFileName("Processing result", "");
}

SectorData :: ~SectorData()
{
}



bool	SectorData::GetSubaperturesMixParams(FramesCombineParams &p)
{
	size_t answer;

	if(n_frames > 1) answer = GetButtonDecision("Subapertures mix function",
			{
			"Coherent add",
			"Non-coherent add",
			"Single subaperture",
			"Single quantile",
			"None"
			}
	);
	else answer = 1;


	if(answer == 4)
		return false;
	else p.alg = FramesCombineAlgorithm(answer);

	if(p.alg == single_subaperture)
		p.frame = GetSigned("Subaperture to display", p.frame, 0, n_frames-1);

	if(p.alg == single_quantile)
		p.quantile = GetSigned("Quantile to display", p.quantile, 0, n_frames-1);

	return true;
}

string	SectorData::GenerateImageTitle(const FramesCombineParams &p)
{
	switch(p.alg)
	{
		case coherent_add:
			return string("Detected signal (coherent sum)");
			break;
		case non_coherent_add:
			return string("Detected signal (non-coherent sum)");
			break;
		case single_subaperture:
			return ssprintf("Detected signal (subaperture # %d)", p.frame);
			break;
		case single_quantile:
			return ssprintf("Detected signal (quantile # %d)", p.quantile);
		default:
			throw logic_error("SectorData:: GenerateImageTitle. Unknown subapertures mix algorithm");
	}
}

void	SectorData::Display(const char *Title)
{
	size_t	answer =1;
	size_t	displaySubapert = 0, displayRow = n_samples/2;
	size_t	displayRay = n_rays/2;

	do
	{
		answer = GetButtonDecision(
			Title,
			{
			"Detected signal",
			"Complex ray",
			"Complex row",
			"Animate subapertures",
			"Animate quantiles",
			"Exit display"
			}
		);
		switch(answer)
		{
			case 0:
			{
				static FramesCombineParams smp;
				bool	build_picture = GetSubaperturesMixParams(smp);

				if(build_picture)
				{
					GrayScanConverter picture;
					ComputeDetectedSignal(picture, smp);

					DisplayMathFunction2D(picture, GenerateImageTitle(smp).c_str(), picture);
				}
			}
			break;
			case 1:
//				SetCurrentRay(displayRay);
				do
				{
					if(OptionPressed())
					{
					//SetAngleUnits(DEGREES);
						float	dAngle = angle_range().degrees()/n_rays;
						displayRay = (GetFloating("Display ray at angle",
									  start_angle().degrees() + displayRay*dAngle, start_angle().degrees(), end_angle().degrees()) - start_angle().degrees())/dAngle;
						printf("Ray # = %d\n", int(displayRay));
						displayRay = range(displayRay, 0, n_rays-1);
					}
					else displayRay = GetSigned("ray No:", displayRay + 1, 1, n_rays) - 1;
//					SetCurrentRay(displayRay);
					//SetDepthUnits(CENTIMETRES);

					if(n_frames > 1)
					{
						displaySubapert = GetSigned("Subaperture No:", displaySubapert + 1, 1, n_frames) - 1;
//						SetCurrentFrame(displaySubapert);
					}
					ComplexFunctionF32	CurrentRay;
					focused_data.GetRow(CurrentRay, {displaySubapert, displayRay, slice_mask(0)});
					DisplayMathFunction(CurrentRay, r_min().cm(), depth_range().cm()/(n_samples), "Focused data");
				} while(YesOrNo("Display another ray?", true));
				break;
			case 2:
//				SetCurrentSampleRow(displayRow);
				do
				{
					if(OptionPressed())
					{
					//SetDepthUnits(CENTIMETRES);
						float	dZ = depth_range().cm()/n_samples;
						displayRow = (GetFloating("Display row at depth:",
									  r_min().cm() + displayRow*dZ, r_min().cm(), r_max().cm()) - r_min().cm())/dZ;
						displayRow = range(displayRow, 0, n_samples-1);
						printf("Row # = %d\n", int(displayRow));
					}
					else displayRow = GetSigned("Row No:", displayRow + 1, 1, n_samples)-1;
//					SetCurrentSampleRow(displayRow);
					if(n_frames > 1)
					{
						displaySubapert = GetSigned("Subaperture No:", displaySubapert + 1, 1, n_frames) - 1;
//						SetCurrentFrame(displaySubapert);
					}
					
					ComplexFunctionF32	row;
					focused_data.GetRow(row, {size_t(displaySubapert), slice_mask(0), size_t(displayRow)});

					DisplayMathFunction(row, start_angle().degrees(), angle_range().degrees()/n_rays, "Row signal");
				} while(YesOrNo("Display another row?", true) && n_rays);
				break;
			case 4:
			case 3:
			{
				static bool	draw_grid = false;
				static bool	flip_image = false;

				FramesCombineParams smp;
				string	title;
				if(answer == 3)
				{
					title = string("Subapertures animation");
					smp.alg = single_subaperture;
				}
				else
				{
					title = string("Quantiles animation");
					smp.alg = single_quantile;
				}

				DataArrayMD<RealFunction2D_UI8>	pointers;
//				DataArray<RealFunction2D_UI8>	pointers(n_frames);
				//DataArray<GrayPixel*>	pointers(n_frames);

				GrayScanConverter frame;
				GrayPixelScanConverter	SC;

				GetCheckboxDecision("Scan converter options", //2,
									{
									"Draw grid",
									"Flip image"
									},
									{
									&draw_grid,
									&flip_image
									}
				);

								  // последующие вызовы только для нахождения правильных размеров
								  // детектированного кадра, вычисленные данные оказываются не нужны.
								  // не совсем хорошо, но лучше не придумал.

				smp.frame = smp.quantile = 0;
				ComputeDetectedSignal(frame, smp);
				NormalizeImage(frame, 0, 255);
				MakeCopy(SC, frame);

				SC.CopyScanConverterOptions(frame);
				SC.SetBackground(80);
				SC.SetGrid(draw_grid, 160, cm(2));
				SC.SetFlip(flip_image);

//				SC.SetImageTitle(Title);
				SC.InitScanConverter(512);

				GUIProgressBar	progress;
				progress.start("Creating animation", n_frames);
				for(size_t i = 0; i < n_frames; ++i)
				{
					smp.frame = i;
					smp.quantile = i;
					ComputeDetectedSignal(frame, smp);

//					#pragma message ("here inversion, check!")	
					NormalizeImage(frame, 0, 255);
					CopyData(SC, frame);
					SC.BuildConvertedImage();
					
					auto	&image = SC.GetConvertedImage();
					if(i==0)
					{
						pointers.realloc({n_frames, image.vsize(), image.hsize()});
					}
					pointers.slice({i, slice_mask(0), slice_mask(1)}).CopyData(image);
//					pointers[i] = &frames[i].at(0,0);
					++progress;
				}
				progress.end();

				DisplayMathFunction3D(pointers, title.c_str(), SC);
// 				DisplayRasterAnimation(title.c_str(), (const uint8_t**)&pointers[0], n_frames, frames[0].vsize(), frames[0].hsize(),
// 									   axis_legend(0, 1, "z"), axis_legend(0, 1, "y"), axis_legend(0, 1, "y"), value_legend(0, 1, 1, "brightness"));
			}
			break;

			case 5:	break;
		}
	} while(answer != 5);
}




void	SectorData::ComputeDetectedSignal(GrayScanConverter &result, const FramesCombineParams &smp)
{
	size_t	samples_compress = min(n_samples, size_t(512));
	float	ratio = float(n_samples)/samples_compress;

	result.realloc(n_rays, samples_compress);
	ExportScanConverterOptions(result);
//	result.pixels_per_cm = 60;
	result.InitScanConverter(samples_compress);

	ComplexFunction2D_F32	slice;

	for(size_t ray = 0; ray < n_rays; ++ray)
	{
		focused_data.GetSlice(slice, {slice_mask(0), ray, slice_mask(1)});


		for(size_t sample_comp = 0; sample_comp < samples_compress; ++sample_comp)
		{
			complexF32	accumulator(0, 0);
			float	depth_window_factor = 1;	// степень размытия по глубине
			size_t	s0 = max(int((float(sample_comp) - depth_window_factor)*ratio), 0);
			size_t	s1 = min(size_t(s0+2*depth_window_factor*ratio), n_samples);

			for(size_t sample = s0; sample < s1; ++sample)
			{
				complexF32	sub_accumulator(0);

				if(smp.alg == single_subaperture)
				{
					sub_accumulator = cabs(slice.at(smp.frame, sample));
				}

				else if(smp.alg == single_quantile)
				{
					ComplexFunctionF32	filter(slice.col(sample));
					ComplexFunctionF32::iterator f0 = filter.begin();
					ComplexFunctionF32::iterator f1 = f0 + smp.quantile;
					ComplexFunctionF32::iterator fe = filter.end();

					nth_element(f0, f1, fe);

					sub_accumulator = *f1;
				}

				else for(size_t subaperture = 0; subaperture < n_frames; ++subaperture)
				{
				//SetCurrentFrame(subaperture);
					switch(smp.alg)
					{
						default:
						case coherent_add:	sub_accumulator += slice.at(subaperture, sample);//CurrentRay[sample];
							// между субапертурами возможно когерентное сложение
							break;
						case non_coherent_add:	sub_accumulator += cabs(slice.at(subaperture, sample));
							break;
					};
				}
				accumulator += cabs(sub_accumulator);
				// а между элементами интервала накопления -- только некогерентное
			}
			result.row(ray)[sample_comp] = cabs(accumulator);
		}
	}
}



/*
dependences

ISignal_Read -- не вызывает ничего
	вызывается из
	SectorData::InitWork
	SignalProcessor::InitWork



ISignal_Write -- не вызывает ничего
	вызывается
	RFDataToSimIOImport::InitWork
	CurvePathfinder::InitWork
	ExactPathfinder::InitWork
	NLPathfinder::InitWork
	Pathfinder::InitWork
	ZPathfinder::InitWork

	Holland::InitWork

	Correlator::Batch()
	MediaSimulator::InitWork
	Focuser::InitWork
	SpeckleSuppressor::InitWork


Read_Data -- не вызывает ничего,
	вызывается из Start_Signal_IO при io_status xxRead

Write_Data -- не вызывает ничего,
	вызывается из End_Signal_Io при io_status xxwrite
*/



void	SectorData::InitWork()
{
	ISignal_Read();
	fflush(stdout);
}

void	SectorData::EndWork()
{
	Display("The signal");
}

void	SectorData::Batch(){}

void	SectorData::Work()
{
	InitWork();
	Batch();
	EndWork();
}


//----------------------------------------
//
//	focused_data operations
//
//----------------------------------------


void	SectorData::Read_Data()
{
	SimGetDescriptor(NUM_PARAMS_IN_RAY_GROUP_DESCRIPTOR,
					 SIMIO_sub::Ray_Group_Descriptor,
					 Signal_IP_File);

	XRAD_ASSERT_THROW_M(size_t(SIMIO::Data_Count) == n_frames*n_rays*n_samples, 
				 invalid_argument, ssprintf("Invalid data count: %d*%d*%d != %d", int(n_frames), int(n_rays), int(n_samples), int(SIMIO::Data_Count)));

	XRAD_ASSERT_THROW_M(SIMIO::Datum_Size == int(complexF32_be_iotype::fsize()), 
				 invalid_argument, ssprintf("Invalid datum size (%d != %d)", int(SIMIO::Datum_Size), int(sizeof(complexF32))));

	DataArray<complexF32>  io_buffer(SIMIO::Data_Count);
	complexF32 *b = &io_buffer[0];

	XRAD_ASSERT_THROW_M(SimGetData(SIMIO::Data_Count * SIMIO::Datum_Size, b, Signal_IP_File) == SUCCESS, 
				 runtime_error, "Can't read rays group");

	int	n = 0;
	for(size_t j = 0; j < n_rays; ++j)
	{
		for(size_t i = 0; i < n_frames; ++i)
		{
			ComplexFunctionF32	CurrentRay;
			focused_data.GetRow(CurrentRay, {i, j, slice_mask(0)});
//			SetCurrentRay(j);
//			SetCurrentFrame(i);
			for(size_t k = 0; k < n_samples; ++k)
			{
//				CurrentRay[k] = macComplexF::get((char*)(b+n));
				CurrentRay[k] = complexF32_be_iotype::get((uint8_t*)(b+n));
				++n;
			}
		}
	}
}


// void	SectorData::SetCurrentRay(size_t r)
// {
// 	current_indices[ray_no_index] = r;
// 
// 	// CurrentRay -- все отсчеты для заданных ray, subapert
// 	SetSubsetIndices(sample_no_index);
// //	focused_data.GetRow(CurrentRay, subset_indices);
// 
// 	// CurrentRay -- все субапертуры для заданных ray, sample
// 	SetSubsetIndices(frame_no_index);
// //	focused_data.GetRow(CurrentCrossFramesRow, subset_indices);
// }

// void	SectorData::SetCurrentSampleRow(size_t r)
// {
// 	current_indices[sample_no_index] = r;
// 
// 	// CurrentCrossRaysRow -- все лучи для заданных sample, subapert
// 	SetSubsetIndices(ray_no_index);
// //	focused_data.GetRow(CurrentCrossRaysRow, subset_indices);
// 
// 	// CurrentRay -- все субапертуры для заданных ray, sample
// 	SetSubsetIndices(frame_no_index);
// //	focused_data.GetRow(CurrentCrossFramesRow, subset_indices);
// }

// void	SectorData::SetCurrentFrame(size_t s)
// {
// 
// 	current_indices[frame_no_index] = s;
// 
// 	// CurrentRay -- все отсчеты для заданных ray, subapert
// 	SetSubsetIndices(sample_no_index);
// //	focused_data.GetRow(CurrentRay, subset_indices);
// 
// 	// CurrentCrossRaysRow -- все лучи для заданных sample, subapert
// 	SetSubsetIndices(ray_no_index);
// //	focused_data.GetRow(CurrentCrossRaysRow, subset_indices);
// 
// 	SetSubsetIndices(ray_no_index, sample_no_index);
// //	focused_data.GetSlice(CurrentFrame, subset_indices);
// }





//-----------------------------------------------------------	


void	SectorData::Append_Data(){}

physical_angle	SectorData::CentreAngle()
{
	physical_angle	dAngle = angle_range()/n_rays;
	return	start_angle() + dAngle*n_rays/2.;
}

physical_angle	SectorData::CurrentRayAngle(size_t ray)
{
	physical_angle	dAngle = angle_range()/n_rays;
//	return (start_angle() + dAngle * current_indices[ray_no_index]);
	return (start_angle() + dAngle * ray);
}

bool	SectorData::Write_Data()
{
	if(Signal_OP_File==nullptr) return false;

	SIMIO::Data_Count = long(n_rays * n_frames * n_samples);
//	SIMIO::Datum_Size = sizeof(complexF32); // @@@ int(complexF32_be_iotype::fsize())
	SIMIO::Datum_Size = int(complexF32_be_iotype::fsize());
	int OP_Data_Size = SIMIO::Data_Count * SIMIO::Datum_Size;

	strcpy(SIMIO::Descriptor_Identifier, RAY_GROUP_DESCRIPTOR);
	SIMIO_sub::_from_Ray = 0;

//OK

	fseek(Signal_OP_File, 0, SEEK_END);
	SimPutDescriptor(NUM_PARAMS_IN_RAY_GROUP_DESCRIPTOR,
					 SIMIO_sub::Ray_Group_Descriptor,
					 Signal_OP_File);

	 //	complexF32 *b = io_buffer.Needed(SIMIO::Data_Count);
	DataArray<complexF32>  io_buffer(SIMIO::Data_Count);
	complexF32 *b = &io_buffer[0];
//bad
	int	n = 0;
	for(size_t j = 0; j < n_rays; ++j)
	{
//		SetCurrentRay(j);
		for(size_t i = 0; i < n_frames; ++i)
		{
//			SetCurrentFrame(i);
			ComplexFunctionF32	CurrentRay;
			focused_data.GetRow(CurrentRay, {i, j, slice_mask(0)});
			for(size_t k = 0; k < n_samples; ++k)
			{
			//CurrentRay[k] = macComplexF::get((const char*)&b[i]);
				complexF32_be_iotype::put((uint8_t*)(b+n), CurrentRay[k]);
// 				macComplexF::put((char*)(b+n), CurrentRay[k]);
				++n;
			}
		}
	}
	Pause();
	SimPutData(OP_Data_Size, b, Signal_OP_File);

//	io_buffer.NotNeeded();
	return true;
}

XRAD_END
