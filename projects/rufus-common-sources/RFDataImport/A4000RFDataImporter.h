#ifndef __aloka_rf_data_h
#define __aloka_rf_data_h

#include <XRADBasic/ContainersAlgebra.h>
#include <RFDataImport/RFDataImporter.h>
#include <A4000File/A4000FileReader.h>

XRAD_BEGIN


class	A4000RFDataImporter : public RFDataImporter
	{
		virtual	void	GetFrame(size_t frame_no);

	private:
		A4000DataReader data_reader;

		const string &DataSourceName() const {return data_reader.hdr_file_name;}
	public:

		A4000RFDataImporter();		
		
		virtual	string	GetComment();
		
		bool	OpenRFData();
		size_t	n_focuses;
	};

XRAD_END




#endif //__aloka_rf_data_h
