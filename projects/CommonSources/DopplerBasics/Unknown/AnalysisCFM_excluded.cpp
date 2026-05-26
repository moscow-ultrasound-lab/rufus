#include "pre.h"

#include <XRADBasic/Sources/Utils/ExponentialBlurAlgorithms.h> 
#include <XRADBasic/Sources/Utils/TimeProfiler.h>

#include <RFDataImport/S500_CFMRawDataDisplay.h>

#include <DopplerBasics/Analysis.h>
#include <DopplerBasics/AnalysisCFM.h>
#include <DopplerBasics/DopplerDataDisplay.h>
#include <DopplerBasics/CFDataPhaseAnalyzer.h>
//#include "ElastoOffsetDisplayer.h>
#include <DopplerBasics/CFDataPhaseAnalyzerCFM.h>
#include <DopplerBasics/CFDataPhaseAnalyzerTwinkling.h>
#include <DopplerBasics/CFDataPhaseAnalyzerElasto.h>

#include <DopplerBasics/WallFiltersInteractive.h>

//	Экспериментальный код из AnalysisCFM, назначение неясно, вынесен сюда, чтобы не удалять совсем


XRAD_BEGIN

#if 0


#if 1
/*
double	 CFMProcessor::ProcessFrames(RealFunction2D_F32 &elastogram, RealFunction2D_F32 &mask)
{
	tp_wall_filter.Start();

	TissueMotionCompensation();
	PrepareForWallFiltering();
	//	WallFilterApply();
	//CalculateMaskCFM(mask);
	tp_wall_filter.Stop();

	tp_conjugate.Start();
	ConjugateRFData();
	tp_conjugate.Stop();

	tp_blur.Start();
	BlurRFData();
	tp_blur.Stop();

	tp_acquire.Start();
//	double average_offset = AcquirePhaseCFM(elastogram);
	tp_acquire.Stop();

	tp_postfilter.Start();
	PostfilterElastogram(elastogram);
	tp_postfilter.Stop();

	++n_frames_acquired;

	return 0;
}

*/

#else

CFMProcessor::CFMProcessor( const S500_CFMFrameSet &in_data ) : data(in_data.cfm_frames), n_beams(in_data.n_cfm_beams), 
			n_shots(in_data.n_cfm_shots), n_samples(in_data.n_cfm_samples),
			n_frames(in_data.n_frames), n_beams_in_sweep(in_data.n_cfm_beams_in_sweep)
{
	data_by_shots.UseData(&data.at({0,0,0}), {n_frames, n_beams, n_shots, n_samples}, 1);
	/*
	shots_array_t	arr(n_rays, n_samples);
	size_t	frame(0);
	for (size_t ray = 0; ray < n_beams; ++ray)
	{
		for (size_t sample = 0; sample < n_samples; ++sample)
		{
			data_by_shots.GetRow(arr.at(ray,sample), {frame, ray, slice_mask(0), sample});
		}
	}
	*/
	numerators.realloc({n_frames, n_beams, n_samples});
	denominators.realloc({n_frames, n_beams, n_samples});
	numerators_re_im.realloc({ n_frames, n_beams, n_samples });
	denominators_re_im.realloc({ n_frames, n_beams, n_samples });

	cfm_phase_frames.realloc({n_frames, n_beams, n_samples});
	power_frames.realloc({n_frames, n_beams, n_samples});
	turbulence_frames.realloc({n_frames, n_beams, n_samples});

	correlation_criteria.realloc({n_frames, n_beams, n_samples});
	correlation_re_im_criteria.realloc({ n_frames, n_beams, n_samples });
	stddev_criteria_conj.realloc({n_frames, n_beams, n_samples});
	average_criteria.realloc({n_frames, n_beams, n_samples});
	tdi_frames.realloc({n_frames, n_beams, n_samples});

	flag_tissue_motion_compensation = false;
	flag_blur_correlation_parts = false;
	flag_blur_std = false;
}


void	CFMProcessor::ProcessFrames(WallFilter &wall_filter)
{
  	GetPreProcType();
//	CalculateAverageCriterium();
//	CalculateTDI(); 
	if (flag_tissue_motion_compensation) TissueMotionCompensation();

	//TODO понять, не надо ли std критерий считать после фильтра?

	if (!wall_filter.IsDummy()) PrepareForWallFiltering(wall_filter);
	CalculateSTDCriterium();
	CalculateCorrelationParts();
	CalculateCorrelationCriteria();
	CalculateCorrelationReImParts();
	CalculateCorrelationReImCriteria();
	CalculateCFMFrames(); 
	CalculatePowerFrames(); 
	CalculateTurbulenceFrames();
}

template<class T>
void CFMProcessor::ChangeLastBeam(DataArrayMD<T> &frameset)
{
	typename DataArrayMD<T>::slice_type slice;

	for (size_t frame_no = 0; frame_no < n_frames; ++frame_no)
	{
		frameset.GetSlice(slice, { frame_no, slice_mask(0), slice_mask(1) });

		for (size_t beam_no = 0; (beam_no+1) < n_beams; ++beam_no)
		{
			if ((beam_no+1) % n_beams_in_sweep == 0)
			{
				slice[beam_no+1].CopyData(slice[beam_no]);
			//	slice[beam_no+2].CopyData(slice[beam_no]);
			//	slice[beam_no+3].CopyData(slice[beam_no]);
			}
		}
	}
}


void CFMProcessor::PrepareForWallFiltering(WallFilter &wall_filter)
{
	size_t	filter_update_period_size;
	switch (wall_filter.UpdateMode())
	{
		default:
		case WallFilter::each_frame:
			filter_update_period_size = n_beams;
			break;
		case WallFilter::each_beam:
			filter_update_period_size = 1;
			break;
		case WallFilter::each_sweep:
			filter_update_period_size = n_beams_in_sweep;
			break;
	};
	size_t	n_update_periods = n_beams / filter_update_period_size;
	
	StartProgress("Wall filtering", n_frames);
	TimeProfiler	tp;
	for (size_t frame_no = 0; frame_no < n_frames; ++frame_no)
	{
		tp.Start();
		ComplexFunctionMD_F32 cfm_slice_frame_by_shots;
		data_by_shots.GetSubset(cfm_slice_frame_by_shots, {frame_no, slice_mask(0), slice_mask(1), slice_mask(2)});
		for (size_t period_no = 0; period_no < n_update_periods; ++period_no)
		{
			wall_filter.UpdateFilter(cfm_slice_frame_by_shots, period_no*filter_update_period_size, (period_no + 1)*filter_update_period_size);
			ApplyWallFilter(wall_filter, frame_no, period_no*filter_update_period_size, (period_no + 1)*filter_update_period_size);
		}
		tp.Stop();
		NextProgress();
	}
	EndProgress();
//	ScanConverterOptions sco(ScanFrameRectangle(cm(10), cm(10)));
//	DisplayCFMDetailed(data, n_shots, ssprintf("After WF, dt=%g ms", tp.MeanElapsed().msec()), sco);
}

