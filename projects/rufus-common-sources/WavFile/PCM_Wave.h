#ifndef __pcm_wave_h
#define __pcm_wave_h

#define NEAR
#define FAR
#define far
#define WINVER 0x0000
#define MMNOTIMER
#define MMNOJOY
#define MMNOMMIO
#define MMNOMCI

#include <pc_int.h>

struct	waveFmt{
	pc_short formatType;
	pc_short nChannels;
	pc_long	nSamplesPerSec;
	pc_long	nAvgBytesPerSec;
	pc_short bytesPerSample; //1, 2, 4
	pc_short bitsPerSample; //8 or 16
	};


struct	wavFilePrefix{
	char	RIFF[4];
	pc_long	fileSize;
	char	WAVEfmt[8];
	pc_long	waveFmtSize;
	waveFmt	formats;
	
	char	data[4];
	pc_long	dataSize;

	wavFilePrefix()
		{
		strcpy(RIFF, "RIFF");
		strcpy(WAVEfmt,"WAVEfmt ");
		strcpy(data, "data");		
		waveFmtSize = 16L;
		SetDataSize(0);
		InitWaveFormats();
		};
		
	void	SetDataSize(long x)
		{
		dataSize = x;
		fileSize = x+36;
		};
		
	void	InitWaveFormats(){ //16-bit, 44100 HZ, mono
		formats.formatType = (short)1;
		formats.nChannels = (short)1;
		formats.nSamplesPerSec = 44100L;
		formats.nAvgBytesPerSec = 2L*formats.nSamplesPerSec;
		formats.bytesPerSample=2;
		formats.bitsPerSample=16;
		};

	
	void	SetRatings(short nChannels, short nBits, long sampleRate)
		{
		formats.nChannels = nChannels;
		formats.bitsPerSample = nBits;
		formats.nSamplesPerSec = sampleRate;
		formats.bytesPerSample = nChannels*nBits/8;
		formats.nAvgBytesPerSec = formats.nSamplesPerSec*formats.bytesPerSample;
		};
	};

#endif // __pcm_wave_h