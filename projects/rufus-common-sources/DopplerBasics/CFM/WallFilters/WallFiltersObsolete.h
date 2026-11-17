#ifndef __WallFiltersObsolete_h
#define __WallFiltersObsolete_h

//------------------------------------------------------------------
//
//	created:	2021/03/10	15:20
//	filename: 	WallFiltersObsolete.h
//	file path:	Q:\Projects\CommonSources\DopplerBasics\CFM\WallFilters
//	author:		kns
//	
//	purpose:	
//
//------------------------------------------------------------------

#include "WallFilters.h"

namespace xrad
{


// фильтр с фурье-преобразованием, всех следующих перевести на него
class WallFilterFourier : public WallFilter
{
protected:
	size_t	fft_size;
	mutable ComplexFunctionF32	fft_buffer;
	mutable RealFunctionF64	filter;

public:
	virtual	bool is_size_actual(size_t in_vector_size) const override { return fft_size == in_vector_size; }

	WallFilterFourier(size_t burst_size);
	void Apply(ComplexFunctionF32& burst) const;
	virtual	wstring	filter_name() const override { return ssprintf(L"WallFilterFourier, fft size = %zu, filter TODO if needed", fft_size); };
};

struct WallFilterF0 : WallFilterFourier
{
	WallFilterF0(size_t burst_size) : WallFilterFourier(burst_size){ filter.fill(1); filter[0] = 0; }
	virtual	wstring	filter_name() const override { return L"WallFilterF0"; };
};


struct WallFilterGaussian : public WallFilterFourier
{
private:
	double bandwidth, power;
	void	UpdateFilter() const;

public:
	WallFilterGaussian(size_t burst_size, double in_STD, double in_power);
	void Apply(ComplexFunctionF32& burst) const;
};

struct WallFilterButterworth : WallFilter
{
	float cutoff_frequency, order;
	RealFunctionF64	filter;
	ComplexFunctionF64 cfilter;
	size_t fft_size;

public:

	virtual	bool is_size_actual(size_t in_vector_size) const override { throw invalid_argument("Butterworth filter is obsolete"); }

	WallFilterButterworth() {}
	void Init(float in_cutoff_frequency, float in_order, size_t in_fft_size);
	void Apply(ComplexFunctionF32& burst) const;
	virtual	wstring	filter_name() const override { return ssprintf(L"WallFilterButterworth, cutoff = %g, order = %g (TODO: see comment in code)", cutoff_frequency, order); };
	// Понять, чем отличается этот баттерворт от следующего (recursive)
};

struct WallFilterButterworth_recursive : WallFilter
{
	float cutoff_frequency;
	RealFunctionF64 b, a;
public:

	virtual	bool is_size_actual(size_t in_vector_size) const override { throw invalid_argument("Butterworth filter is obsolete"); }

	WallFilterButterworth_recursive() {}
	void Init(float in_cutoff_frequency);
	void Apply(ComplexFunctionF32& burst) const;
	virtual	wstring	filter_name() const override { return ssprintf(L"WallFilterButterworth, cutoff = %g (TODO: see comment in code)", cutoff_frequency); };
	// Понять, чем отличается этот баттерворт от предыдущего (не-recursive)
};




struct WallFilterKL : WallFilterBasis
{
	void UpdateFilter(ComplexFunctionMD_F32& frame_by_shots, size_t first_beam, size_t last_beam);
	virtual	wstring	filter_name() const override { return L"WallFilterKL"; };
};

struct WallFilterKL_Frame : WallFilterKL
{
	virtual update_mode	GetUpdateMode() override { return update_mode::each_frame; }
	virtual	wstring	filter_name() const override { return L"WallFilterKL each frame"; };
};

struct WallFilterKL_Sweep : WallFilterKL
{
	virtual update_mode	GetUpdateMode() override { return update_mode::each_sweep; }
	virtual	wstring	filter_name() const override { return L"WallFilterKL each sweep"; };
};

struct WallFilterKL_Beam : WallFilterKL
{
	virtual update_mode	GetUpdateMode() override { return update_mode::each_beam; }
	virtual	wstring	filter_name() const override { return L"WallFilterKL each beam"; };
};





}//namespace xrad

#endif //__WallFiltersObsolete_h
