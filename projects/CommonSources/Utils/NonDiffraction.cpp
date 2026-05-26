#include "pre.h"
#include <omp.h>
#include <Utils/NonDiffraction.h>
#include <XRADBasic/ContainersAlgebra.h>
#include <XRADBasic/FFT2D.h>


XRAD_BEGIN

non_diffraction_setup :: non_diffraction_setup()
	{
	algorithm = fast_segment_algorithm;
	zeta_factor = -1;
	}


void	FocusNonDiffraction(ComplexFunction2D_F32 &frame, const non_diffraction_setup &setup)
	{
	NonDiffractionFocuser	focuser(setup);
	focuser.Init(frame);
	
	switch(setup.algorithm)
		{
		case slow_ft_algorithm:
			focuser.FocusNonDiffractionSlowFT(frame);
			break;
		
		case fast_segment_algorithm:
			focuser.FocusNonDiffractionRanges(frame);
			break;		
		};
	}

//--------------------------------------------------------------
//
//	NonDiffractionFocuser initialization
//
//--------------------------------------------------------------

NonDiffractionFocuser :: NonDiffractionFocuser(const non_diffraction_setup &s) : parent(s)
	{
	array_size = array_pitch*n_elements;
	aperture_size = array_pitch*n_aperture_elements;
	}


void	NonDiffractionFocuser :: Init(ComplexFunction2D_F32 &frame)
	{
	size_t	n_rays = frame.vsize();
	size_t	n_samples = frame.hsize();

	dz = (r_max - r_min)/n_samples;

	if(convex_radius.cm()) use_exact_zeta_formula = false;
//	else	use_exact_zeta_formula = YesOrNo("Use exact formula?", NO);	
	else	use_exact_zeta_formula = true;
	
	if(convex_radius.cm()) dx = array_size/(convex_radius*n_rays);
	else	dx = array_size.cm()/n_rays;

	//TODO в случае конвексной решетки используетсЯ полЯрнаЯ система координат, а для линейки декартова. это плохо, подумать как исправить
	}

//--------------------------------------------------------------
//
//	NonDiffractionFocuser focusing algorithms
//
//--------------------------------------------------------------



void	NonDiffractionFocuser :: FocusNonDiffractionSlowFT(ComplexFunction2D_F32 &frame)
	{
	size_t	n_rays = frame.vsize();
	size_t	n_samples = frame.hsize();
	
	size_t n_samples_ft = ceil_fft_length(n_samples);
	size_t n_rays_ft = ceil_fft_length(n_rays);
	
	if(double(n_rays_ft)/n_rays < 1.5) n_rays_ft *= 2;
		// этим создаем  буферную нулевую зону, чтобы исключить
		// циклические артефакты при бпф

	focusing_factors.realloc(n_rays_ft, n_samples_ft);
	fft_buffer.realloc(n_rays_ft, n_samples_ft);
	fft_buffer.CopyData(frame);
	FFTf(fft_buffer, fftFwd, fftFwdRollAfter);//2016_09_16
	
	// это алгоритм, в котором вместо бпф выполняется медленное обратное преобразование фурье
	
	ComplexFunction2D_F32	spectra2(n_rays_ft, n_samples_ft, complexF32(0));
	RealFunction2D_F32	zetas(n_samples, n_samples_ft);
	
	// вычисляется zeta для всех z, omega
	for(size_t	sample = 0; sample < n_samples; sample ++)
		{
		physical_length	target_focus;
		physical_length	original_focus;
		double	range_index = double(sample)/n_samples;
		
		original_focus = original_focuses.in(range_index*original_focuses.size(), &interpolators::linear);
		if(target_focuses.size())
			{
			target_focus = target_focuses.in(range_index*target_focuses.size(), &interpolators::linear);
			}
		else
			{
			physical_length	distance = r_min + sample*dz;
			if(distance.cm() == 0) distance = cm(0.001);
			target_focus = distance;
			}
		for(size_t harmonic_t = 0; harmonic_t < n_samples_ft; harmonic_t ++)
			{
			physical_frequency	omega = sample_rate * double(harmonic_t)/n_samples_ft;			
						
			if(!use_exact_zeta_formula) zetas.at(sample,harmonic_t) = zeta_original_formula(omega, original_focus, target_focus);
			else zetas.at(sample,harmonic_t) = zeta_exact_formula(omega, original_focus, target_focus);
			}
		}
	zetas *= zeta_factor;

	const double	d_OMEGA = two_pi()/(dx*n_rays_ft);
	const double	OMEGA_0 = -(d_OMEGA*n_rays_ft)/2.;
	
	GUIProgressBar	progress;
	progress.start("Focusing (slow Fourier algorithm)", n_samples_ft);
	for(size_t harmonic_t = 0; harmonic_t < n_samples_ft; ++harmonic_t)
		{
		physical_frequency	omega = sample_rate * double(harmonic_t)/n_samples_ft;			

 		#pragma omp parallel for schedule (guided)
		for(int	sample = 0; sample < int(n_samples); ++sample)
			{
			double	relative_depth = double(sample)/n_samples_ft;
			double	depth_phasor = two_pi()*relative_depth*harmonic_t;

			double	zeta = zetas.at(sample,harmonic_t);
									
			ComplexFunctionF32::iterator	result_it = spectra2.col(sample).begin();
			ComplexFunctionF32::iterator	spectrum_it = fft_buffer.col(harmonic_t).begin();
			
			for(size_t harmonic_s = 0; harmonic_s < n_rays_ft; ++harmonic_s)
				{
				double	OMEGA_sq = square(OMEGA_0 + harmonic_s*d_OMEGA);
				
// 				spectra2[harmonic_s,sample] += fft_buffer.at(harmonic_s,harmonic_t) * polar(1, zeta*OMEGA_sq + depth_phasor);
				*result_it += *spectrum_it * polar(1, zeta*OMEGA_sq + depth_phasor);
				++result_it, ++spectrum_it;
				}
			}
		++progress;
		}
	FFTf(spectra2, fftNone, fftRevRollBefore);//2016_09_16
	frame.CopyData(spectra2);
	}

