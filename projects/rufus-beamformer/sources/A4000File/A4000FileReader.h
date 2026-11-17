//---------------------------------------------------------------------------

#ifndef __a4000_file_reader_h
#define __a4000_file_reader_h


#include <XRADBasic/ContainersAlgebra.h>
#include <XRADBasic/DataArrayIO.h>

#include <A4000File/A4000FileHeader.h>
#include <XRADSystem/CFile.h>

XRAD_BEGIN


//----------------------------------------------------------------------
//
//	класс отвечает только за открывание набора данных в формате A4000
//	и чтение нужного кадра в заданный буфер.
//

class	A4000DataReader : public A4000DataHeader
	{
	private:	
		DataArray<shared_cfile> dat_file; // public is temporary

		size_t GetFrameFileNo(size_t frame_no) const {return frame_no/n_frames_per_file;}
		shared_cfile	&GetFrameFile(size_t frame_no){return dat_file[GetFrameFileNo(frame_no)];}

		size_t RayDataOffset(size_t frame_no, size_t ray_no) const;

		void	OpenDataFile(shared_cfile &datafile, const string& filename);
	public:
		void	OpenDataFiles();

		void	GetA4000Frame(ComplexFunction2D_F32 &container, size_t n);
	};



XRAD_END


//---------------------------------------------------------------------------
#endif   // __a4000_file_reader_h