void CFMProcessor::ApplyFrameAveraging(RealFunctionMD_F32	&real_data, double &blur_time)
{
	RealFunction2D_F32 averaging_buffer(n_beams, n_samples, 0);
	double	a0 = blur_time;
	double	a1 = 1. - blur_time;
	if (a0)
	{
		double	a1_a0 = a1 / a0;

		StartProgress("Frame Averaging", n_frames);
		for (size_t frame_no = 0; frame_no < n_frames; ++frame_no)
		{
			RealFunctionMD_F32::slice_type cfm_slice;
			real_data.GetSlice(cfm_slice, { frame_no, slice_mask(0), slice_mask(1) });
			// применяем рекурсивный фильтр 1-го порядка
			for (size_t i = 0; i < cfm_slice.vsize(); ++i)
			{
				averaging_buffer[i] *= a1_a0;
				averaging_buffer[i] += cfm_slice[i];
				averaging_buffer[i] *= a0;
				cfm_slice[i].CopyData(averaging_buffer[i]);
			}
			NextProgress();
		}
		EndProgress();
	}
}

void CFMProcessor::TissueMotionCompensation()
{
	StartProgress("Tissue Motion Compensation", n_frames);
	TimeProfiler	tp;
	for(size_t frame_no = 0; frame_no < n_frames; ++frame_no)
	{
		tp.Start();
		CFDataPhaseAnalyzer::shots_array_t	shots_array(n_beams, n_samples);
		for (size_t beam_no = 0; beam_no < n_beams; ++beam_no)
		{
			for (size_t sample_no = 0; sample_no < n_samples; ++sample_no)
			{
				data_by_shots.GetRow(shots_array.at(beam_no,sample_no), { frame_no, beam_no, slice_mask(0), sample_no });
			}
		}
		TissueMotionCompensationOneFrame(shots_array, n_beams_in_sweep, n_shots, tissue_motion_blur, acceleration_flag);
		tp.Stop();
		NextProgress();
	}
	EndProgress();
	if(CapsLock())ShowText("Tissue Motion Compensation", ssprintf("Time per frame dt=%g ms", tp.MeanElapsed().msec()));
}

void CFMProcessor::CalculateTDI()
{
	ComplexFunctionF32	burst;
	RealFunctionMD_F32::slice_type tdi_slice;

	StartProgress("Calculating TDI", n_frames);
	for(size_t frame_no = 0; frame_no < n_frames; ++frame_no)
	{
		tdi_frames.GetSlice(tdi_slice, {frame_no, slice_mask(0), slice_mask(1)});
		for(size_t sample = 0; sample < n_samples; ++sample)
		{
			for(size_t beam_no = 0; beam_no < n_beams; ++beam_no)
			{
				data_by_shots.GetRow(burst, {frame_no, beam_no, slice_mask(0), sample});
				complexF64	complex_accumulator(0);

				for(size_t shot_no = 0; shot_no < n_shots-1; ++shot_no)
				{
					complex_accumulator.add_multiply_conj(burst[shot_no], burst[shot_no+1]);
				}

				tdi_slice.at(beam_no,sample) = sqrt(cabs(complex_accumulator));
			}
		}
		NextProgress();
	}
	EndProgress();
	BiexpBlur3D(tdi_frames, point3_F64(1, 1, 2), e_use_omp);
}

void CFMProcessor::CalculateAverageCriterium()
{
	ComplexFunctionF32	burst;
	RealFunctionMD_F32::slice_type average_slice;
	StartProgress("Calculating Average Criterium", n_frames);
	for (size_t frame_no = 0; frame_no < n_frames; ++frame_no)
	{

		average_criteria.GetSlice(average_slice, {frame_no, slice_mask(0), slice_mask(1)});

		for (size_t sample = 0; sample < n_samples; ++sample)
		{
			for (size_t beam_no = 0; beam_no < n_beams; ++beam_no)
			{
				data_by_shots.GetRow(burst, {frame_no, beam_no, slice_mask(0), sample});
				average_slice.at(beam_no,sample) = cabs(AverageValue(burst));
			}
		}
		NextProgress();
	}
	EndProgress();
}

void CFMProcessor::CalculateSTDCriterium()
{
	ComplexFunctionF32	burst;
	RealFunctionMD_F32::slice_type stddev_slice_conj;
	RealFunctionMD_F32::slice_type stddev_slice_mult;
	StartProgress("Calculating STD Criterium", n_frames);
	for(size_t frame_no = 0; frame_no < n_frames; ++frame_no)
	{
		stddev_criteria_conj.GetSlice(stddev_slice_conj, {frame_no, slice_mask(0), slice_mask(1)});
		for(size_t sample = 0; sample < n_samples; ++sample)   
		{
			for(size_t beam_no = 0; beam_no < n_beams; ++beam_no)    
			{
				data_by_shots.GetRow(burst, {frame_no, beam_no, slice_mask(0), sample});
				double	cabs2_accumulator(0);	
				for(size_t shot_no = 0; shot_no < n_shots-1; ++shot_no)
				{
					cabs2_accumulator += cabs2(burst[shot_no]);
				}
				stddev_slice_conj.at(beam_no,sample) = sqrt(cabs2_accumulator/(n_shots-1));
			}
		}
		for (ptrdiff_t sample_no = 0; sample_no < ptrdiff_t(n_samples); ++sample_no)
		{
			RealFunctionF64	buffer_std_conj(stddev_slice_conj.col(sample_no));
			nth_element(buffer_std_conj.begin(), buffer_std_conj.begin() + n_beams / 2, buffer_std_conj.end());
			stddev_slice_conj.col(sample_no) = (stddev_slice_conj.col(sample_no) - buffer_std_conj[n_beams / 2])/(buffer_std_conj[n_beams / 2]);
		}
		BiexpBlur2D(stddev_slice_conj, 0, std_blur_axial);
		NextProgress();
	}
	EndProgress();
	ChangeLastBeam(stddev_criteria_conj);
	if (flag_blur_std)
	{
		ApplyFrameAveraging(stddev_criteria_conj, std_blur_time);
	}
} 

