#ifndef __spectromed_rf_data_h
#define __spectromed_rf_data_h

#include <XRADBasic/ContainersAlgebra.h>
#include <Utils/SignalFilters.h>
#include <RFDataImport/RFDataImporter.h>
#include <A4000File/A4000FileReader.h>

//радиочастотные данные от Сономеда-400

XRAD_BEGIN


#ifndef UINT
#define UINT unsigned int
#endif

#ifndef BYTE
#define BYTE char
#endif

#ifndef BOOL
#define BOOL bool
#endif

#ifndef WORD
#define WORD unsigned short
#endif





class	SpectromedRFDataImporter : public RFDataImporter
{
private:
	size_t frame_size_bytes;
	size_t interleave_count;

	shared_cfile	theFile;
	string file_name;

public:
	const string &DataSourceName() const {
		return file_name;
	}
	virtual	string	GetComment(){
		return string("Spectromed data, no comment");
	}

	SpectromedRFDataImporter();
	virtual ~SpectromedRFDataImporter();


	bool	OpenRFData();
	virtual	void	GetFrame(size_t frame_no);
	size_t GetFrameOffset(size_t frame_no, size_t line_no) const;
};

XRAD_END




#endif // __spectromed_rf_data_h
