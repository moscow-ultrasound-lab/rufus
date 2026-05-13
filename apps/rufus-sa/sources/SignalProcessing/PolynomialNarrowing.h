#ifndef	__polynomial_narrowing_h
#define	__polynomial_narrowing_h

#include "SyntheticFocusingOptions.h"
#include "LUT_8bit_interpolator.h"
#include "SyntheticApertureFocuser.h"
#include "RawSFDataSourceTransEcho.h"
#include "SyntheticApertureFocuserAina.h"
//#include "ApertureFocusingOptionsOld.h"

XRAD_BEGIN


RealFunctionF64	PolynomBasis(int number_of_transmit_elements, size_t no, double magn);
RealFunctionF64	HarmonicBasis(int number_of_transmit_elements, double freq, double magn, double phi_0);
RealFunctionF64	CosX_1_Basis(int number_of_transmit_elements, double freq, double magn, int center);



class CorrectionTraits 
{
public:
	CorrectionTraits(int in_n_elements, int in_n_rays) :
		//lineGS(L"", L"", L""),
		lineGS(L"Line", L"value", L"degrees"),
		errorGS(L"Error", L"value", L"amp"),
		approximationGS(L"Correction function", L"value", L"element number"),
		phaseGS(L"Phase", L"degrees", L""),
		n_elements(in_n_elements),
		n_rays(in_n_rays)
	{
		Init();
	}

	void	Init()//аргументами являются варианты стратегий
	{
		best_fit_matrix_correction.realloc(n_rays, n_elements);
		matrix_correction.realloc(n_rays, n_elements);
		best_fit_gauss_function.realloc(n_rays);
		best_fit_line.realloc(n_rays, n_rays);
		rays_corrected.realloc(n_rays);
		ray_to_correct.realloc(n_rays);
		best_fit.realloc(n_rays);

		approximationGS.SetScale(range2_F64(-amplitude_margin * 2, 0, amplitude_margin * 2, n_elements));
		errorGS.SetScale(range2_F64(0, -amplitude_margin, 20, amplitude_margin));
		error_line.realloc(int(2 * amplitude_margin / amplitude_step) + 1);
		approximationGS.SetGraphStyle(solid_color_lines, 5.5);
		lineGS.SetGraphStyle(solid_color_lines, 5.5);

		best_fit_gauss_function.fill(0);
		best_fit_line.fill(0);
		error_line.fill(0);
		best_fit.fill(0);
	}

	GraphSet	lineGS;
	GraphSet	errorGS;
	GraphSet	approximationGS;
	GraphSet	phaseGS;
	
	const int	n_rays;
	const int	n_elements;

	RealFunction2D_F64 matrix_correction;
	RealFunction2D_F64 best_fit_matrix_correction;

	RealFunctionF32 rays_corrected;
	RealFunctionF32 ray_to_correct;

	int first_sample;
	int last_sample;
	int n_lines;
	
	RealFunctionF64 min_error;

	GUIProgressBar prog;

	bool last_inside_empirical_margin_flag;
	bool inside_empirical_margin_flag;

	double amplitude;
	double amplitude_margin = 1e-7;
	double amplitude_step = 1e-8;
	double current_frequency;

	size_t ray_no;

	RealFunctionF64 line;
	RealFunctionF64 gauss_function;

	double empirical_margin = 3;
	double best_fit_sq_error;

	RealFunctionF64 error_line;


	RealFunction2D_F64 best_fit_line;

	RealFunctionF64 best_fit_gauss_function;
	RealFunctionF64 correction;

	RealFunctionF32 best_fit;



	physical_angle half_scanning_sector;
	physical_angle scanning_sector; 
	physical_angle fixed_angle_transmit;
	size_t number_of_rays;

	RealFunctionF32 best_result;

	bool best_correction_flag;
	size_t number_of_iterations;

};

XRAD_END

#endif