void	CFMProcessor::ApplyWallFilter(const WallFilter &wall_filter, size_t frame_no, size_t beam_start, size_t beam_end)
{
	ComplexFunctionF32 burst;
	for(size_t sample = 0; sample < n_samples; ++sample)   
	{ 
		for(size_t beam_no = beam_start; beam_no < beam_end; ++beam_no)    
		{
			data_by_shots.GetRow(burst, {frame_no, beam_no, slice_mask(0), sample});
			wall_filter.Apply(burst);
		}
	}
}

void CFMProcessor::CalculateCorrelationParts()
{
	ComplexFunctionF32	burst;

	StartProgress("Calculating Correlation Parts", n_frames);
	for(size_t frame_no = 0; frame_no < n_frames; ++frame_no)
	{
		ComplexFunctionMD_F64::slice_type numerators_slice;
		numerators.GetSlice(numerators_slice, {frame_no, slice_mask(0), slice_mask(1)});
		RealFunctionMD_F64::slice_type denominators_slice;
		denominators.GetSlice(denominators_slice, {frame_no, slice_mask(0), slice_mask(1)});
		for(size_t sample_no = 0; sample_no < n_samples; ++sample_no)
		{
			for(size_t beam_no = 0; beam_no < n_beams; ++beam_no)    
			{
				data_by_shots.GetRow(burst, {frame_no, beam_no, slice_mask(0), sample_no});
				ComplexFunctionF32::const_iterator	it = burst.cbegin();
				ComplexFunctionF32::const_iterator	it1 = it+1;

				double	d1 = 0, d2 = 0;
				complexF64	n(0);

				for(size_t shot_no = 0; shot_no < n_shots-1; ++shot_no, ++it, ++it1)
				{
					n.add_multiply_conj(*it, *it1);
					d1  += cabs2(*it);
					d2  += cabs2(*it1);
				}
				numerators_slice.at(beam_no,sample_no) = n;
				denominators_slice.at(beam_no,sample_no) = sqrt(d1*d2);
			}
		}
		NextProgress();
	}
	EndProgress();
	ChangeLastBeam(numerators);
	ChangeLastBeam(denominators);
}

void CFMProcessor::CalculateCorrelationReImParts()
{
	ComplexFunctionF32	burst;

	StartProgress("Calculating Correlation Re-Im Parts", n_frames);
	for (size_t frame_no = 0; frame_no < n_frames; ++frame_no)
	{
		RealFunctionMD_F64::slice_type numerators_re_im_slice;
		numerators_re_im.GetSlice(numerators_re_im_slice, { frame_no, slice_mask(0), slice_mask(1) });
		RealFunctionMD_F64::slice_type denominators_re_im_slice;
		denominators_re_im.GetSlice(denominators_re_im_slice, { frame_no, slice_mask(0), slice_mask(1) });
		for (size_t sample_no = 0; sample_no < n_samples; ++sample_no)
		{
			for (size_t beam_no = 0; beam_no < n_beams; ++beam_no)
			{
				data_by_shots.GetRow(burst, { frame_no, beam_no, slice_mask(0), sample_no });
				ComplexFunctionF32::const_iterator	it = burst.cbegin();

				double	n(0), d1(0), d2(0);

				for (size_t shot_no = 0; shot_no < n_shots - 1; ++shot_no, ++it)
				{
					n += real(*it)*imag(*it);
					d1 += (abs(real(*it)))*(abs(real(*it)));
					d2 += (abs(imag(*it)))*(abs(imag(*it)));
				}
				numerators_re_im_slice.at(beam_no,sample_no) = n;
				denominators_re_im_slice.at(beam_no,sample_no) = sqrt(d1*d2);
			}
		}
		NextProgress();
	}
	EndProgress();
	ChangeLastBeam(numerators_re_im);
	ChangeLastBeam(denominators_re_im);
}

void CFMProcessor::CalculateCorrelationCriteria()
{
	ComplexFunctionMD_F64::slice_type numerators_slice;
	RealFunctionMD_F64::slice_type denominators_slice;
	RealFunctionMD_F32::slice_type correlation_slice;

	StartProgress("Calculating Correlation Criteria", n_frames);
	for(size_t frame_no = 0; frame_no < n_frames; ++frame_no)
	{
		numerators.GetSlice(numerators_slice, {frame_no, slice_mask(0), slice_mask(1)});
		denominators.GetSlice(denominators_slice, {frame_no, slice_mask(0), slice_mask(1)});
		correlation_criteria.GetSlice(correlation_slice, {frame_no, slice_mask(0), slice_mask(1)});
		for(size_t i = 0; i < n_samples; ++i)   
		{
			for(size_t beam_no = 0; beam_no < n_beams; ++beam_no)    
			{
				correlation_slice.at(beam_no,i) = cabs(numerators_slice.at(beam_no,i)/denominators_slice.at(beam_no,i));
			}
		}
		if (flag_blur_correlation_parts)
		{
			double	additional_blur_radius = double(n_frames + 10) / double(n_frames + 1);
			BiexpBlur2D(correlation_slice, additional_blur_radius, additional_blur_radius*correlation_blur_axial);
			BiexpBlur2D(correlation_slice, 0, correlation_blur_axial);
		}
		NextProgress();
	}
	EndProgress();
	ApplyFrameAveraging(correlation_criteria, correlation_blur_time);
}


