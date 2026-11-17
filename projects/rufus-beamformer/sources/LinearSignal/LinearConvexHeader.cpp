#include "pre.h"
#include "LinearConvexHeader.h"

void	LinearSignalHeader :: print(FILE *stream)
	{
	char	b[2] = {'%', 0};
	if(!stream)
		{
		*b = 0;
		stream  = stdout;
		}

	fprintf(stream, "\n%sversion = %lg;\n", b, currentLSHeaderVersion);

	fprintf(stream, "%sfilename = \"%s\";\n", b, fileName.c_str());
	fprintf(stream, "%ssoundSpeed = %lg cm/sec;\n", b, soundSpeed);
// 	fprintf(stream, "%somega0 = %lg MHz;\n", b, omega0);
// 	fprintf(stream, "%shalfWidth = %lg MHz;\n", b, halfWidth);
	fprintf(stream, "%ssampleRate = %lg MHz;\n", b, sampleRate);
 
	fprintf(stream, "%snElements = %lu;\n", b, nElements);
	fprintf(stream, "%snApertElements = %lu;\n", b, nApertElements);
	fprintf(stream, "%sarrayPitch = %lg cm;\n", b, arrayPitch);
	
	fprintf(stream, "%sTX_Focusing = %lu;\n", b, long(TX_Focusing));
	fprintf(stream, "%sRX_Focusing = %lu;\n", b, long(RX_Focusing));
	
	fprintf(stream, "%sn_Frames = %lu;\n", b, n_frames);
	fprintf(stream, "%sn_Rays = %lu;\n", b, nRays);
	fprintf(stream, "%sn_Samples = %lu;\n", b, nSamples);

	fprintf(stream, "%srMin = %lg cm;\n", b, rMin);
	fprintf(stream, "%srMax = %lg cm;\n", b, rMax);
	fprintf(stream, "%sTX_Focus = %lg cm;\n", b, TX_Focus);
	fprintf(stream, "%sRX_Focus = %lg cm;\n", b, RX_Focus);
	fprintf(stream, "%sconvexRadius = %lg cm;\n", b, convexRadius);
	
	}


long	fskip_until(FILE *theFile, unsigned char c)
	{
	long	count = 0;
	int	value = 0;
	while(value != c && value != EOF)
		{
		value = fgetc(theFile);
		count ++;
		}
	if(value == c) return count;
	else return 0;
	}

string	fget_string(FILE *theFile)
	{
	if(!fskip_until(theFile, '\"')) FatalError("Unexpected end of file!");
	
	long	pos0 = ftell(theFile);
	long	len = fskip_until(theFile, '\"');
	
	if(!len) FatalError("Unexpected end of file!");
	//if(len >= 255) FatalError("Too long string in file!");
	
	DataArray<char> buffer(len);
	
	fseek(theFile, pos0, SEEK_SET);
	fread(&buffer[0], len, sizeof(char), theFile);
	buffer[len-1] = 0;
	
	return string(&buffer[0]);
	}

int	fget_param(FILE *theFile, char *input_format, void *value)
	{
	int	result = 0;
	int	original_position = ftell(theFile);
	bool	flipped = false;

	while(!result)
		{		
		if(!fskip_until(theFile, '%') && !flipped)
			{
			flipped = true;
			// начинаем поиск с начала
			fseek(theFile, 0, SEEK_SET);
			}

		int	current_position = ftell(theFile);
		if(flipped && current_position>original_position)
			{
			// при ошибке возвращаем исходное положение в файле и бросаем исключение
			fseek(theFile, original_position, SEEK_SET);
			FatalError(ssprintf("Parameter '%s' not found!", input_format));
			}
		result = fscanf(theFile, input_format, value);
		}
	return result;
	}

void	LinearSignalHeader :: scan(FILE *textFile)
	{
	// Total data file specificatoin
	
	fget_param(textFile, "version = %lg;", &version);
	if((version - currentLSHeaderVersion) >= 0.09) FatalError("Header version does not match!");
	
	if(!fskip_until(textFile, '%')) FatalError("Unexpected end of file!");
	fileName = fget_string(textFile);
		
	fget_param(textFile, "soundSpeed = %lg cm/sec;", &soundSpeed);
	
// 	fget_param(textFile, "omega0 = %lg;", &omega0);
// 	fget_param(textFile, "halfWidth = %lg;", &halfWidth);
	fget_param(textFile, "sampleRate = %lg MHz;", &sampleRate);
	
	fget_param(textFile, "nElements = %lu;", &nElements);
	fget_param(textFile, "nApertElements = %lu;", &nApertElements);
	fget_param(textFile, "arrayPitch = %lg cm;", &arrayPitch);
	
	long	b;
	fget_param(textFile, "TX_Focusing = %lu;", &b); TX_Focusing = b?true:false;
	fget_param(textFile, "RX_Focusing = %lu;", &b); RX_Focusing = b?true:false;
	
	try
		{
		fget_param(textFile, "n_Frames = %lu;", &n_frames);
		}
	catch(runtime_error &)
		{
		// для совместимости с прежними версиями
		n_frames = 1;
		}
	
	fget_param(textFile, "n_Rays = %lu;", &nRays);
	fget_param(textFile, "n_Samples = %lu;", &nSamples);
	fget_param(textFile, "rMin = %lg cm;", &rMin);
	fget_param(textFile, "rMax = %lg cm;", &rMax);
	fget_param(textFile, "TX_Focus = %lg cm;", &TX_Focus);
	fget_param(textFile, "RX_Focus = %lg cm;", &RX_Focus);
	fget_param(textFile, "convexRadius = %lg cm;", &convexRadius);
	refresh_data();
	}
	
	
void	LinearSignalHeader :: refresh_data()
	{
	if(sampleRate)
		{
		double	rFact = soundSpeed * nSamples/(2*sampleRate*1.e6);
		rMax = rMin + rFact;
		}
	else if(!(rMax - rMin)) Error("Invalid data specification!");
	else{
		sampleRate = soundSpeed * nSamples/(2*(rMax - rMin) * 1.e6);
		}
	dZ = (rMax-rMin)/nSamples;
	}
	
	
		
void	areaHeader :: scan(FILE *textFile)
	{
	if(!fskip_until(textFile, '%')) FatalError("Unexpected end of file!");
	comment = fget_string(textFile);
	
	fget_param(textFile, "firstRay = %lu;", &firstRay);
	fget_param(textFile, "n_Rays = %lu;", &nRays);
	fget_param(textFile, "firstSample = %lu;", &firstSample);
	fget_param(textFile, "n_Samples = %lu;", &nSamples);
	}
	
void	areaHeader :: print(FILE *stream)
	{
	char	b[2] = {'%', 0};
	if(!stream)
		{
		*b = 0;
		stream  = stdout;
		}
	fprintf(stream, "\n%scomment = \"%s\";", b, comment.c_str());

	fprintf(stream, "\n%sfirstRay = %lu;", b, firstRay);
	fprintf(stream, "\n%sn_Rays = %lu;", b, nRays);
	fprintf(stream, "\n%sfirstSample = %lu;", b, firstSample);
	fprintf(stream, "\n%sn_Samples = %lu;\n", b, nSamples);
	}