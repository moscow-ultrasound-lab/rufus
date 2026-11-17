#ifndef __linear_signal_h
#define __linear_signal_h


#include "LinearConvexHeader.h"
#include <MathFunctionTypesMD.h>
#include <MathFunctionGUIMD.h>
//#include <DopplerParams.h>
#include <LinearConvexFile.h>
#include <PhysicalUnits.h>




class	LinearSignalOptions
{
protected:
	int	n_aperture_elements;
	physical_length	arrayPitch;

	bool	TX_Focusing, RX_Focusing;

	int	n_frames;
	int	n_rays;
	int	n_array_elements;
	int	n_samples;
	int	nRepeats;

	physical_length	rMin, rMax;
	physical_length	TX_Focus, RX_Focus;
	physical_length	convexRadius;
	string	comment;

	static	physical_speed	soundSpeed; //cm/sec
	static	physical_frequency	sampleRate;
	static	double	arrayQuotient;

	LC_File	data_file;
	void	WriteHeaderFile(const char *data_filename) const;

	LinearSignalOptions();
	virtual ~LinearSignalOptions();
};

enum	signal_save_option{
	create_new_file,
	continue_existing_file
};

class	LinearSignal: protected LinearSignalOptions, public ComplexFunction2D_F32
{
	ComplexFunctionMD_F32	data;

public:
	LinearSignal(){};
	virtual ~LinearSignal(){};

	void	SetFrame(int frame_no);

	void	Save();
	void	Save(string filename, signal_save_option op);
	int		GetNFrames() const {return n_frames;}

	void	Open();
	void	OpenSpectromed();

	void	Display(char *, double scalefactor = 1);
	void	DisplayFrame(int frame_no, char *, double scalefactor = 1);
	void	DisplayFrameSet(char *Title, double scalefactor = 1);

	PhysicalFrameDimensions	GetFrameDimensions(double scalefactor);
};



#endif