#include "pre.h"

#include <XRADBasic/LinearVectorTypes.h>
#include <XRADBasic/MathMatrixTypes.h>
#include <XRADBasic/MathFunctionTypes2D.h>
#include <XRADBasic/FFT1D.h>

#include <RFDataImport/S500_CFMDataTypes.h>
#include <DopplerBasics/EMD.h>

#include <DopplerBasics/WallFiltersInterface.h>


XRAD_BEGIN


WallFilterPtr GetWallFilter(size_t burst_size, size_t option)
{
		WallFilter *wall_filter = NULL;
		enum
		{ 
			wf_LS, 
			wf_average, 
			wf_f0,
			wf_finite_difference, 
			wf_gaussian, wf_butterworth, 
			wf_butterworth_recursive, 
			wf_fourier, wf_legendre, 
			wf_LEMD, wf_CSEMD, 
			wf_kl, 
			wf_bank_amp, 
			wf_bank_phase, 
			No_WF,
			n_options 
		};

	switch (option)
	{
	case wf_average:
		wall_filter = new WallFilterAverage;
		break;

	case wf_f0:
	{
		WallFilterBasis	*wf = new WallFilterBasis;
		RealVectorF64	weights(burst_size, 1);
		weights[0] = 0;
		wf->InitFourierBasis(burst_size, weights);
		wall_filter = wf;
	}
	break;
	case wf_finite_difference:
		wall_filter = new WallFilterFiniteDifference;
		break;
	case wf_gaussian:
	{
		double STD = burst_size*GetFloating("Filter bandwidth", 0.1, 0, 1);
		double power = GetUnsigned("Power", 5, 0, 10);
		wall_filter = new WallFilterGaussian(burst_size, STD, power);
	}
	break;
	case wf_butterworth:
	{
		float cutoff_frequency = burst_size*GetFloating("Cutoff frequency in PRF", 0.02, 0, 0.5);
		float order = GetUnsigned("Order", 2, 0, 100);
		size_t	fft_size = ceil_fft_length(burst_size);
		if (fft_size > size_t(burst_size)) fft_size /= 2;
		WallFilterButterworth *wf = new WallFilterButterworth;
		wf->Init(cutoff_frequency, order, fft_size);
		wall_filter = wf;
	}
	break;
	case wf_butterworth_recursive:
	{
		float cutoff_frequency = 1 - 2 * GetFloating("Cutoff frequency in PRF", 0.02, 0, 0.5);
		WallFilterButterworth_recursive *wf = new WallFilterButterworth_recursive;
		wf->Init(cutoff_frequency);
		wall_filter = wf;
	}
	break;
	case wf_fourier:
	{
		size_t	n_components = GetUnsigned("Number of components to suppress", 1, 0, burst_size / 2);
		WallFilterBasis	*wf = new WallFilterBasis;
		RealVectorF64	weights(burst_size, 1);
		std::fill(weights.begin(), weights.begin() + n_components, 0);
		std::fill(weights.end() - n_components, weights.end(), 0);
		wf->InitFourierBasis(burst_size, weights);
		wall_filter = wf;
	}
	break;
	case wf_legendre:
	{
		size_t	n_components = GetUnsigned("Number of components to suppress", 1, 0, burst_size / 2);
		WallFilterBasis	*wf = new WallFilterBasis;
		RealVectorF64	weights(burst_size, 1);
		std::fill(weights.begin(), weights.begin() + n_components, 0);
		std::fill(weights.end() - n_components, weights.end(), 0);
		wf->InitFourierBasis(burst_size, weights);
		wall_filter = wf;
	}
	break;
	case wf_LEMD:
	{
		size_t	number_of_iterations = GetUnsigned("Number of iterations", 1, 0, 10);
		WallFilterEMD::e_interpolation_type flag(WallFilterEMD::e_linear);
		wall_filter = new WallFilterEMD(flag, number_of_iterations);
	}
	break;
	case wf_CSEMD:
	{
		size_t	number_of_iterations = GetUnsigned("Number of iterations", 1, 0, 10);
		WallFilterEMD::e_interpolation_type flag(WallFilterEMD::e_spline);
		wall_filter = new WallFilterEMD(flag, number_of_iterations);
	}
	break;
	case wf_kl:
	{
		size_t	n_components = GetUnsigned("Number of components to suppress", 1, 0, burst_size);
		RealVectorF64	weights(burst_size, 0);
		std::fill(weights.begin(), weights.end() - n_components, 1);

		enum { frame_average, sweep_average, beam_average, n_options };
		size_t	type = GetButtonDecision("Choose Averaging Type", { "Frame", "Sweep", "Beam" });
		switch (type)
		{
		case frame_average:
		{
			WallFilterKL_Frame	*wf = new WallFilterKL_Frame;
			wf->SetWeights(weights);
			wall_filter = wf;
		}
		break;
		case sweep_average:
		{
			WallFilterKL_Sweep	*wf = new WallFilterKL_Sweep;
			wf->SetWeights(weights);
			wall_filter = wf;
		}
		break;
		case beam_average:
		{
			WallFilterKL_Beam	*wf = new WallFilterKL_Beam;
			wf->SetWeights(weights);
			wall_filter = wf;
		}
		break;
		}
	}
	break;
	case No_WF:
		wall_filter = new WallFilterNone;
		break;
	}
	return WallFilterPtr(wall_filter);
	}


