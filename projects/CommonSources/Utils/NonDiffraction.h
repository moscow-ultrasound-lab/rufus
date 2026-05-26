#ifndef __non_diffraction_h
#define __non_diffraction_h

#include <XRADBasic/ContainersAlgebra.h>
#include <XRADBasic/Sources/Utils/PhysicalUnits.h>

XRAD_BEGIN

enum	non_diffraction_algorithm
	{
	fast_segment_algorithm,
	slow_ft_algorithm,
	};

struct non_diffraction_setup
	{
	physical_length	r_min, r_max;
	size_t	n_elements;
	size_t	n_aperture_elements;
	physical_length	array_pitch;
	physical_length	convex_radius;
	
	
	MathFunction<physical_length, double, AlgebraicStructures::FieldTagScalar>	original_focuses;
	MathFunction<physical_length, double, AlgebraicStructures::FieldTagScalar>	target_focuses;
	
	
	physical_frequency	sample_rate; //MHz
	physical_speed	sound_speed;

	double	zeta_factor;
		// can be 1 or -1 depending on signal phase

	bool	tx_focusing, rx_focusing;	
	non_diffraction_algorithm algorithm;
	
	non_diffraction_setup();
	};

class	NonDiffractionFocuser : public non_diffraction_setup
	{
	PARENT(non_diffraction_setup);
//	size_t	n_rays;
//	size_t	n_samples;
	
	physical_length	array_size;
	physical_length	aperture_size;
	double	dx;// система единиц длЯ него неопределена!!!!!
	physical_length	dz;
	
	bool	use_exact_zeta_formula;
	ComplexFunction2D_F32	fft_buffer;
	ComplexFunction2D_F32	focusing_factors;

	double	zeta_exact_formula(physical_frequency omega, physical_length original_focus, physical_length target_focus) const;
	double	zeta_original_formula(physical_frequency omega, physical_length original_focus, physical_length target_focus) const;


public:	
	NonDiffractionFocuser(const non_diffraction_setup &s);

	void	Init(ComplexFunction2D_F32 &frame);

	void	FocusRangeFFT(ComplexFunction2D_F32 &frame, physical_length original_focus, physical_length target_focus);
	void	FocusNonDiffractionSlowFT(ComplexFunction2D_F32 &frame);
	void	FocusNonDiffractionRanges(ComplexFunction2D_F32 &frame);
	};

void	FocusNonDiffraction(ComplexFunction2D_F32 &frame, const non_diffraction_setup &setup);

XRAD_END

#endif // __non_diffraction_h
