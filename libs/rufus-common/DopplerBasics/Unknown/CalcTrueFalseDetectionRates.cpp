#include "pre.h"

#include <XRADBasic/Sources/Utils/ExponentialBlurAlgorithms.h> 
#include <RFDataImport/S500_CFMRawDataDisplay.h>

#include "CalcTrueFalseDetectionRates.h"


XRAD_BEGIN

void InitCoordinates(RealFunctionF32 &a_coordinates, RealFunctionF32& c_coordinates, RealFunctionF32& d_coordinates, RealFunctionF32& e_coordinates)
{
	// A - blood
	a_coordinates[0] = 0;  //frame
	a_coordinates[1] = 547;
	a_coordinates[2] = 38; //beam
	a_coordinates[3] = 51;
	a_coordinates[4] = 88; //sample
	a_coordinates[5] = 119;

	// C - twinkling
	c_coordinates[0] = 647;  //frame
	c_coordinates[1] = 1216;
	c_coordinates[2] = 0; //beam
	c_coordinates[3] = 38;
	c_coordinates[4] = 70; //sample
	c_coordinates[5] = 127;

	// D - twinkling
	d_coordinates[0] = 1326;  //frame
	d_coordinates[1] = 1863;
	d_coordinates[2] = 0; //beam
	d_coordinates[3] = 50;
	d_coordinates[4] = 60; //sample
	d_coordinates[5] = 90;

	// E - noise
	e_coordinates[0] = 1963;  //frame
	e_coordinates[1] = 2153;
	e_coordinates[2] = 0; //beam
	e_coordinates[3] = 126;
	e_coordinates[4] = 60; //sample
	e_coordinates[5] = 130;
}

void InitThresholds(RealFunctionF32& disp, RealFunctionF32& cor, RealFunctionF32& correim, RealFunctionF32& std, RealFunctionF32& amp)
{
	disp[0] = 3;
	cor[0] = 0.35;
	correim[0] = 0.3;
	std[0] = 1;
	amp[0] = 1;
}

void GetCoordinates(RealFunctionF32& coordinates, const S500_CFMFrameSet& frames)
{
	coordinates[0] = GetUnsigned("Frame No start", saved_default_value, 0, frames.b_frames.sizes(0));  //frame
	coordinates[1] = GetUnsigned("Frame No end", saved_default_value, coordinates[0], frames.b_frames.sizes(0));

	coordinates[2] = GetUnsigned("Beam No start", saved_default_value, 0, frames.b_frames.sizes(1));//beam
	coordinates[3] = GetUnsigned("Beam No end", saved_default_value, coordinates[2], frames.b_frames.sizes(1));
	
	coordinates[4] = GetUnsigned("Sample No start", saved_default_value, 0, frames.b_frames.sizes(2)); //sample
	coordinates[5] = GetUnsigned("Sample No end", saved_default_value, coordinates[4], frames.b_frames.sizes(2));
}

void ChangeRule(RealFunctionF32& a_coordinates, RealFunctionF32& c_coordinates, RealFunctionF32& d_coordinates, RealFunctionF32& e_coordinates, const S500_CFMFrameSet& frames)
{
	enum
	{
		A, C, D, display_exit
	} option;

	try
	{
		while (true)
		{
			option = GetButtonDecision("Choose additional task",
				{
				MakeButton("A", A),
				MakeButton("C", C),
				MakeButton("D", D),
				MakeButton("Exit", display_exit)
				}
			);
			switch (option)
			{
			case A:
			{
				GetCoordinates(a_coordinates, frames);
			}
			break;
			case C:
			{
				GetCoordinates(c_coordinates, frames);
			}
			break;
			case D:
			{
				GetCoordinates(d_coordinates, frames);
			}
			break;

			break;
			default:
				throw canceled_operation("");
			}
		}
	}
	catch (canceled_operation&) {}
	catch (quit_application&) { throw; }
	catch (...) { Error(GetExceptionString()); }
}


void ChangeSignal(size_t &region)
{
	enum
	{
		A, C, D, display_exit
	} option;

	try
	{
	//	while (true)
		{
			option = GetButtonDecision("Choose additional task",
				{
				MakeButton("A", A),
				MakeButton("C", C),
				MakeButton("D", D),
				MakeButton("Exit", display_exit)
				}
			);
			switch (option)
			{
			case A:
			{
				region = 0;
			}
			break;
			case C:
			{
				region = 1;
			}
			break;
			case D:
			{
				region = 2;
			}
			break;

			break;
			default:
				throw canceled_operation("");
			}
		}
	}
	//catch (canceled_operation&) {}
	//catch (quit_application&) { throw; }
	catch (...) { Error(GetExceptionString()); }
}