WallFilterPtr	GetWallFilterLS(size_t burst_size)
{
	size_t p_power(2);
	size_t	polynom_power = 1 + GetUnsigned("Polynom power", p_power, 0, 10);
	WallFilterLS *wf = new WallFilterLS;
	wf->Init(burst_size, polynom_power);
	return WallFilterPtr(wf);
}

WallFilterPtr	GetWallFilterNone(size_t)
{
	return make_shared<WallFilterNone>();
}


WallFilterPtr	GetWallFilterF0(size_t burst_size)
{
	WallFilterBasis	*wf = new WallFilterBasis;
	RealVectorF64	weights(burst_size, 1);
	weights[0] = 0;
	wf->InitFourierBasis(burst_size, weights);
	return WallFilterPtr(wf);
}

WallFilterPtr	GetWallFilterAverage(size_t)
{
	return make_shared<WallFilterAverage>();
}

WallFilterPtr	GetWallFilterFiniteDifference(size_t)
{
	return make_shared<WallFilterFiniteDifference>();
}

WallFilterPtr	GetWallFilterGaussian(size_t burst_size)
{
	double STD = burst_size*GetFloating("Filter bandwidth", 0.1, 0, 1);
	double power = GetUnsigned("Power", 5, 0, 10);
	return make_shared<WallFilterGaussian>(burst_size, STD, power);
}

WallFilterPtr	GetWallFilterButterworth(size_t burst_size)
{
	float cutoff_frequency = burst_size*GetFloating("Cutoff frequency in PRF", 0.02, 0, 0.5);
	float order = GetUnsigned("Order", 2, 0, 100);
	size_t	fft_size = ceil_fft_length(burst_size);
	if (fft_size > size_t(burst_size)) fft_size /= 2;
	WallFilterButterworth *wf = new WallFilterButterworth;
	wf->Init(cutoff_frequency, order, fft_size);
	return WallFilterPtr(wf);
}

WallFilterPtr	GetWallFilterButterworthRecursive(size_t burst_size)
{
	float cutoff_frequency = 1 - 2 * GetFloating("Cutoff frequency in PRF", 0.02, 0, 0.5);
	WallFilterButterworth_recursive *wf = new WallFilterButterworth_recursive;
	wf->Init(cutoff_frequency);
	return WallFilterPtr(wf);
}

WallFilterPtr	GetWallFilterFourier(size_t burst_size)
{
	size_t	n_components = GetUnsigned("Number of components to suppress", 1, 0, burst_size / 2);
	WallFilterBasis	*wf = new WallFilterBasis;
	RealVectorF64	weights(burst_size, 1);
	std::fill(weights.begin(), weights.begin() + n_components, 0);
	std::fill(weights.end() - n_components, weights.end(), 0);
	wf->InitFourierBasis(burst_size, weights);
	return WallFilterPtr(wf);
}

WallFilterPtr	GetWallFilterLegendre(size_t burst_size)
{
	size_t	n_components = GetUnsigned("Number of components to suppress", 1, 0, burst_size / 2);
	WallFilterBasis	*wf = new WallFilterBasis;
	RealVectorF64	weights(burst_size, 1);
	std::fill(weights.begin(), weights.begin() + n_components, 0);
	std::fill(weights.end() - n_components, weights.end(), 0);
	wf->InitFourierBasis(burst_size, weights);
	return WallFilterPtr(wf);
}


