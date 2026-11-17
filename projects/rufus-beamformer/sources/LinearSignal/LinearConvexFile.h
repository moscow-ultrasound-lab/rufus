#ifndef __lcfile_h
#define __lcfile_h


#include <LinearConvexHeader.h>

XRAD_BEGIN

class	LC_File
	{
	int	data_start_pos;
	int	datum_size;
	FILE	*theFile;

	ioNumberOptions	data_format;
	int	*ext;
	string	opened_LC_file_name;
// 	fpos_t	file_len;
	
	int	n_frames;
	int	full_frame_size;
	
	int	gap_before_first_ray;
	int	gap_before_first_sample;
	int	gap_after_last_sample;

	bool	includes_b_data;
	//	spectromed CFM files with B-data included
	int	burst_interleave;
	int	burst_count;
	void	GuessFileFormat();
	string ConvertFileName(const string &);
public:
	LC_File();
	virtual ~LC_File();
	int	GetFramesCount(){return n_frames;};
	LinearSignalHeader	OpenLC_File();
	void	GetFrame(ComplexFunction2D_F32 &m, unsigned int frame_no);
	void	GetBFrame(RealFunction2D_F32 &m, unsigned int frame_no);
	};



XRAD_END

#endif //__lcfile_h
