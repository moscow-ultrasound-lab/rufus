#include "pre.h"
//#include <shared_cfile.h>
#include <XRADBasic/Sources/Utils/LeastSquares.h>
#include <XRADBasic/Sources/Utils/StatisticUtils.h>
#include <XRADBasic/ContainersAlgebra.h>

#include "S500_BFrameSet.h"
#include "S500_BFrameSetAnalyze.h"

XRAD_BEGIN



BFrameSet::BFrameSet()
{
	m_n_frames = 50;
	m_n_rays = 0;
	m_n_samples = 512;
}

const BFrameSet::frame_ref_t BFrameSet::frame_ref(size_t frame_no) const
{
	frame_ref_t	result;
	const_cast<DataArrayMD<BFrameSet::frame_t> &>(frames).GetSlice(result, {frame_no, slice_mask(0), slice_mask(1)});
	return result;
}


void BFrameSet::LoadFrames(wstring filename)
{
#pragma message Переделать под shared_cfile
	RealFunctionMD_UI8	buffer;

	FILE	*f = _wfopen(filename.c_str(), L"rb");
	if(!f) throw invalid_argument(string("File '")+wstring_to_string(filename, e_encode_literals)+string("' could not be opened"));

	LoadS500_BFrameSet(buffer, f);
	fclose(f);

// 	m_n_rays = f.filesize()/(m_n_frames*m_n_samples);
// 	size_t	m_default_n_samples = 512;
// 	m_n_rays = filesize(f)/(m_n_frames*m_default_n_samples);
	m_n_frames = buffer.sizes(0);
	m_n_rays = buffer.sizes(1);
	m_n_samples = buffer.sizes(2);



	frames.realloc({m_n_frames, m_n_rays, m_n_samples});

	for(size_t i = 0; i < m_n_frames; ++i)
	{
//		RealFunction2D_UI8 frame_buffer, frame_part;//(m_n_rays, m_default_n_samples), frame_part;
		auto frame_buffer = buffer.GetSlice({i, slice_mask(0), slice_mask(1)});
//		buffer.GetSlice(frame_buffer, {i, slice_mask(0), slice_mask(1)});
//		frame_part.UseDataFragment(frame_buffer, 0, m_start_sample, m_n_rays, m_end_sample);

		frame_t	slice;
		frames.GetSlice(slice, {i, slice_mask(0), slice_mask(1)});
		//		void	UseDataFragment(DataArray2D &new_data, size_t v0, size_t h0, size_t v1, size_t h1);
		CopyData(slice, frame_buffer);
	}
}




XRAD_END