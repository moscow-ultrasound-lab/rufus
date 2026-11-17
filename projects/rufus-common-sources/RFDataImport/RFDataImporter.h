#ifndef __rf_data_importer_h
#define __rf_data_importer_h

#include <XRADBasic/ContainersAlgebra.h>
#include <Utils/SignalFilters.h>
#include <XRADSystem/CFile.h>
#include <XRADBasic/Sources/ScanConverter/ScanAreaGeometry.h>
#include <ProbeOptions/UltrasonicProbeOptions.h>
#include <ProbeOptions/ApertureFocusingOptions.h>
#include <ProbeOptions/ScanParams.h>

XRAD_BEGIN






class	RFDataImporter //: public ScanParams
	{

	public:
		SignalFilter	*filter;

		virtual const string &DataSourceName() const = 0;
	public:

//		int	interleave_count;
		size_t n_frames;
//		physical_length	z0;


		ScanParams	sp;

		physical_length	dz;
		physical_length	z_window_center;
		physical_length	z_window_width;
		
		ComplexFunction2D_F32	frame_buffer;


		bool	use_dynamic_filter;
	
	private:
		RealFunctionF32	depth_window;
	
	public:
		RFDataImporter();
		virtual ~RFDataImporter();
		void	DisplayFrameBuffer(const string &title);
		
	public:
		//	virtual pure void functions
		virtual	bool	OpenRFData() = 0;
		virtual	void	GetFrame(size_t frame_no) = 0;
		virtual	string	GetComment() = 0;

		//	filtering and import functions
		//void	ImportFrame(ComplexFunction2D original);
		void	SelectDepthWindow();
		void	ApplyDepthWindow();
		
		void	SelectFilter();
		void	FilterFrame();		
		void	DownsampleFrame(ComplexFunction2D_F32 &md, size_t downsampling_factor);
		double	AllowedDownsampleRatio(){return filter->AllowedDownsampleRatio();}
		
	};

XRAD_END




#endif // __rf_data_importer_h