WallFilterPtr	GetWallFilterLEMD(size_t burst_size)
{
	size_t	number_of_iterations = GetUnsigned("Number of iterations", 1, 0, 10);
	WallFilterEMD::e_interpolation_type flag(WallFilterEMD::e_linear);
	return make_shared<WallFilterEMD>(flag, number_of_iterations);
}

WallFilterPtr	GetWallFilterSEMD(size_t burst_size)
{
	size_t	number_of_iterations = GetUnsigned("Number of iterations", 1, 0, 10);
	WallFilterEMD::e_interpolation_type interpolation_type(WallFilterEMD::e_spline);
	return make_shared<WallFilterEMD>(interpolation_type, number_of_iterations);
}

WallFilterPtr	GetWallFilterKL(size_t burst_size)
{
	size_t	n_components = GetUnsigned("Number of components to suppress", 1, 0, burst_size);
	RealVectorF64	weights(burst_size, 0);
	std::fill(weights.begin(), weights.end() - n_components, 1);

	enum { frame_average, sweep_average, beam_average, n_options };
	size_t	type = GetButtonDecision("Choose Averaging Type", {"Frame", "Sweep", "Beam"});
	switch(type)
	{
		case frame_average:
		{
			WallFilterKL_Frame	*wf = new WallFilterKL_Frame;
			wf->SetWeights(weights);
			return WallFilterPtr(wf);
		}
		break;
		case sweep_average:
		{
			WallFilterKL_Sweep	*wf = new WallFilterKL_Sweep;
			wf->SetWeights(weights);
			return WallFilterPtr(wf);
		}
		break;
		case beam_average:
		{
			WallFilterKL_Beam	*wf = new WallFilterKL_Beam;
			wf->SetWeights(weights);
			return WallFilterPtr(wf);
		}
		break;
		default:
			throw invalid_argument("Unknown wall-filter type");
	}
}



WallFilterPtr GetWallFilterType(size_t burst_size)
{
	/*
	auto	fn = GetButtonDecision("Select string", 
		{ 
			MakeButton("Least Squares", make_fn(GetWallFilterLS)),
			MakeButton("Average", make_fn(GetWallFilterAverage)),
			MakeButton("F0", make_fn(GetWallFilterF0)),
			MakeButton("Finite Difference", make_fn(GetWallFilterFiniteDifference)),
			MakeButton("Gaussian", make_fn(GetWallFilterGaussian)),
			MakeButton("Butterworth", make_fn(GetWallFilterButterworth)),
			MakeButton("Butterworth Recursive", make_fn(GetWallFilterButterworthRecursive)),
			MakeButton("Fourier", make_fn(GetWallFilterFourier)),
			MakeButton("Legendre", make_fn(GetWallFilterLegendre)),
			MakeButton("Linear EMD", make_fn(GetWallFilterLEMD)),
			MakeButton("Spline EMD", make_fn(GetWallFilterSEMD)),
			MakeButton("Karhunen-Loeve", make_fn(GetWallFilterKL)),
			MakeButton("None", make_fn(GetWallFilterNone)),			
		});

	return fn(burst_size);

	*/
		size_t	option = 0;
	enum {wf_LS,
	wf_average,
	wf_f0,
	wf_finite_difference,
	wf_gaussian,
	wf_butterworth,
	wf_butterworth_recursive,
	wf_fourier,
	wf_legendre,
	wf_LEMD,
	wf_CSEMD,
	wf_kl,
	No_WF, n_options};
	
	option = GetButtonDecision("Choose filter type", 
	{
		"Least Squares",
		"Average",
		"F0",
		"Finite Difference",
		"Gaussian",
		"Butterworth",
		"Butterworth Recursive",
		"Fourier",
		"Legendre",
		"Linear EMD",
		"Spline EMD",
		"Karhunen-Loeve",
		"No WF"
	});
	WallFilterPtr	wall_filter = GetWallFilter(burst_size, option);
//	return WallFilterPtr(wall_filter);
	return wall_filter;
	
}

XRAD_END