void ChangeCoordinates(RealFunctionF32& a_coordinates, RealFunctionF32& c_coordinates, RealFunctionF32& d_coordinates, RealFunctionF32& e_coordinates, const S500_CFMFrameSet& frames)
{
	enum {A, C, D, E, display_exit } option;
	try
	{
		while (true)
		{
			option = GetButtonDecision("Choose additional task",
				{
				MakeButton("A", A),
				MakeButton("C", C),
				MakeButton("D", D),
				MakeButton("E", E),
				MakeButton("Exit", display_exit)
				}
			);
			switch (option)
			{
			case A:
			{
				GetCoordinates(a_coordinates, frames);
			}
			break;
			case C:
			{
				GetCoordinates(c_coordinates, frames);
			}
			break;
			case D:
			{
				GetCoordinates(d_coordinates, frames);
			}
			break;

			case E:
			{
				GetCoordinates(e_coordinates, frames);
			}
			break;
			default:
				throw canceled_operation("");
			}
		}
	}
	catch (canceled_operation&) {}
	catch (quit_application&) { throw; }
	catch (...) { Error(GetExceptionString()); }
}




void Rule(float phase_disp, float cor, float cor_reim, float std, float amp, float phase_disp_threshold, float cor_threshold, float cor_reim_threshold, float std_threshold, float amp_threshold, size_t rule, size_t &j)
{
	if (rule == 0) { if ((cor >= cor_threshold) && (cor_reim <= cor_reim_threshold) && (std >= std_threshold) && (amp <= amp_threshold)) { j++;} }
	if (rule == 1) { if ((phase_disp <= phase_disp_threshold) && (cor >= cor_threshold) && (cor_reim <= cor_reim_threshold) && (std >= std_threshold)) { j++;} }
	if (rule == 2) { if ((phase_disp <= phase_disp_threshold) && (cor >= cor_threshold) && (cor_reim <= cor_reim_threshold) && (std >= std_threshold) && (amp <= amp_threshold)) { j++;} }
	if (rule == 3) { if ((cor <= cor_threshold) && (cor_reim >= cor_reim_threshold) && (std >= std_threshold) && (amp >= amp_threshold)) { j++;} }
	if (rule == 4) { if ((phase_disp >= phase_disp_threshold) && (cor <= cor_threshold) && (cor_reim >= cor_reim_threshold) && (std >= std_threshold)) { j++;} }
	if (rule == 5) { if ((phase_disp >= phase_disp_threshold) && (cor <= cor_threshold) && (cor_reim >= cor_reim_threshold) && (std >= std_threshold) && (amp >= amp_threshold)) { j++; } }
	if (rule == 6) { if ((cor <= cor_threshold) && (cor_reim <= cor_reim_threshold) && (std >= std_threshold) && (amp >= amp_threshold)) { j++; } }
	if (rule == 7) { if ((phase_disp >= phase_disp_threshold) && (cor <= cor_threshold) && (cor_reim <= cor_reim_threshold) && (std >= std_threshold)) { j++; } }
	if (rule == 8) { if ((phase_disp >= phase_disp_threshold) && (cor <= cor_threshold) && (cor_reim <= cor_reim_threshold) && (std >= std_threshold) && (amp >= amp_threshold)) { j++; } }
}

