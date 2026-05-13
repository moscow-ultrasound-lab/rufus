#include "pre.h"
#include <ComplexFunction2D.h>
#include <DataIO.h>
#include <QMFilesize.h>
#include "LinearConvexHeader.h"
#include "LinearConvexFile.h"
//#include "spectromed_cfm_header.h"

void	OpenLC_File_NO_Header(ComplexFunction2D_F32 &theMatrix)
	{
	string	fileName;
	GetFileNameRead(fileName, "Open header file");

	long	i;
	
	long	dataCount;

	FILE	*theFile = fopen(fileName.c_str(), "rb");
	fseek(theFile, 0, SEEK_END);

	dataCount = ftell(theFile);
	fseek(theFile, 0x147, SEEK_SET);

	dataCount -= 0x147;
	dataCount /= sizeof(complexF32);
	
	
	long	hs = GetLong("Total N samples:", 8192, 1, dataCount);
	long	frameH0 = GetLong("Samples to skip:", 90, 0, hs);
	long	frameHS = GetLong("Samples to read:", 512, 1, hs);
	long	skipHS = hs - frameHS - frameH0;
	
	long vs = dataCount/hs;

	long	frameVS = GetLong("N strings to read:", 128, 1, vs); // это чтобы читать только дырки
	long	frameV0 = GetLong("N strings to skip:", 256, 0, vs-frameVS);

	ComplexFunctionF32	waste1(hs), waste2(skipHS), waste3(frameH0);

	theMatrix.realloc(frameVS, frameHS);

	for(i = 0; i < frameV0; i++)
		{
		fread_numbers(waste1, theFile, ioComplexF32_LE);
		}
	for(i = 0; i < frameVS; i++)
	{
//		waste3.read_similar_type(theFile, ioBinaryLE);
//		theMatrix[i].read_similar_type(theFile, ioBinaryLE);
//		waste2.read_similar_type(theFile, ioBinaryLE);

		fread_numbers(waste3, theFile, ioComplexF32_LE);
		fread_numbers(theMatrix[i], theFile, ioComplexF32_LE);
		fread_numbers(waste2, theFile, ioComplexF32_LE);
		}
	fclose(theFile);


	physical_length displayV = cm(5*frameHS/256);
	physical_length displayH = cm(18*frameVS/256);
	DisplayMathFunction2D(theMatrix, "Matrix read", ScanFrameRectangle(displayV, displayH));
	}

string	opened_LC_file_name;

LC_File :: LC_File()
	{
	data_format = ioComplexF32_LE;
//	datum_size = sizeof(complexF32);//complexIO<float>::fsize();
	datum_size = complexIO<pcFloat32>::fsize();
	data_start_pos = 0x147;// в подражание о-брайеновскому стандарту
	theFile = NULL;
	burst_interleave = 1;
	}

LC_File :: ~LC_File()
	{
	if(theFile) fclose(theFile);
	}
XRAD_BEGIN
//tagRAW_BUF_HEADER SpectromedHeader;
XRAD_END

void	LC_File :: GuessFileFormat()
	{
	includes_b_data = false;

	fpos_t	file_len = filesize(theFile);

	if(!(file_len % (0x147 + full_frame_size * complexIO<pcFloat32>::fsize())))
		{
		// данные в формате о-брайена (только не помню, были они инт или флоат)
		data_format = ioComplexF32_LE;
		printf("\nPC float complex data, 0x147 header\n");
		datum_size = complexIO<pcFloat32>::fsize();
		data_start_pos = 0x147;
		}
	else if(!(file_len % (0x147 + full_frame_size * complexIO<pcInt16>::fsize())))
		{
		// данные в формате о-брайена (только не помню, были они инт или флоат)
		data_format = ioComplexI16PC;
		printf("\nPC int16 complex data, 0x147 header\n");
		datum_size = complexIO<pcInt16>::fsize();
		data_start_pos = 0x147;
		}
	else if(!(file_len % (full_frame_size * complexIO<pcFloat32>::fsize())))
		{
		// данные флоат без заголовка
		data_format = ioComplexF32_LE;
		printf("\nPC float complex data, no header\n");
		datum_size = complexIO<pcFloat32>::fsize();
		data_start_pos = 0;
		}
	else if(!(file_len % (full_frame_size * complexIO<pcInt16>::fsize())))
		{
		// данные инт16 без заголовка
		if(Decide2("Byte order", "PC","Mac",0)) data_format = ioComplexI16_BE;
		else data_format = ioComplexI16PC;
		printf("\nint16 complex data, no header\n");
		datum_size = complexIO<pcInt16>::fsize();
		data_start_pos = 0;
		}

	else if(*ext == '.dat')
		{
		// доплеровские данные от спектромеда
//		data_format = ioComplexLE;

// 		void	GetSpectromedCFMHeader(tagRAW_BUF_HEADER &SpectromedHeader, FILE *f);
// 		GetSpectromedCFMHeader(SpectromedHeader, theFile);
// 		Pause();
// 		data_format = ioComplexI32PC;
// 		printf("\nPC int32 complex data from spectromed, header containing B data\n");
// 		datum_size = complexIO<pcFloat>::fsize();
// 		data_start_pos = 512*SpectromedHeader.NumOfBeamsB;
// //		data_start_pos = 512*128;
// 		burst_interleave = GetLong("Burst interleave", pow(2, SpectromedHeader.CFMInterleave), 1, 4);
// 		includes_b_data = true;
		}
	else if(*ext == '.raw')
	{
	//с этим расширением данные могут быть в каком-либо нестандартном формате. стандартный ioPCComplex
		int	data_format = Get_Button_Decision("Data format", 4,
			"PC float",
			"Mac int 16",
			"PC int 16",
			"Mac float");
		
		switch(data_format)
			{
			case 0:
				data_format = ioComplexF32_LE;
				printf("\nPC float complex data\n");
				datum_size = 8;
				break;
			case 1:
				data_format = ioComplexI16_BE;
				printf("\nMac int16 complex data\n");
				datum_size = 4;
				break;
			case 2:
				data_format = ioComplexI16_LE;
				printf("\nPC int16 complex data\n");
				datum_size = 4;
				break;
			case 3:
				data_format = ioComplexF32_BE;
				printf("\nMac float complex data\n");
				datum_size = 8;
				break;
			}
		data_start_pos = 0x147;
		fflush(stdout);
		//Pause();
		}
	else FatalError("Invalid data format!");	
	n_frames = (file_len)/(datum_size*full_frame_size + data_start_pos);
	}



