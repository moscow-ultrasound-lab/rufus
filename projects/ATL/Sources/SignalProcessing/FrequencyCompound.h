#ifndef __frequency_compound_h
#define __frequency_compound_h

#include "SignalProcessing/SignalProcessor.h"
#include <Utils/SignalFilters.h>

XRAD_BEGIN




//------------------------------------------------------------------------------------
//
//

template<class FILTER_T>
class	TimeDomainFilter : public FILTER_T
	{
		PARENT(FILTER_T);
	private:
		// наспех это:
//		enum	{upsample_ratio = 6};
		enum	{td_filter_order=57};
		
	public:
		double	filter_gain;
		RealFunctionF32 td_filter;
		TimeDomainFilter(physical_frequency f0 = MHz(0), physical_frequency bandwidth = MHz(0)) : parent(f0, bandwidth){}
		virtual void ApplyFilter(ComplexFunction2D_F32 &frame_buffer);
	};

//
//------------------------------------------------------------------------------------




class	ForceSpectrumFilter : public StaticFrequencyDomainFilter
	{
	PARENT(StaticFrequencyDomainFilter);
	public:
		ForceSpectrumFilter(physical_frequency f0 = MHz(0), physical_frequency bandwidth = MHz(0)) : StaticFrequencyDomainFilter(f0, bandwidth){};
//		virtual void ApplyFilter(ComplexFunction2D_F32 &frame_buffer);
		virtual void CalculateFDFilter(RealFunctionF32 &filter, const ComplexFunction2D_F32 &frame_buffer);
		void	ApplyAttenuation(double factor_per_sample);
	};




//------------------------------------------------------------------------------------
//
//

//class	FilterSet : public DataArray<ForceSpectrumFilter>
class	FilterSet : public DataArray<TimeDomainFilter<ForceSpectrumFilter> >
	{
//		typedef ForceSpectrumFilter filter_type;
		typedef TimeDomainFilter<ForceSpectrumFilter> filter_type;
		
		physical_frequency	default_bandwidth;
		physical_frequency	max_frequency;
		physical_frequency	min_frequency;
	public:
		RealFunctionF32	filter_attenuation;
		RealFunctionF32	filter_gain;
		physical_frequency	sample_rate;
		void	DisplayFilters();
		void	WriteFilters(const char* fn_template, size_t n_samples);
		void	InitSignalFilters(size_t nsf, double max_frequency_factor);
	};

//
//------------------------------------------------------------------------------------



class	FrequencyCompound : public SignalProcessor
	{
		
	private:
		
		size_t n_signal_filters;
		
		FilterSet SignalFilters;
//		DataArray<ForceSpectrumFilter> SignalFilters;
		DataArray<ComplexFunction2D_F32>	frame_buffer;

				
		virtual	void	Batch();

		virtual	void	ProcessDynamicFilters();
		virtual	void	ProcessStaticFilters();

		void	FilterFrame(size_t n, ComplexFunction2D_F32 &buffer);
		virtual	void	InitWork();


	public:	
		FrequencyCompound();	
//		virtual ~FrequencyCompound();	
		
	};


XRAD_END

	
#endif //__frequency_compound_h
