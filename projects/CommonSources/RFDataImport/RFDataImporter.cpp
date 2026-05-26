#include "pre.h"
//#include <MathFunctionGUI2D.h>

#include <RFDataImport/RFDataImporter.h>
#include <RFDataImport/S400_SpectromedRFDataImporter.h>

//bool return_default_value = true;
bool return_default_value = false;

//--------------------------------------------------------
//
//	quasi-rect filter
//
//--------------------------------------------------------
namespace
{
using std::fabs;
XRAD_USING

float	smoothed_rect(float x, float w, float dw_relative = 0.125)
{
	float	x0 = fabs(x); // even function
	float	w2 = w/2;
	float	dw = dw_relative*w;

	if(x0 < w2-dw) return 1;
	if(x0 > w2+dw) return 0;
	return (1. - sin(pi()*0.5*(x0-w2)/dw))/2;
}

//#define window_function gauss
#define window_function smoothed_rect


} //namespace




XRAD_BEGIN


//--------------------------------------------------------
//
//	Initialization
//
//--------------------------------------------------------


RFDataImporter::RFDataImporter() : filter(NULL)
{
//	sample_rate = 78.848;
//	sample_rate = Hz(0);
	n_frames = 0;
//	sfd.n_rays = 0;
//	sfd.n_samples = 0;

//	carrier = Hz(0);
//	carrier_offset = Hz(0);
//	filter_bandwidth = 0;
	use_dynamic_filter = false;
}

RFDataImporter :: ~RFDataImporter()
{
}



//--------------------------------------------------------
//
//	Depth window selection
//
//--------------------------------------------------------

void	RFDataImporter::SelectDepthWindow()
{ // сделать BOOL ?!
	RealFunctionF32	sample_magnitude(sp.sfd.n_samples, 0);
// 	CopyData(sample_magnitude, frame_buffer[sp.sfd.n_rays/2], cabs_functor<float, complexF32>());

#pragma message здесь проблема с функтором, лямбда помогла
	CopyData(sample_magnitude, frame_buffer.row(sp.sfd.n_rays/2), [](float &c, const complexF32 &x){return c=cabs(x);});
//	CopyData(sample_magnitude, frame_buffer[sp.sfd.n_rays/2], Functors::absolute_value());
	sample_magnitude /= MaxValue(sample_magnitude);
	depth_window.realloc(sp.sfd.n_samples);

//	GraphSet	range_select("Depth window selection", "cm", "");		
//	range_select.AddGraph(sample_magnitude, 0, dz, "Sample signal");
//	range_select.AddGraph(depth_window, 0, dz, "Range window");

//	do
//		{
	return_default_value ?
		z_window_center :
		z_window_center = cm(GetFloating("Depth of interest", z_window_center.cm(), 0, sp.pfd.depth_range().cm()));
	return_default_value ?
		z_window_width :
		z_window_width = cm(GetFloating("Depth range", z_window_width.cm(), 0.1, HUGE_VAL));

	for(size_t i = 0; i < sp.sfd.n_samples; i++)
	{
		physical_length	z = i*dz;
		depth_window[i] = window_function((z-z_window_center).cm(), z_window_width.cm());
	}
//		range_select.ChangeGraph(1, depth_window, 0, dz);		
//		}while(!YesOrNo("Is range correct?", YES));

	if(0)
	{
		size_t n0(10), n1(10);
		double	a = 0.01;
		for(size_t i = 0; i < n0; ++i) depth_window[i] = a;
		cos2_window	wf;
		for(size_t i = 0; i < n1; ++i) depth_window[i+n0] = a + (1.-a)*wf(i, 2*n1);
		for(size_t i = n0+n1; i < sp.sfd.n_samples/2; ++i) depth_window[i] = 1;

		DisplayMathFunction(depth_window, 0, 1, "DW");
	}
}


void	RFDataImporter::ApplyDepthWindow()
{
	GUIProgressBar	progress;
	progress.start("Depth ranging", sp.sfd.n_rays);
	for(size_t i = 0; i < sp.sfd.n_rays; i++)
	{
		frame_buffer.row(i) *= depth_window;
		++progress;
	}
}

//--------------------------------------------------------
//
//	Filter selection and filtering
//
//--------------------------------------------------------


void	RFDataImporter::SelectFilter()
{
	return_default_value ?
		use_dynamic_filter = false :
		use_dynamic_filter = YesOrNo("Use dynamic filter?", false);
	if(use_dynamic_filter) filter = new DynamicFrequencyDomainFilter();
	else filter = new StaticFrequencyDomainFilter(sp.upo.CarrierFrequency(), sp.upo.CarrierFrequency());

	filter->sample_rate = sp.sfd.sample_rate;
//	filter->SetUpFilter(frame_buffer, sfd.sample_rate, z0, pfd.depth_range);
	filter->SetUpFilter(frame_buffer, sp.sfd.sample_rate, sp.pfd.r_min(), sp.pfd.depth_range());
}

void	RFDataImporter::FilterFrame()
{
	filter->ApplyFilter(frame_buffer);
}


//--------------------------------------------------------
//
//	Downsampling
//
//--------------------------------------------------------


void	RFDataImporter::DownsampleFrame(ComplexFunction2D_F32 &md, size_t downsampling_factor)
{
	size_t n_rays = frame_buffer.vsize();
	size_t n_samples = frame_buffer.hsize();
	size_t n_samples_d = n_samples/downsampling_factor;

	md.realloc(n_rays, n_samples_d);
	for(size_t i = 0; i < n_samples_d; i++)
	{
		md.col(i).CopyData(frame_buffer.col(i*downsampling_factor));
	}

}

void RFDataImporter::DisplayFrameBuffer(const string &title)
{
	DisplayMathFunction2D(frame_buffer, title, sp.pfd);
}

//--------------------------------------------------------
//
//	Test function
//
//--------------------------------------------------------


void	TestSpectromedRF()
{
	RFDataImporter *sd = new SpectromedRFDataImporter;
	RFDataImporter &sample_data(*sd);

	if(!sample_data.OpenRFData()) return;

	size_t frame_no(0);

	do
	{
		frame_no = GetSigned("Frame no", frame_no+1, 1, sample_data.n_frames)-1;
		sample_data.GetFrame(frame_no);
		sample_data.SelectDepthWindow();
		sample_data.ApplyDepthWindow();
		sample_data.SelectFilter();
		sample_data.FilterFrame();

		ComplexFunction2D_F32	md;

		sample_data.DownsampleFrame(md, 10);
		DisplayMathFunction2D(md, "Final data", ScanFrameRectangle(cm(10), cm(10)));
	} while(YesOrNo("See another frame?", true));
}

XRAD_END