void	LC_File :: GetFrame(ComplexFunction2D_F32 &frame_container, unsigned int frame_no)
	{
	frame_no %= n_frames;
	
//	fseek(theFile, data_start_pos*(frame_no+1) + frame_no*full_frame_size*datum_size, SEEK_SET);
//	fseek(theFile, gap_before_first_ray, SEEK_CUR);
	
	fseek(theFile, data_start_pos*(frame_no+1) + frame_no*full_frame_size*datum_size + gap_before_first_ray, SEEK_SET);

	
//TODO что-то недоделано тут... размер кадра не проверяется, читает в тот буфер, который подсунули.
	
	for(int i = 0; i < frame_container.vsize(); i++)
		{
		int	line_to_read = i;
		int	quad_no = i/(burst_interleave*burst_count);
		int	quad_start = quad_no*(burst_interleave*burst_count);
		
		line_to_read = quad_start ? 
			quad_start + 
			((i%quad_start)/burst_interleave) +
			(i%burst_interleave)*burst_count
			: 0;
		
		fseek(theFile, gap_before_first_sample, SEEK_CUR);
		fread_numbers(frame_container[line_to_read], theFile, data_format);
		fseek(theFile, gap_after_last_sample, SEEK_CUR);
		}
	}

void	LC_File :: GetBFrame(RealFunction2D_F32 &m, unsigned int frame_no)
	{
	frame_no %= n_frames;

//	m.realloc(SpectromedHeader.NumOfBeamsB, 512);

	fseek(theFile, data_start_pos*frame_no + frame_no*full_frame_size*datum_size, SEEK_SET);

	for(int i = 0; i < m.vsize(); i++) fread_numbers(m[i], theFile, ioUI8);
	}



string LC_File :: ConvertFileName(const string &filename_with_path){
#ifndef __MWERKS__
	string result = filename_with_path;
#else
	int	count = strlen(filename_with_path);
	while(--count && filename_with_path[count] != '\\'){}
	if(count) count++;
	const	char	*result = filename_with_path + count;
#endif
	return result;
	}

	
LinearSignalHeader LC_File :: OpenLC_File()
	{
	LinearSignalHeader gHdr;
	areaHeader	aHdr;

	string hdrFileName;
	GetFileNameRead(hdrFileName, "Open header file", "*.note");
	FILE	*hdrFile = fopen(hdrFileName.c_str(), "rb");

	gHdr.scan(hdrFile);
	aHdr.scan(hdrFile);
	fclose(hdrFile);


	gHdr.print();
	aHdr.print();

	const string data_filename = ConvertFileName(gHdr.fileName);
	
	//TODO топорная работа со строкой, исправить
	ext = (int *)(&gHdr.fileName[0] + gHdr.fileName.length() - 4);
	theFile = fopen(data_filename.c_str(), "rb");
	if(!theFile) FatalError("Can't open the file!");
	
	full_frame_size = gHdr.nRays*gHdr.nSamples;
	
	burst_count = gHdr.nRays/gHdr.nElements;
	GuessFileFormat();
	printf("\n%d frames in the file\n", n_frames);
	
	gap_before_first_ray = aHdr.firstRay * gHdr.nSamples * datum_size;
	gap_before_first_sample = aHdr.firstSample * datum_size;
	gap_after_last_sample = (gHdr.nSamples - (aHdr.firstSample + aHdr.nSamples)) * datum_size;

	
//	GetFrame(theMatrix, 0);

	
 	int	nRepeats = gHdr.nRays/gHdr.nElements;
	fflush(stdout);

	gHdr.rMin += aHdr.firstSample*gHdr.dZ;
	gHdr.rMax = gHdr.rMin + aHdr.nSamples*gHdr.dZ;
	gHdr.nRays = aHdr.nRays;
	gHdr.nSamples = aHdr.nSamples;
	gHdr.nElements = aHdr.nRays / nRepeats;
	return	gHdr;
	}
	
