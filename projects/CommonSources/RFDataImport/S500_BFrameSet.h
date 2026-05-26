#ifndef __BFrameSet_h
#define __BFrameSet_h

//------------------------------------------------------------------
//
//	author:		kns
//	
//	purpose:	хранение многокадровых массивов уз данных
//
//------------------------------------------------------------------

#include <XRADBasic/ContainersAlgebra.h>
#include <XRADBasic/Sources/ScanConverter/ScanConverterOptions.h>

XRAD_BEGIN


class	BFrameSet
{
public:
//		typedef MathFunction<uint8_t, double>  row_t;//double - важно, чтобы можно было умножать на дробные коэффициенты без потери
	typedef RealFunctionUI8F row_t;//D - важно, чтобы можно было умножать на дробные коэффициенты без потери
	typedef MathFunction2D<row_t>  frame_t;
	typedef ReferenceOwner<frame_t> frame_ref_t;

// 		double	min_speed, max_speed;

protected:

	size_t	m_n_frames;
	size_t	m_n_rays, m_n_samples;

	DataArrayMD<frame_t>	frames;

public:
	//initialization
	BFrameSet();
	void LoadFrames(wstring filename);

	// info
	size_t	n_frames(){ return m_n_frames; }
	size_t	n_rays(){ return m_n_rays; }
	size_t	n_samples(){ return m_n_samples; }

	// access
	const frame_ref_t	frame_ref(size_t frame_no) const;
	const DataArrayMD<frame_t>	GetFrames() const{ return frames; }
};



XRAD_END


#endif //__BFrameSet_h