void	NonDiffractionFocuser :: FocusNonDiffractionRanges(ComplexFunction2D_F32 &frame)
	{
	size_t	n_rays = frame.vsize();
	size_t	n_samples = frame.hsize();
	
	physical_length	range_size;//cm
	
	size_t	range_samples;
	double	range_step;
	size_t	n_ranges;
	
	if(target_focuses.size())
		{
		n_ranges = target_focuses.size();
		range_size = 2*(r_max-r_min)/(n_ranges+1);//cm
		}
	else
		{
		//TODO длину интервала фокусировки по умолчанию перенести в настройки
		range_size = cm(0.25);//cm
		n_ranges = 2*round((r_max-r_min)/range_size) + 1;
		target_focuses.realloc(n_ranges);
		
		for(size_t i = 0; i < n_ranges; ++i)
			{
			target_focuses[i] = r_min + (i+1)*range_size/2;
			}
		}
	
	range_step = double(n_samples)/(n_ranges+1);
	range_samples = 2*range_step;

	

	size_t	range_samples_ft = ceil_fft_length(range_samples);
	

	
	ComplexFunction2D_F32	seg_ref;
	ComplexFunction2D_F32	result(n_rays, n_samples, complexF32(0));
	
	GUIProgressBar	progress;
	progress.start("Focusing ranges", n_ranges);
	
	for(size_t range = 0; range < n_ranges; range++)
		{
		size_t	range_start = range * range_step;
		ComplexFunction2D_F32	seg(n_rays, range_samples_ft);
		seg_ref.UseDataFragment(frame, 0, range_start, n_rays, min(range_start + range_samples, n_samples));
		seg.CopyData(seg_ref);
		
		for(size_t sample = 0; sample < range_samples; sample++)
			{
			double	x = two_pi()*double(sample-range_step)/range_samples;
			if(range == 0 && x < 0){}
			else if(range == n_ranges-1 && x > 0){}
		//	else seg.col(sample) *= (1.+cos(x))/2;
			else seg.col(sample) *= sqrt(1.+cos(x))/2;
			}
		// косинусное окно применЯем в два этапа, чтобы избежать скачка, который может возникнуть
		// в результате обработки
		
//		physical_length	new_focus = r_min + range_start*dz + range_samples*dz/2;//середина интервала
		physical_length	new_focus = target_focuses.in(target_focuses.size()*double(range)/n_ranges, &interpolators::linear);
		physical_length	original_focus = original_focuses.in(original_focuses.size()*double(range)/n_ranges, &interpolators::linear);
		
		FocusRangeFFT(seg, original_focus, new_focus);

		for(size_t sample = 0; sample < range_samples; sample++)
			{
			double	x = two_pi()*double(sample - range_step)/range_samples;
			if(range == 0 && x < 0){}
			else if(range == n_ranges-1 && x > 0){}
		//	else seg.col(sample) *= (1.+cos(x))/2;
			else seg.col(sample) *= sqrt(1.+cos(x))/2;
			}

		seg_ref.UseDataFragment(result, 0, range_start, n_rays, min(range_start + range_samples, n_samples));
		seg.resize(n_rays, min(range_start + range_samples, n_samples) - range_start);
		seg_ref += seg;
		++progress;
		}
	frame.CopyData(result);
	
	original_focuses.MakeCopy(target_focuses);
	}