void CFMProcessor::CalculateCorrelationReImCriteria()
{
	RealFunctionMD_F64::slice_type numerators_re_im_slice;
	RealFunctionMD_F64::slice_type denominators_re_im_slice;
	RealFunctionMD_F32::slice_type correlation_re_im_slice;

	StartProgress("Calculating Correlation Re-Im Criteria", n_frames);
	for (size_t frame_no = 0; frame_no < n_frames; ++frame_no)
	{
		numerators_re_im.GetSlice(numerators_re_im_slice, { frame_no, slice_mask(0), slice_mask(1) });
		denominators_re_im.GetSlice(denominators_re_im_slice, { frame_no, slice_mask(0), slice_mask(1) });
		correlation_re_im_criteria.GetSlice(correlation_re_im_slice, { frame_no, slice_mask(0), slice_mask(1) });
		for (size_t i = 0; i < n_samples; ++i)
		{
			for (size_t beam_no = 0; beam_no < n_beams; ++beam_no)
			{
				correlation_re_im_slice.at(beam_no,i) = abs(numerators_re_im_slice.at(beam_no,i) / denominators_re_im_slice.at(beam_no,i));
			}
		}

		for (ptrdiff_t sample_no = 0; sample_no < ptrdiff_t(n_samples); ++sample_no)
		{
			RealFunctionF64	buffer_cor_re_im(correlation_re_im_slice.col(sample_no));
			nth_element(buffer_cor_re_im.begin(), buffer_cor_re_im.begin() + n_beams / 2, buffer_cor_re_im.end());
			correlation_re_im_slice.col(sample_no) = (correlation_re_im_slice.col(sample_no) - buffer_cor_re_im[n_beams / 2]) / (buffer_cor_re_im[n_beams / 2]);
		}

		if (flag_blur_correlation_parts)
		{
			double	additional_blur_radius = double(n_frames + 10) / double(n_frames + 1);
			BiexpBlur2D(correlation_re_im_slice, additional_blur_radius, additional_blur_radius*correlation_blur_axial);
			BiexpBlur2D(correlation_re_im_slice, 0, correlation_blur_axial);		
		}

		NextProgress();
	}
	EndProgress();
	if (flag_blur_correlation_parts)
	{
		ApplyFrameAveraging(correlation_re_im_criteria, correlation_blur_time);
	}
}


void CFMProcessor::CalculateCFMFrames()
{
	StartProgress("Calculating Phases", n_frames);
	for(size_t frame_no = 0; frame_no < n_frames; ++frame_no)
	{
		RealFunctionMD_F32::slice_type phase_slice;
		cfm_phase_frames.GetSlice(phase_slice, {frame_no, slice_mask(0), slice_mask(1)});
		ComplexFunctionMD_F64::slice_type numerators_slice;
		numerators.GetSlice(numerators_slice, {frame_no, slice_mask(0), slice_mask(1)});
		RealFunctionMD_F64::slice_type denominators_slice;
		denominators.GetSlice(denominators_slice, {frame_no, slice_mask(0), slice_mask(1)});
		for(size_t i = 0; i < n_samples; ++i)
		{
			ComplexFunctionF64::const_iterator	n_it = numerators_slice.col(i).cbegin();
			RealFunctionF64::const_iterator	d_it = denominators_slice.col(i).cbegin();
			for(size_t beam_no = 0; beam_no < n_beams; ++beam_no, ++n_it, ++d_it)    
			{
				double phase;
				phase = arg(*n_it);
				phase_slice.at(beam_no,i) = phase;
			}
		}
		NextProgress();
	}
	EndProgress();
	ChangeLastBeam(cfm_phase_frames);
};


void CFMProcessor::GetPreProcType()
{
	correlation_blur_axial = 0;
#if 0
	if(!GetCheckboxDecision("Choose Preprocessing Type",//3,
		{
			"Tissue Motion Compensation",
			"Blur correlation",
			"Blur std",
		},
		{
			&flag_tissue_motion_compensation,
			&flag_blur_correlation_parts,
			&flag_blur_std
		}))
	{
		throw canceled_operation("GetPreProcType canceled");
	}
	if(flag_tissue_motion_compensation)
	{
		tissue_motion_blur = GetFloating("Tissue Motion Compensation Blur", 5, 0, 1000);
	}
	if (flag_blur_correlation_parts)
	{
		correlation_blur_time = GetFloating("Time blur", 0.25, 0, 10);
		correlation_blur_axial = GetFloating("Axial blur", 1, 0, 10);
	}
	if (flag_blur_std)
	{
		if (flag_blur_correlation_parts)
		{
			std_blur_time = correlation_blur_time;
			std_blur_axial = correlation_blur_axial;
		}
		else
		{
			std_blur_time = GetFloating("Time blur", 0.25, 0, 10);
			std_blur_axial = GetFloating("Axial blur", 1, 0, 10);
		}
	}
#else
	flag_tissue_motion_compensation = true;
	flag_blur_correlation_parts = true;
	flag_blur_std = true;
	tissue_motion_blur = 5;
	correlation_blur_time = 0.25;
	correlation_blur_axial = 1;
	std_blur_time = correlation_blur_time;
	std_blur_axial = correlation_blur_axial;
#endif
}

void CFMProcessor::CalculatePowerFrames()
{
	RealFunctionMD_F32::slice_type power_slice;
	ComplexFunctionMD_F64::slice_type numerators_slice;
	RealFunctionMD_F64::slice_type denominators_slice;

	// следует обратить внимание: здесь что-то непонятное считается
	StartProgress("Calculating Power", n_frames);
	for(size_t frame_no = 0; frame_no < n_frames; ++frame_no)
	{
		power_frames.GetSlice(power_slice, {frame_no, slice_mask(0), slice_mask(1)});
		numerators.GetSlice(numerators_slice, {frame_no, slice_mask(0), slice_mask(1)});
		denominators.GetSlice(denominators_slice, {frame_no, slice_mask(0), slice_mask(1)});
		for(size_t i = 0; i < n_samples; ++i)   
		{
			ComplexFunctionF64::const_iterator	n_it = numerators_slice.col(i).cbegin();
			RealFunctionF64::const_iterator	d_it = denominators_slice.col(i).cbegin();
			for(size_t beam_no = 0; beam_no < n_beams; ++beam_no, ++n_it, ++d_it)
			{
				double power;
				if (*d_it<=0) power = 0;
				else	power = log(*d_it);
				power_slice.at(beam_no,i) = power;
			}
		}
		NextProgress();
	}
	EndProgress();
}