size_t CountPixelsAndDetermineDetectionRates( RealFunctionF32 coordinates, const RealFunctionMD_F32 cfm_phase_dispersion, const RealFunctionMD_F32 correlation_frames,
	const RealFunctionMD_F32 correlation_re_im_frames, const RealFunctionMD_F32 std_frames, const RealFunctionMD_F32 amplitude_frames,
	const double phase_dispersion_threshold, const double correlation_threshold, const double correlation_re_im_threshold,
	const double std_threshold, const double amplitude_threshold, size_t rule_type)
{
	size_t pixel_count(0);
	size_t n_frames = coordinates[1] - coordinates[0];
	//GUIProgressBar	progress;
	//progress.start("Creating animation", n_frames);
	for (size_t frame_no = coordinates[0]; frame_no < coordinates[1]; ++frame_no)
	{
		auto phase_disp_slice = cfm_phase_dispersion.GetSlice({ frame_no, slice_mask(0), slice_mask(1) });
		auto correlation_slice = correlation_frames.GetSlice({ frame_no, slice_mask(0), slice_mask(1) });
		auto correlation_re_im_slice = correlation_re_im_frames.GetSlice({ frame_no, slice_mask(0), slice_mask(1) });
		auto std_slice = std_frames.GetSlice({ frame_no, slice_mask(0), slice_mask(1) });
		auto amplitude_slice = amplitude_frames.GetSlice({ frame_no, slice_mask(0), slice_mask(1) });

		for (size_t i = coordinates[2]; i < coordinates[3]; ++i)
		{
			for (size_t j = coordinates[4]; j < coordinates[5]; ++j)
			{
				Rule(phase_disp_slice.at(i, j), correlation_slice.at(i, j), correlation_re_im_slice.at(i, j), std_slice.at(i, j), amplitude_slice.at(i, j),
					phase_dispersion_threshold, 	correlation_threshold,	correlation_re_im_threshold,	std_threshold, amplitude_threshold,
					rule_type, pixel_count);
			}
		}
		//++progress;
	}
	//progress.end();
	return pixel_count;
}

size_t TotalNumberOfPixels(RealFunctionF32 &coord)
{
	size_t area = (coord[1] - coord[0]) * (coord[3] - coord[2]) * (coord[5] - coord[4]);
	return area;
}

void GetThresholds(RealFunctionF32& threshold, size_t& number_of_steps)
{
	number_of_steps = GetUnsigned("number_of_threshold_steps", saved_default_value, 0, 100) + 1;
	threshold.realloc(number_of_steps);
	threshold[0] = GetFloating("First threshold value", saved_default_value, 0, 100000);
	threshold[number_of_steps-1] = GetFloating("Last threshold value", saved_default_value, threshold[0], 100000);
	if (number_of_steps > 0)
	{
		float step = (threshold[number_of_steps - 1] - threshold[0]) / number_of_steps;
		for (size_t i = 1; i < number_of_steps; i++)
		{
			threshold[i] = threshold[i - 1] + step;
		}
	}
}


void ChangeThresholds(RealFunctionF32 &phase_dispersion_threshold, size_t &number_of_phase_disp_threshold_steps,
	RealFunctionF32& correlation_threshold, size_t& number_of_cor_threshold_steps,
	RealFunctionF32& correlation_re_im_threshold, size_t& number_of_correim_threshold_steps,
	RealFunctionF32& std_threshold, size_t& number_of_std_threshold_steps,
	RealFunctionF32& amplitude_threshold, size_t& number_of_amp_threshold_steps)
{

	enum { phase_dispersion, cor, cor_re_im, std, amplitude, display_exit } option;
	try
	{
		while (true)
		{
			option = GetButtonDecision("Choose additional task",
				{
				MakeButton("phase_dispersion", phase_dispersion),
				MakeButton("cor", cor),
				MakeButton("cor_re_im", cor_re_im),
				MakeButton("std", std),
				MakeButton("amplitude", amplitude),
				MakeButton("Exit", display_exit)
				}
			);
			switch (option)
			{
			case phase_dispersion:
			{
				GetThresholds(phase_dispersion_threshold, number_of_phase_disp_threshold_steps);
			}
			break;
			case cor:
			{
				GetThresholds(correlation_threshold, number_of_cor_threshold_steps);
			}
			break;
			case cor_re_im:
			{
				GetThresholds(correlation_re_im_threshold, number_of_correim_threshold_steps);
			}
			break;
			case std:
			{
				GetThresholds(std_threshold, number_of_std_threshold_steps);
			}
			break;
			case amplitude:
			{
				GetThresholds(amplitude_threshold, number_of_amp_threshold_steps);
			}
			break;
			default:
				throw canceled_operation("");
			}
		}
	}
	catch (canceled_operation&) {}
	catch (quit_application&) { throw; }
	catch (...) { Error(GetExceptionString()); }
}