void	NonDiffractionFocuser :: FocusRangeFFT(ComplexFunction2D_F32 &frame, physical_length original_focus, physical_length target_focus)
	{
	size_t	n_rays = frame.vsize();
	size_t	n_samples = frame.hsize();
	
	size_t n_samples_ft = ceil_fft_length(n_samples);
	size_t n_rays_ft = ceil_fft_length(n_rays);


	if(double(n_rays_ft)/n_rays < 1.5) n_rays_ft *= 2;
		// этим создаем  буферную нулевую зону, чтобы исключить
		// циклические артефакты при бпф

	focusing_factors.realloc(n_rays_ft, n_samples_ft);
	fft_buffer.realloc(n_rays_ft, n_samples_ft);
	fft_buffer.CopyData(frame);
	FFTf(fft_buffer, fftFwd, fftFwdRollAfter);//2016_09_16
	
	physical_frequency	d_omega = sample_rate/n_samples_ft;
	for(size_t harmonic_t = 0; harmonic_t < n_samples_ft; harmonic_t ++)
		{
		physical_length	distance = target_focus;
		double	zeta;
		
		physical_frequency	omega = harmonic_t * d_omega;
		
		if(distance.cm() == 0) distance = cm(0.001);
		
		if(!use_exact_zeta_formula) zeta = zeta_original_formula(omega, original_focus, target_focus);
		else zeta = zeta_exact_formula(omega, original_focus, target_focus);
					
		zeta *= zeta_factor;
		
		double	d_OMEGA = two_pi()/(dx*n_rays_ft);
		double	OMEGA_0 = -double(n_rays_ft)*d_OMEGA/2.;
		for(size_t harmonic_s = 0; harmonic_s < n_rays_ft; harmonic_s ++)
			{
			double	OMEGA_sq = square(OMEGA_0 + harmonic_s*d_OMEGA);
			
//			fft_buffer.at(harmonic_s,harmonic_t) *= polar(1 ,zeta*OMEGA_sq);
			focusing_factors.at(harmonic_s,harmonic_t) = polar(1 ,zeta*OMEGA_sq);
			}		
		}
	fft_buffer *= focusing_factors;
	FFTf(fft_buffer, fftRev, fftRevRollBefore);//2016_09_16
	frame.CopyData(fft_buffer);
	}
	
	
//--------------------------------------------------------------
//
//	NonDiffractionFocuser zeta formulas
//
//--------------------------------------------------------------


double	NonDiffractionFocuser :: zeta_original_formula(physical_frequency omega, physical_length original_focus, physical_length new_focus) const
	{
	double	beta1, beta2;
	double	wave_k = omega.rad_sec()/sound_speed.cm_sec();

//TODO new_focus==original_focus, деление на 0, потом еще раз, поэтому компенсируется и незаметно
	if(!convex_radius.cm())
		{
		beta1 = wave_k/(new_focus - original_focus).cm();
		beta2 = wave_k/new_focus.cm();
		}
	else
		{
		physical_length	f_equivalent = convex_radius*(original_focus/(convex_radius + original_focus));
		physical_length	r_equivalent = convex_radius*(new_focus/(convex_radius + new_focus));
		double	factor = square(convex_radius.cm());

		beta1 = wave_k/(r_equivalent - f_equivalent).cm();
		beta2 = wave_k/r_equivalent.cm();
		
		beta1 *= factor;
		beta2 *= factor;
		}
	
	if(tx_focusing && rx_focusing) beta2 = beta1;
	if(!tx_focusing && !rx_focusing) beta1 = beta2;
						
	if(omega.Hz()) return 0.5/(beta1 + beta2);
	else return 0;
	}

double	NonDiffractionFocuser :: zeta_exact_formula(physical_frequency omega, physical_length original_focus, physical_length new_focus) const
	{
	double	a = .5*aperture_size.cm();
	double	a4 = a*a*a*a;
	double	wave_k = omega.rad_sec()/sound_speed.cm_sec();
	double	zeta=0;
	
	if(tx_focusing && rx_focusing)
		{
		double	k2F = wave_k/(2*original_focus.cm());
		double	k2Z = wave_k/(2*new_focus.cm());
		double	v0 = k2F + 1./(a4*k2F); 
		
		double	Up = k2Z - v0;
		
		if(wave_k) zeta = -Up/(4.*2.*k2Z*v0);
		else zeta = 0;
		}
	
	else if(tx_focusing || rx_focusing)
		{
		double	p1 = 1./new_focus.cm() - 2./original_focus.cm();
		if(wave_k) zeta = .25*wave_k*p1/(wave_k*wave_k*p1*p1 + 16/a4) + new_focus.cm()/(4*wave_k);
		else zeta = 0;
		}
	
	else
		{
		FatalError("No formula for synthetic aperture!");
		}
	
	return zeta;
	}

XRAD_END