void CFMProcessor::CalculateTurbulenceFrames()
{
	RealFunctionMD_F32::slice_type turbulence_slice;
	ComplexFunctionMD_F64::slice_type numerators_slice;
	RealFunctionMD_F64::slice_type denominators_slice;

	StartProgress("Calculating Turbulence", n_frames);
	for(size_t frame_no = 0; frame_no < n_frames; ++frame_no)
	{
		turbulence_frames.GetSlice(turbulence_slice, {frame_no, slice_mask(0), slice_mask(1)});
		numerators.GetSlice(numerators_slice, {frame_no, slice_mask(0), slice_mask(1)});
		denominators.GetSlice(denominators_slice, {frame_no, slice_mask(0), slice_mask(1)});
		for(size_t i = 0; i < n_samples; ++i)   
		{
			ComplexFunctionF64::const_iterator	n_it = numerators_slice.col(i).cbegin();
			RealFunctionF64::const_iterator	d_it = denominators_slice.col(i).cbegin();
			for(size_t beam_no = 0; beam_no < n_beams; ++beam_no, ++n_it, ++d_it)
			{
				double turbulence;
				double	cc = cabs(*n_it)/(*d_it);
				turbulence = 1-cc;
				turbulence_slice.at(beam_no,i) = turbulence;
			}
		}
		NextProgress();
	}
	EndProgress();
}

#endif



