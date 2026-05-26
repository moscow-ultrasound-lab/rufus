#include "pre.h"

#include "PCM_Wave.h"



void	GetWaveHeader()
	{
	wavFilePrefix	thePrefix;
	FILE	*theFile, *theOPFile;
	long	howMuch = Get_Long("How KB to leave?", 4, 1, 100);

	char	*buffer = (char *)calloc(howMuch,1024);
	char	theFileName[256];
	char	fileName2[256];
		
	pc_long	number;
	pc_short number2;
	long	x;
	short	y;
	
	Get_File_Name2(theFileName,"file to cut", TRUE);
	theFile = fopen(theFileName, "rb");
	if(!theFile) Fatal_Error("Can't open file!!!");

	_ftype = 'MPEG';
	_fcreator = 'MPgP';
	
	sprintf(fileName2, "This is a cutС%s", theFileName);
	theOPFile = fopen(fileName2, "wb");
	if(!theOPFile) Fatal_Error("Can't open file to cut");
	fread(buffer, 1, 4096, theFile);
	fwrite(buffer, 1, 4096, theOPFile);
	
	ExitToShell();
	}

/*	
	923-96-33
	
	{RIFF
	long	fileLength (file length without 8 bytes)
	WAVEfmt
	
	}
*/