void	DisplayDopplerDataTrueFalseDetectionRates(
	const RealFunctionMD_F32 &cfm_phase_dispersion,
	const RealFunctionMD_F32 &correlation_frames,
	const RealFunctionMD_F32 &correlation_re_im_frames,
	const RealFunctionMD_F32 &std_frames,
	const RealFunctionMD_F32 &amplitude_frames,
	const S500_CFMFrameSet &frames)
{
	size_t number_of_coords(6);
	RealFunctionF32 a_coordinates(number_of_coords), c_coordinates(number_of_coords),	 d_coordinates(number_of_coords),	 e_coordinates(number_of_coords);
	InitCoordinates(a_coordinates, c_coordinates, d_coordinates, e_coordinates);
	size_t number_of_phase_disp_threshold_steps(1), number_of_cor_threshold_steps(1), number_of_correim_threshold_steps(1), number_of_std_threshold_steps(1), number_of_amp_threshold_steps(1);
	RealFunctionF32 phase_dispersion_threshold(number_of_phase_disp_threshold_steps), correlation_threshold(number_of_cor_threshold_steps),
		correlation_re_im_threshold(number_of_correim_threshold_steps), std_threshold(number_of_std_threshold_steps), amplitude_threshold(number_of_amp_threshold_steps);
	InitThresholds(phase_dispersion_threshold, correlation_threshold, correlation_re_im_threshold, std_threshold, amplitude_threshold);
	size_t rule_type(0);
	RealFunctionMD_F32 amplitude_frames_reverse(amplitude_frames);
	RealFunctionMD_F32 std(std_frames);
	size_t pixels_a(0), pixels_c(0), pixels_d(0), pixels_e(0), region(0);

	enum
	{
		show_values,
		change_coords,
		change_signal,
		change_thresholds,
		change_rule,
		calculate_pixels,
		display_exit
	} option;

	try
	{
		while(true)
		{
			option = GetButtonDecision("Choose additional task",
			{
			MakeButton("Show", show_values),
			MakeButton("Change coordinates", change_coords),
			MakeButton("Change signal of interest", change_signal),
			MakeButton("Change thresholds", change_thresholds),
			MakeButton("Change decision rule", change_rule),
			MakeButton("Calculate", calculate_pixels),
			MakeButton("Exit", display_exit)});
			switch(option)
			{
				case show_values:
				{
					ShowText("Initial data", ssprintf("\nA \nframe %g-%g\nbeam %g-%g\nsample %g-%g\n\nC \nframe %g-%g\nbeam %g-%g\nsample %g-%g\n\nD \nframe %g-%g\nbeam %g-%g\nsample %g-%g\n\nE \nframe %g-%g\nbeam %g-%g\nsample %g-%g",
						a_coordinates[0], a_coordinates[1], a_coordinates[2], a_coordinates[3], a_coordinates[4], a_coordinates[5],
						c_coordinates[0], c_coordinates[1], c_coordinates[2], c_coordinates[3], c_coordinates[4], c_coordinates[5],
						d_coordinates[0], d_coordinates[1], d_coordinates[2], d_coordinates[3], d_coordinates[4], d_coordinates[5],
						e_coordinates[0], e_coordinates[1], e_coordinates[2], e_coordinates[3], e_coordinates[4], e_coordinates[5]) +
						ssprintf("\n\nThresholds\nphase_dispersion_threshold (first-steps_no-last) %g-%d-%g \ncorrelation_threshold (first-steps_no-last) %g-%d-%g  \ncorrelation_re_im_threshold (first-steps_no-last) %g-%d-%g  \nstd_threshold (first-steps_no-last) %g-%d-%g  \namplitude_threshold (first-steps_no-last) %g-%d-%g ",
							phase_dispersion_threshold[0], number_of_phase_disp_threshold_steps-1, phase_dispersion_threshold[number_of_phase_disp_threshold_steps-1], 
							correlation_threshold[0], number_of_cor_threshold_steps-1, correlation_threshold[number_of_cor_threshold_steps-1],
							correlation_re_im_threshold[0], number_of_correim_threshold_steps-1, correlation_re_im_threshold[number_of_correim_threshold_steps-1],
							std_threshold[0], number_of_std_threshold_steps-1, std_threshold[number_of_std_threshold_steps-1],
							amplitude_threshold[0], number_of_amp_threshold_steps-1, amplitude_threshold[number_of_amp_threshold_steps-1]) +
						ssprintf("\n\nChosen signals for detection and optimisation thresholds %d\n0 - A, \n1 - C, \n2 - D  ", region) +
						ssprintf("\n\nRule %d", rule_type) +
						"\nA" +
						"\n0 - (cor >= cor_threshold) && (cor_reim <= cor_reim_threshold) && (std >= std_threshold) && (amp <= amp_threshold))" +
						"\n1 - (phase_disp <= phase_disp_threshold) && (cor >= cor_threshold) && (cor_reim <= cor_reim_threshold) && (std >= std_threshold)" +
						"\n2 - (phase_disp <= phase_disp_threshold) && (cor >= cor_threshold) && (cor_reim <= cor_reim_threshold) && (std >= std_threshold) && (amp <= amp_threshold))" +
						"\nC" +
						"\n3 - (cor <= cor_threshold) && (cor_reim >= cor_reim_threshold) && (std >= std_threshold) && (amp >= amp_threshold))" +
						"\n4 - (phase_disp >= phase_disp_threshold) && (cor <= cor_threshold) && (cor_reim >= cor_reim_threshold) && (std >= std_threshold)" +
						"\n5 - (phase_disp >= phase_disp_threshold) && (cor <= cor_threshold) && (cor_reim >= cor_reim_threshold) && (std >= std_threshold) && (amp >= amp_threshold))" + 
						"\nD" +
						"\n6 - (cor <= cor_threshold) && (cor_reim <= cor_reim_threshold) && (std >= std_threshold) && (amp >= amp_threshold))" +
						"\n7 - (phase_disp >= phase_disp_threshold) && (cor <= cor_threshold) && (cor_reim <= cor_reim_threshold) && (std >= std_threshold)" +
						"\n8 - (phase_disp >= phase_disp_threshold) && (cor <= cor_threshold) && (cor_reim <= cor_reim_threshold) && (std >= std_threshold) && (amp >= amp_threshold))");
				}
				break;
				case change_coords:
				{
					ChangeCoordinates(a_coordinates, c_coordinates, d_coordinates, e_coordinates, frames);
				}
				break;
				case change_signal:
				{
					ChangeSignal(region);
				}
				break;
				case change_thresholds:
				{
					ChangeThresholds( phase_dispersion_threshold, number_of_phase_disp_threshold_steps, 
						correlation_threshold, number_of_cor_threshold_steps, 
						correlation_re_im_threshold, number_of_correim_threshold_steps, 
						std_threshold, number_of_std_threshold_steps, 
						amplitude_threshold, number_of_amp_threshold_steps);
				}
				break;

				case change_rule:
				{
					rule_type = GetUnsigned("Choose rule No", saved_default_value, 0, 7 );
				}
				break;
				case calculate_pixels:
				{
					float tp_a(0), fp_a(0), tn_a(0), fn_a(0), de_a(0);
					float tp_c(0), fp_c(0), tn_c(0), fn_c(0), de_c(0);
					float tp_d(0), fp_d(0), tn_d(0), fn_d(0), de_d(0);
					float tp_r(0), fp_r(0), tn_r(0), fn_r(0), de_r(0);
					float tp(0), fp(0), tn(0), fn(0), de(0);

					size_t pd_i(0), c_i(0), cri_i(0), s_i(0), a_i(0);
					float pd_t(0), c_t(0), cri_t(0), s_t(0), a_t(0);

					GUIProgressBar	progress;
					progress.start("Creating animation", phase_dispersion_threshold.size()* correlation_threshold.size()* correlation_re_im_threshold.size()* std_threshold.size()* amplitude_threshold.size());
					for (size_t pd_i = 0; pd_i < phase_dispersion_threshold.size(); pd_i++)
					{
						for (size_t c_i = 0; c_i < correlation_threshold.size(); c_i++)
						{
							for (size_t cri_i = 0; cri_i < correlation_re_im_threshold.size(); cri_i++)
							{
								for (size_t s_i = 0; s_i < std_threshold.size(); s_i++)
								{
									for (size_t a_i = 0; a_i < amplitude_threshold.size(); a_i++)
									{

										pixels_a = CountPixelsAndDetermineDetectionRates(a_coordinates, cfm_phase_dispersion, correlation_frames, correlation_re_im_frames, std_frames, amplitude_frames,
											phase_dispersion_threshold[pd_i], correlation_threshold[c_i], correlation_re_im_threshold[cri_i], std_threshold[s_i], amplitude_threshold[a_i], rule_type);
										pixels_c = CountPixelsAndDetermineDetectionRates(c_coordinates, cfm_phase_dispersion, correlation_frames, correlation_re_im_frames, std_frames, amplitude_frames,
											phase_dispersion_threshold[pd_i], correlation_threshold[c_i], correlation_re_im_threshold[cri_i], std_threshold[s_i], amplitude_threshold[a_i], rule_type);
										pixels_d = CountPixelsAndDetermineDetectionRates(d_coordinates, cfm_phase_dispersion, correlation_frames, correlation_re_im_frames, std_frames, amplitude_frames,
											phase_dispersion_threshold[pd_i], correlation_threshold[c_i], correlation_re_im_threshold[cri_i], std_threshold[s_i], amplitude_threshold[a_i], rule_type);
										pixels_e = CountPixelsAndDetermineDetectionRates(e_coordinates, cfm_phase_dispersion, correlation_frames, correlation_re_im_frames, std_frames, amplitude_frames,
											phase_dispersion_threshold[pd_i], correlation_threshold[c_i], correlation_re_im_threshold[cri_i], std_threshold[s_i], amplitude_threshold[a_i], rule_type);

										tp_a = float(pixels_a) / float(TotalNumberOfPixels(a_coordinates));
										fp_a = float(pixels_c + pixels_d + pixels_e) / float(TotalNumberOfPixels(c_coordinates) + TotalNumberOfPixels(d_coordinates) + TotalNumberOfPixels(e_coordinates));
										tn_a = 1 - fp_a;
										fn_a = 1 - tp_a;
										de_a = (tp_a + tn_a) / (tp_a + tn_a + fp_a + fn_a);

										tp_c = float(pixels_c) / float(TotalNumberOfPixels(c_coordinates));
										fp_c = float(pixels_a + pixels_d + pixels_e) / float(TotalNumberOfPixels(a_coordinates) + TotalNumberOfPixels(d_coordinates) + TotalNumberOfPixels(e_coordinates));
										tn_c = 1 - fp_c;
										fn_c = 1 - tp_c;
										de_c = (tp_c + tn_c) / (tp_c + tn_c + fp_c + fn_c);

										tp_d = float(pixels_d) / float(TotalNumberOfPixels(d_coordinates));
										fp_d = float(pixels_a + pixels_c + pixels_e) / float(TotalNumberOfPixels(a_coordinates) + TotalNumberOfPixels(c_coordinates) + TotalNumberOfPixels(e_coordinates));
										tn_d = 1 - fp_d;
										fn_d = 1 - tp_d;
										de_d = (tp_d + tn_d) / (tp_d + tn_d + fp_d + fn_d);

										if (region == 0) { de_r = de_a; tp_r = tp_a; fp_r = fp_a; tn_r = tn_a; fn_r = fn_a; };
										if (region == 1) { de_r = de_c; tp_r = tp_c; fp_r = fp_c; tn_r = tn_c; fn_r = fn_c; };
										if (region == 2) { de_r = de_d; tp_r = tp_d; fp_r = fp_d; tn_r = tn_d; fn_r = fn_d;	};

										if (de < de_r)
										{
											de = de_a; tp = tp_r; fp = fp_r; tn = tn_r; fn = fn_r;
											pd_t = phase_dispersion_threshold[pd_i];
											c_t = correlation_threshold[c_i];
											cri_t = correlation_re_im_threshold[cri_i];
											s_t = std_threshold[s_i];
											a_t = amplitude_threshold[a_i];
										}
										++progress;
									}
								}
							}
						}
					}
					progress.end();				



					ShowText("Results", ssprintf("Best results\nphase_dispersion_threshold %g \ncorrelation_threshold %g \ncorrelation_re_im_threshold %g \nstd_threshold %g \namplitude_threshold %g", pd_t, c_t, cri_t, s_t, a_t) +
						ssprintf("\nTrue Positive %g \nFalse Positive %g", tp, fp) +
						ssprintf("\nTrue Negative %g \nFalse Negative %g", tn, fn) +
						ssprintf("\nDiagnostic efficiency %g", de));
				}
				break;

				default:
					throw canceled_operation("");
			}
		}
	}
	catch(canceled_operation &) {}
	catch(quit_application &) { throw; }
	catch(...) { Error(GetExceptionString()); }

}



XRAD_END