void	CFMMode_find_blur_1D(S500_CFMFrameSet &frames)
{
	shared_ptr<CFDataPhaseAnalyzer> analyzer = GetAnalyzer();

	analyzer->PrepareBuffers({ size_t(frames.file_params.NumOfCFMBeams), size_t(frames.file_params.NumOfCFShots), size_t(frames.file_params.SizeofCFMBeamAtSamples) }, size_t(frames.file_params.NumOfCFMBeams / frames.file_params.NumOfSweeps), frames.file_params.HeaderSize);
	if (!(analyzer->algorithm() == ElastoGrafica::mode_elastography))
	{
		analyzer->wall_filter = GetWallFilterInteractive(frames.file_params.NumOfCFShots);
	}

	double max_frame_agility_factor(1);
	double max_elastogram_blur(.3);
	size_t blur_factor(100);
	size_t factor(100);
	RealFunctionF32 blur_true_detection(size_t(max_elastogram_blur * blur_factor), 0);
	RealFunctionF32 blur_false_detection(size_t(max_elastogram_blur * blur_factor), 0);
	RealFunctionF32 blur_mlfunction(size_t(max_elastogram_blur * blur_factor), 0);
	RealFunctionF32 blur_std(size_t(max_elastogram_blur * blur_factor), 0);
	RealFunctionF32 blur_correlation(size_t(max_elastogram_blur * blur_factor), 0);
//	StartProgress("FindBlur", (max_frame_agility_factor * blur_factor));
	//	for (size_t coef1 = 0; coef1 < (max_frame_agility_factor * blur_factor); ++coef1)
	float coef1 = max_frame_agility_factor * blur_factor;
	{
	//	ProgressBar	progress;
		GUIProgressBar	progress;
		progress.start("FindBlur", (max_elastogram_blur * blur_factor));
		for (size_t coef2 = 0; coef2 < (max_elastogram_blur * blur_factor); ++coef2)
		{
			analyzer->frame_agility_factor = float(coef2) / blur_factor;
			analyzer->elastogram_blur = float(coef1) / blur_factor;
			RealFunctionMD_F32 std_frames({ size_t(frames.file_params.NumOfFrames), size_t(frames.file_params.NumOfCFMBeams), size_t(frames.file_params.SizeofCFMBeamAtSamples) }, 0);
			RealFunctionMD_F32 correlation_frames({ size_t(frames.file_params.NumOfFrames), size_t(frames.file_params.NumOfCFMBeams), size_t(frames.file_params.SizeofCFMBeamAtSamples) }, 0);
			RealFunctionMD_F32 correlation_re_im_frames({ size_t(frames.file_params.NumOfFrames), size_t(frames.file_params.NumOfCFMBeams), size_t(frames.file_params.SizeofCFMBeamAtSamples) }, 0);
			RealFunctionMD_F32 phase_frames({ size_t(frames.file_params.NumOfFrames), size_t(frames.file_params.NumOfCFMBeams), size_t(frames.file_params.SizeofCFMBeamAtSamples) }, 0);
			RealFunctionMD_F32 mask_frames({ size_t(frames.file_params.NumOfFrames), size_t(frames.file_params.NumOfCFMBeams), size_t(frames.file_params.SizeofCFMBeamAtSamples) }, 0);
			RealFunctionMD_F32 phase_frames_normalized({ size_t(frames.file_params.NumOfFrames), size_t(frames.file_params.NumOfCFMBeams), size_t(frames.file_params.SizeofCFMBeamAtSamples) }, 0);
			for (size_t frame_no = 0; frame_no < frames.n_frames; ++frame_no)
			{
				RealFunction2D_F32	phase_slice, mask_slice, stddev_buffer;
				mask_frames.GetSlice(mask_slice, { frame_no, slice_mask(0), slice_mask(1) });
				phase_frames_normalized.GetSlice(phase_slice, { frame_no, slice_mask(0), slice_mask(1) });
				std_frames.GetSlice(analyzer->stddev_buffer, { frame_no, slice_mask(0), slice_mask(1) });
				correlation_frames.GetSlice(analyzer->correlation_buffer, { frame_no, slice_mask(0), slice_mask(1) });
				correlation_re_im_frames.GetSlice(analyzer->correlation_re_im_buffer, { frame_no, slice_mask(0), slice_mask(1) });
				phase_frames.GetSlice(analyzer->phase_buffer, { frame_no, slice_mask(0), slice_mask(1) });
				analyzer->ray_header_size_in_bytes = 0;
				analyzer->ImportRawData(&frames.cfm_shot_frames.at({ frame_no, 0, 0, 0 }), false);
				analyzer->CalculateOffsets(phase_slice, mask_slice);
			//	NextProgress();
			}
		//	EndProgress();
			//DisplayDopplerData(phase_frames, phase_frames_normalized, mask_frames, correlation_frames, correlation_re_im_frames, std_frames, frames);
			RealFunction2D_F32 mask(frames.n_cfm_beams, frames.n_cfm_samples, 0);
			size_t blood_area(0), noise_area(0);
//			Calculate_mask(std_frames, frames, mask, blood_area, noise_area);


			size_t first_beam(12);
			size_t last_beam(33);
			size_t first_sample(44);
			size_t last_sample(124);

			RealFunctionMD_F32 mask_data(std_frames);
			RealFunction2D_F32 slice;
			mask_data.GetSlice(slice, { 0, slice_mask(0), slice_mask(1) });
			ColorImageF32 color_mask;
			MakeCopy(color_mask, slice);
			//	DisplayMathFunction2D(slice, "slice", frames.sco_cfm);

			double	beam_centre = double(first_beam + last_beam) / 2;
			double	beam_radius = double(last_beam - first_beam) / 2;
			double	sample_centre = double(first_sample + last_sample) / 2;
			double	sample_radius = double(last_sample - first_sample) / 2;
			for (size_t i = 0; i < frames.n_cfm_beams; ++i)
			{
				double	y = double(i) - beam_centre;
				for (size_t j = 0; j < frames.n_cfm_samples; ++j)
				{
					double x = double(j) - sample_centre;
					double	p = square(y / beam_radius) + square(x / sample_radius);
					if (p < 0.8)
					{
						mask.at(i,j) = 1;
						blood_area += 1;
					}
					else if (p > 1.2)
					{
						mask.at(i,j) = -1;
						noise_area += 1;
					}
					else mask.at(i,j) = 0;
					if ((p > 0.6) && (p < 1.2)) color_mask.at(i,j) = ColorSampleF32(0, 255, 255);
				}
			}


			//	CalculateMLC_2D(std_frames, correlation_frames, frames, 4, 1, mask, blood_area, noise_area, 100);

			size_t max_threshold_value1(1);//std
			size_t max_threshold_value2(1);//correlation
			RealFunctionMD_F32 data1(std_frames);
			RealFunctionMD_F32 data2(correlation_frames);
			size_t last_frame(10);
			size_t n_frames = last_frame - 0;
			RealFunction2D_F32 true_detection(max_threshold_value1 * factor, max_threshold_value2 * factor, 0);
			RealFunction2D_F32 false_detection(true_detection);
		//	StartProgress("CalculateMLC_2D", last_frame);
			for (size_t frame_no = 0; frame_no < last_frame; ++frame_no)
			{
				RealFunction2D_F32	data1_slice;
				data1.GetSlice(data1_slice, { frame_no, slice_mask(0), slice_mask(1) });
				RealFunction2D_F32	data2_slice;
				data2.GetSlice(data2_slice, { frame_no, slice_mask(0), slice_mask(1) });
				for (size_t beam_no = 0; beam_no < frames.n_cfm_beams; ++beam_no)
				{
					for (size_t sample_no = 0; sample_no < frames.n_cfm_samples; ++sample_no)
					{
						float a1 = factor * data1_slice.at(beam_no,sample_no);
						float a2 = factor * data2_slice.at(beam_no,sample_no);
						float m = mask.at(beam_no,sample_no);
						if ((a1 > 0) && (a2 > 0) && !(m == 0))
						{
							for (size_t k1 = 0; k1 < true_detection.vsize(); ++k1)
							{
								for (size_t k2 = 0; k2 < true_detection.hsize(); ++k2)
								{
									if ((a1 > k1) && (a2 > k2))
									{
										if (m == 1)
										{
											true_detection.at(k1,k2) += 1;
										}
										if (m == -1)
										{
											false_detection.at(k1,k2) += 1;
										}
									}
								}
							}
						}
					}
				}
				//NextProgress();
			}
		//	EndProgress();
			true_detection /= (blood_area * n_frames);
			false_detection /= (noise_area * n_frames);

			RealFunction2D_F32 mlfunction(true_detection);
			mlfunction -= false_detection;
			size_t coordinate1(0), coordinate2(0);
			float max_value(mlfunction.at(0,0));
			for (size_t k1 = 0; k1 < mlfunction.vsize(); ++k1)
			{
				for (size_t k2 = 0; k2 < mlfunction.hsize(); ++k2)
				{
					if (max_value < mlfunction.at(k1,k2))
					{
						coordinate1 = k1;
						coordinate2 = k2;
						max_value = mlfunction.at(coordinate1,coordinate2);
					}
				}
			}
			blur_true_detection[coef2] = true_detection.at(coordinate1,coordinate2);
			blur_false_detection[coef2] = false_detection.at(coordinate1,coordinate2);
			blur_mlfunction[coef2] = mlfunction.at(coordinate1,coordinate2);
			blur_std[coef2] = coordinate1;
			blur_correlation[coef2] = coordinate2;
		
			++progress;
			}
		progress.end();
		}
	size_t /*coordinate1,*/ coordinate2(0);
	float max_value(blur_mlfunction[0]);
//	for (size_t k1 = 0; k1 < blur_mlfunction.vsize(); ++k1)
	{
		for (size_t k2 = 0; k2 < blur_mlfunction.size(); ++k2)
		{
			if (max_value < blur_mlfunction[k2])
			{
				//coordinate1 = k1;
				coordinate2 = k2;
				max_value = blur_mlfunction[coordinate2];
			}
		}
	}

	DisplayMathFunction(blur_true_detection,0,1,"blur_true_detection");
	DisplayMathFunction(blur_false_detection, 0,1,"blur_false_detection");
	string title = ssprintf("blur_true_detection -= blur_false_detection; max_value =  %g; frame_agility_factor = %g; blur = %g; true detection rate = %g; false datection rate = %g", max_value, float(coef1) / blur_factor, float(coordinate2) / blur_factor, blur_true_detection[coordinate2], blur_false_detection[coordinate2]);
	DisplayMathFunction(blur_mlfunction, 0,1,title);
	string title2 = ssprintf(" blur_true_detection -= blur_false_detection max_value =  %g;\n frame_agility_factor = %g;\n blur = %g;  \n STD = %g; \n Correlation = %g; \n true detection rate = %g; \n false datection rate = %g", max_value, float(coef1) / blur_factor, float(coordinate2) / blur_factor, blur_std[coordinate2]/factor, blur_correlation[coordinate2]/factor, blur_true_detection[coordinate2], blur_false_detection[coordinate2]);
	ShowString("Optimal values" + ssprintf(" for %d pulses", frames.n_cfm_shots), title2);
}

