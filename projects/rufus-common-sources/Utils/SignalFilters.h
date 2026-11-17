#ifndef __signal_filters_h
#define __signal_filters_h

#include <XRADBasic/ContainersAlgebra.h>
//#include <MathFunctionGUI.h>
#include <XRADBasic/Sources/Utils/PhysicalUnits.h>

XRAD_BEGIN

class	SignalFilter
	{
	protected:
	public:

		physical_frequency	sample_rate;
	
	public:
		virtual	bool	LoadFilter(physical_length z0, physical_length z_range) = 0;
		virtual	void	SaveFilter() = 0;
		virtual	void	ApplyFilter(ComplexFunction2D_F32 &frame_buffer) = 0;
		virtual	void	SetUpFilter(ComplexFunction2D_F32 &frame_buffer, physical_frequency sample_rate, physical_length z0, physical_length z_range) = 0;
		virtual double	AllowedDownsampleRatio() = 0;
	};

class	StaticFrequencyDomainFilter : public SignalFilter
	{
		bool	LoadFilter(physical_length , physical_length){return false;};
		
		double	allowed_downsample_ratio;
	public:

		physical_frequency	filter_f0;
		physical_frequency	filter_bandwidth;
//		double	filter_flatness;
		
		RealFunctionF32	fd_filter;
		
		StaticFrequencyDomainFilter(physical_frequency f0 = Hz(0), physical_frequency bandwidth = Hz(0)) : filter_f0(f0), filter_bandwidth(bandwidth)/*, filter_flatness(0.1)*/
			{
			allowed_downsample_ratio = 1;
			};

		void	SaveFilter(){};
		virtual void ApplyFilter(ComplexFunction2D_F32 &frame_buffer);
		void	SetUpFilter(ComplexFunction2D_F32 &frame_buffer, physical_frequency sample_rate, physical_length z0, physical_length z_range);
		void	InitFilter(physical_frequency f0, physical_frequency bandwidth/*, double flatness*/);
		
		
//		virtual void	CalculateFDFilter(RealFunctionF32 &filter);
		virtual void CalculateFDFilter(RealFunctionF32 &filter, const ComplexFunction2D_F32 &frame_buffer);
		double	AllowedDownsampleRatio(){return allowed_downsample_ratio;}
	};




struct	DynamicFrequencyDomainFilter: public SignalFilter
	{
	size_t n_ranges;
//	RealFunctionF32	f0;
//	RealFunctionF32	bandwidth;
	double	filter_flatness;
	DataArray<StaticFrequencyDomainFilter>	filters;
//	DynamicFrequencyDomainFilter():filter_flatness(0.5), f0(0), bandwidth(0){}
		
	void	InitDynamicFilter(size_t n, double in_filter_flatness)
		{
		n_ranges = n;
		if(!(n_ranges%2)) n_ranges++;
		filters.realloc(n_ranges);
		filter_flatness = in_filter_flatness;
		}
	
	bool	LoadFilter(physical_length z0, physical_length z_range);
	bool	LoadFilter(const string &filename);
	bool	LoadUniversalFilter(const string &filename, physical_length z0, physical_length z_range);
	void	SaveFilter();
	virtual void	ApplyFilter(ComplexFunction2D_F32 &frame_buffer);
	void	SetUpFilter(ComplexFunction2D_F32 &frame_buffer, physical_frequency sample_rate, physical_length z0, physical_length z_range);

	double	AllowedDownsampleRatio()
		{
		RealFunctionF32 adr(n_ranges);
		for(size_t i = 0; i < n_ranges; ++i) adr[i] = filters[i].AllowedDownsampleRatio();
		DisplayMathFunction(adr, 0,1,"adr");
		return MinValue(adr);
		}
	};



XRAD_END


#endif // __signal_filters_h
