#ifndef Options_H
#define Options_H

#include "SignalOptions.h"
#include "SectorOptions.h"
#include "SimIOHeaders/SimIOHeaders.h"
//#include "Q:\projects\ATL\Sources\SimIOHeaders/SimIOHeaders.h"
//#include "SubapertDataDescriptor.h"

//#define	V_Tissue	1.54	// 1-way speed in tissue (mm/µSec)


XRAD_USING



class	Options :public PhysicalFrameDimensions, public SignalOptions
{
public:
	static	int	Object_Count;
	static	int	numFiles;
	static	int	Path_Num;
//		int	focus_Algorithm;

	aperture_focusing TX_Focus;
	aperture_focusing RX_Focus;

//	bool	Visible;

//		float	sound_speed;
//		float	omega0;
//		float	band_half_width;
//		float	sample_Rate;

	physical_speed	sound_speed;

//		physical_frequency sample_rate;
	physical_frequency band_half_width;
	physical_frequency omega0;

//		float	array_Pitch;
	physical_length	array_Pitch;


//		char	*Depth_Units;
//		char	*Frequency_Units;
//		char	*Angle_Units;
//		char	*Speed_Units;

//	static	char	**IP_File_Names;
//	static	char	**OP_File_Names;

	Options();
	virtual ~Options();

	void	CopyToSIMIO();
	void	CopyFromSIMIO();
	bool	CompareWithSIMIO();
	void	DumpSIMIO(FILE *);
	void	LoadSIMIO(FILE *);

//		void	SetDepthUnits(const char *);	
//		void	SetFrequencyUnits(const char *);	
//		void	SetAngleUnits(const char *);	
//		void	SetSpeedUnits(const char *);	// always CM_SEC

		//string	Object_Comment;
	void	Attach(Options *);

	//SETUP_TIMER
	void	ExportScanConverterOptions(ScanConverterOptions &sco);
};




#endif