void	CFMMode_find_blur_2D(S500_CFMFrameSet &frames)
{
	shared_ptr<CFDataPhaseAnalyzer> analyzer = GetAnalyzer();

	analyzer->PrepareBuffers({ size_t(frames.file_params.NumOfCFMBeams), size_t(frames.file_params.NumOfCFShots), size_t(frames.file_params.SizeofCFMBeamAtSamples) }, size_t(frames.file_params.NumOfCFMBeams / frames.file_params.NumOfSweeps), frames.file_params.HeaderSize);
	if (!(analyzer->algorithm() == ElastoGrafica::mode_elastography))
	{
		analyzer->wall_filter = GetWallFilterInteractive(frames.file_params.NumOfCFShots);
	}

	double max_frame_agility_factor(1.1);
	double max_elastogram_blur(0.5);
	size_t blur_factor(10);
	size_t factor(100);
	RealFunction2D_F32 blur_true_detection(max_frame_agility_factor * blur_factor, max_elastogram_blur * blur_factor, 0);
	RealFunction2D_F32 blur_false_detection(max_frame_agility_factor * blur_factor, max_elastogram_blur * blur_factor, 0);
	RealFunction2D_F32 blur_mlfunction(max_frame_agility_factor * blur_factor, max_elastogram_blur * blur_factor, 0);
	RealFunction2D_F32 blur_std(max_frame_agility_factor * blur_factor, max_elastogram_blur * blur_factor, 0);
	RealFunction2D_F32 blur_correlation(max_frame_agility_factor * blur_factor, max_elastogram_blur * blur_factor, 0);
	GUIProgressBar	progress;
	progress.start("FindBlur", (max_frame_agility_factor * blur_factor));
	for (size_t coef1 = 0; coef1 < (max_frame_agility_factor * blur_factor); ++coef1)
	{
		for (size_t coef2 = 0; coef2 < (max_elastogram_blur * blur_factor); ++coef2)
		{
			analyzer->frame_agility_factor = float(coef2) / blur_factor;
			analyzer->elastogram_blur = float(coef1) / blur_factor;
			RealFunctionMD_F32 std_frames({ size_t(frames.file_params.NumOfFrames), size_t(frames.file_params.NumOfCFMBeams), size_t(frames.file_params.SizeofCFMBeamAtSamples) }, 0);
			RealFunctionMD_F32 correlation_frames({ size_t(frames.file_params.NumOfFrames), size_t(frames.file_params.NumOfCFMBeams), size_t(frames.file_params.SizeofCFMBeamAtSamples) }, 0);
			RealFunctionMD_F32 correlation_re_im_frames({ size_t(frames.file_params.NumOfFrames), size_t(frames.file_params.NumOfCFMBeams), size_t(frames.file_params.SizeofCFMBeamAtSamples) }, 0);
			RealFunctionMD_F32 phase_frames({ size_t(frames.file_params.NumOfFrames), size_t(frames.file_params.NumOfCFMBeams), size_t(frames.file_params.SizeofCFMBeamAtSamples) }, 0);
			RealFunctionMD_F32 mask_frames({ size_t(frames.file_params.NumOfFrames), size_t(frames.file_params.NumOfCFMBeams), size_t(frames.file_params.SizeofCFMBeamAtSamples) }, 0);
			RealFunctionMD_F32 phase_frames_normalized({ size_t(frames.file_params.NumOfFrames), size_t(frames.file_params.NumOfCFMBeams), size_t(frames.file_params.SizeofCFMBeamAtSamples) }, 0);
			//StartProgress("Frame Analyzer", frames.n_frames);
			for (size_t frame_no = 0; frame_no < frames.n_frames; ++frame_no)
			{
				RealFunction2D_F32	phase_slice, mask_slice, stddev_buffer;
				mask_frames.GetSlice(mask_slice, { frame_no, slice_mask(0), slice_mask(1) });
				phase_frames_normalized.GetSlice(phase_slice, { frame_no, slice_mask(0), slice_mask(1) });
				std_frames.GetSlice(analyzer->stddev_buffer, { frame_no, slice_mask(0), slice_mask(1) });
				correlation_frames.GetSlice(analyzer->correlation_buffer, { frame_no, slice_mask(0), slice_mask(1) });
				correlation_re_im_frames.GetSlice(analyzer->correlation_re_im_buffer, { frame_no, slice_mask(0), slice_mask(1) });
				phase_frames.GetSlice(analyzer->phase_buffer, { frame_no, slice_mask(0), slice_mask(1) });
				analyzer->ray_header_size_in_bytes = 0;
				analyzer->ImportRawData(&frames.cfm_shot_frames.at({ frame_no, 0, 0, 0 }), false);
				analyzer->CalculateOffsets(phase_slice, mask_slice);
				//	NextProgress();
			}
			//	EndProgress();
			//DisplayDopplerData(phase_frames, phase_frames_normalized, mask_frames, correlation_frames, correlation_re_im_frames, std_frames, frames);
			RealFunction2D_F32 mask(frames.n_cfm_beams, frames.n_cfm_samples, 0);
			size_t blood_area(0), noise_area(0);
			//			Calculate_mask(std_frames, frames, mask, blood_area, noise_area);


			size_t first_beam(12);
			size_t last_beam(33);
			size_t first_sample(44);
			size_t last_sample(124);

			RealFunctionMD_F32 mask_data(std_frames);
			RealFunction2D_F32 slice;
			mask_data.GetSlice(slice, { 0, slice_mask(0), slice_mask(1) });
			ColorImageF32 color_mask;
			MakeCopy(color_mask, slice);
			//	DisplayMathFunction2D(slice, "slice", frames.sco_cfm);

			double	beam_centre = double(first_beam + last_beam) / 2;
			double	beam_radius = double(last_beam - first_beam) / 2;
			double	sample_centre = double(first_sample + last_sample) / 2;
			double	sample_radius = double(last_sample - first_sample) / 2;
			for (size_t i = 0; i < frames.n_cfm_beams; ++i)
			{
				double	y = double(i) - beam_centre;
				for (size_t j = 0; j < frames.n_cfm_samples; ++j)
				{
					double x = double(j) - sample_centre;
					double	p = square(y / beam_radius) + square(x / sample_radius);
					if (p < 0.8)
					{
						mask.at(i,j) = 1;
						blood_area += 1;
					}
					else if (p > 1.2)
					{
						mask.at(i,j) = -1;
						noise_area += 1;
					}
					else mask.at(i,j) = 0;
					if ((p > 0.6) && (p < 1.2)) color_mask.at(i,j) = ColorSampleF32(0, 255, 255);
				}
			}


			//	CalculateMLC_2D(std_frames, correlation_frames, frames, 4, 1, mask, blood_area, noise_area, 100);

			size_t max_threshold_value1(1);//std
			size_t max_threshold_value2(1);//correlation
			RealFunctionMD_F32 data1(std_frames);
			RealFunctionMD_F32 data2(correlation_frames);
			size_t last_frame(10);
			size_t n_frames = last_frame - 0;
			RealFunction2D_F32 true_detection(max_threshold_value1 * factor, max_threshold_value2 * factor, 0);
			RealFunction2D_F32 false_detection(true_detection);
			//	StartProgress("CalculateMLC_2D", last_frame);
			for (size_t frame_no = 0; frame_no < last_frame; ++frame_no)
			{
				RealFunction2D_F32	data1_slice;
				data1.GetSlice(data1_slice, { frame_no, slice_mask(0), slice_mask(1) });
				RealFunction2D_F32	data2_slice;
				data2.GetSlice(data2_slice, { frame_no, slice_mask(0), slice_mask(1) });
				for (size_t beam_no = 0; beam_no < frames.n_cfm_beams; ++beam_no)
				{
					for (size_t sample_no = 0; sample_no < frames.n_cfm_samples; ++sample_no)
					{
						float a1 = factor * data1_slice.at(beam_no,sample_no);
						float a2 = factor * data2_slice.at(beam_no,sample_no);
						float m = mask.at(beam_no,sample_no);
						if ((a1 > 0) && (a2 > 0) && !(m == 0))
						{
							for (size_t k1 = 0; k1 < true_detection.vsize(); ++k1)
							{
								for (size_t k2 = 0; k2 < true_detection.hsize(); ++k2)
								{
									if ((a1 > k1) && (a2 > k2))
									{
										if (m == 1)
										{
											true_detection.at(k1,k2) += 1;
										}
										if (m == -1)
										{
											false_detection.at(k1,k2) += 1;
										}
									}
								}
							}
						}
					}
				}
				//NextProgress();
			}
			//	EndProgress();
			true_detection /= (blood_area * n_frames);
			false_detection /= (noise_area * n_frames);

			RealFunction2D_F32 mlfunction(true_detection);
			mlfunction -= false_detection;
			size_t coordinate1(0), coordinate2(0);
			float max_value(mlfunction.at(0,0));
			for (size_t k1 = 0; k1 < mlfunction.vsize(); ++k1)
			{
				for (size_t k2 = 0; k2 < mlfunction.hsize(); ++k2)
				{
					if (max_value < mlfunction.at(k1,k2))
					{
						coordinate1 = k1;
						coordinate2 = k2;
						max_value = mlfunction.at(coordinate1,coordinate2);
					}
				}
			}
			blur_true_detection.at(coef1,coef2) = true_detection.at(coordinate1,coordinate2);
			blur_false_detection.at(coef1,coef2) = false_detection.at(coordinate1,coordinate2);
			blur_mlfunction.at(coef1,coef2) = mlfunction.at(coordinate1,coordinate2);
			blur_std.at(coef1,coef2) = coordinate1;
			blur_correlation.at(coef1,coef2) = coordinate2;


		}
		++progress;
	}
	progress.end();
	size_t coordinate1(0), coordinate2(0);
	float max_value(blur_mlfunction.at(0,0));
	for (size_t k1 = 0; k1 < blur_mlfunction.vsize(); ++k1)
	{
		for (size_t k2 = 0; k2 < blur_mlfunction.hsize(); ++k2)
		{
			if (max_value < blur_mlfunction.at(k1,k2))
			{
				coordinate1 = k1;
				coordinate2 = k2;
				max_value = blur_mlfunction.at(coordinate1,coordinate2);
			}
		}
	}
	
	DisplayMathFunction2D(blur_true_detection, "blur_true_detection", ScanFrameRectangle(cm(max_frame_agility_factor), cm(max_elastogram_blur)));
	DisplayMathFunction2D(blur_false_detection, "blur_false_detection", ScanFrameRectangle(cm(max_frame_agility_factor), cm(max_elastogram_blur)));
	string title = ssprintf("blur_true_detection -= blur_false_detection; max_value =  %g; frame_agility_factor = %g; blur = %g; true detection rate = %g; false datection rate = %g", max_value, float(coordinate1) / blur_factor, float(coordinate2) / blur_factor, blur_true_detection.at(coordinate1,coordinate2), blur_false_detection.at(coordinate1,coordinate2));
	DisplayMathFunction2D(blur_mlfunction, title, ScanFrameRectangle(cm(max_frame_agility_factor), cm(max_elastogram_blur)));
	string title2 = ssprintf(" blur_true_detection -= blur_false_detection max_value =  %g;\n frame_agility_factor = %g;\n blur = %g;  \n STD = %g; \n Correlation = %g; \n true detection rate = %g; \n false datection rate = %g", max_value, float(coordinate1) / blur_factor, float(coordinate2) / blur_factor, blur_std.at(coordinate1,coordinate2) / factor, blur_correlation.at(coordinate1,coordinate2) / factor, blur_true_detection.at(coordinate1,coordinate2), blur_false_detection.at(coordinate1,coordinate2));
	ShowString("Optimal values" + ssprintf(" for %d pulses", frames.n_cfm_shots), title2);
}
#endif

XRAD_END
