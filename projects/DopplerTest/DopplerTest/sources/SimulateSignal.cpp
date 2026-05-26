#include "pre.h"
#include <RFDataImport/S500_CFMFrameSet.h>
#include <RFDataImport/S500_CFMRawDataDisplay.h>
#include "SimulateSignal.h"

XRAD_BEGIN

S500_CFMFrameSet SimulateSignal()
{
	enum
	{
		clutter, 
		clutter_accelerated,
		noise,
		blood,
		oscillations,
		noisy_clutter,
		noisy_accelerated_clutter,
		noisy_blood,
		noisy_oscillations,
		cluttery_blood
	};
	auto answer = Decide(L"What should we simulate?",
	{
		MakeButton(L"Tissue motion", clutter),
		MakeButton(L"Accelerated tissue motion",  clutter_accelerated),
		MakeButton("Noise", noise),
		MakeButton("Blood", blood),
		MakeButton("Oscillations", oscillations),
		MakeButton("Noise + Tissue motion", noisy_clutter),
		MakeButton("Noise + Accelerated tissue motion", noisy_accelerated_clutter),
		MakeButton("Noise + Blood", noisy_blood),
		MakeButton("Noise + Oscillations", noisy_oscillations),
		MakeButton("Tissue motion + Blood", cluttery_blood)
	});

	size_t n_shots = GetUnsigned("Number of shots", 17, 2, 100);
	size_t n_beams = 100;//25;
	size_t n_samples = 300;//50;
	size_t n_sweeps = 1;
	size_t n_frames = 10;
	n_frames = GetUnsigned("Number of frames", 10, 1, 100);
	size_t n_beams_in_sweep = n_beams/n_sweeps;
	size_t n_shots_in_sweep = n_shots*n_beams_in_sweep; 
	size_t n_shots_in_frame = n_shots_in_sweep*n_sweeps;

	ComplexMatrixF32 test_frame_flow(n_samples, n_shots_in_frame, complexF32(0));
	ComplexMatrixF32 test_frame_C_component(test_frame_flow);
	ComplexMatrixF32 test_frame_clutter(test_frame_flow);
	ComplexMatrixF32 test_frame_clutter_accelerated(test_frame_flow);
	ComplexMatrixF32 test_frame_noise(test_frame_flow);
	double clutter_amp = decibel_to_amplitude(40);
	
	double lowest_v_с_velocity = 0;
	double highest_v_с_velocity = pi();
	double lowest_v_b_velocity = 0;
	double highest_v_b_velocity = pi();
	

	if (answer == clutter || answer == noisy_clutter || answer == cluttery_blood) 
	{
		clutter_amp = decibel_to_amplitude(GetUnsigned("Tissue signal level in decibels", 40, 0, 100));
		lowest_v_с_velocity = pi() * (GetUnsigned("Lowest level of tissue velocity (grad)", 0, 0, 180)) / 180;
		highest_v_с_velocity = pi() * (GetUnsigned("Highest level of tissue velocity (grad)", 18, 0, 180)) / 180;
	}
	double blood_amp = decibel_to_amplitude(4);
	if (answer == blood || answer == noisy_blood || answer == cluttery_blood) 
	{ 
		blood_amp = decibel_to_amplitude(GetUnsigned("Blood signal level in decibels", 4, 0, 100)); 
		lowest_v_b_velocity = pi()*(GetUnsigned("Lowest level of blood velocity (grad)", 0, 0, 180))/180;
		highest_v_b_velocity = pi() * (GetUnsigned("Highest level of blood velocity (grad)", 180, 0, 180))/180;
	}
	double oscillation_amp = decibel_to_amplitude(4);
	if (answer == oscillations || answer == noisy_oscillations) { oscillation_amp = decibel_to_amplitude(GetUnsigned("Oscillations signal level in decibels", 4, 0, 100)); }
	size_t sigma = 1000;
	if (answer == noise || answer == noisy_clutter || answer == noisy_accelerated_clutter || answer == noisy_blood || answer == noisy_oscillations)
	{ sigma = decibel_to_amplitude(GetUnsigned("Sigma for noise distribution", 4, 0, 100)); }
	ComplexMatrixF32 test_frame0(test_frame_flow);

	S500_CFMParamFileData	test_params;
	test_params.NumOfFrames = int(n_frames);
	test_params.RawFrameSize = 0;
	test_params.HeaderSize = 0;
	test_params.NumOfBBeams = int(n_beams);
	test_params.SizeofBBeamAtSamples = int(n_samples);
	test_params.NumOfCFShots = int(n_shots);
	test_params.NumOfSweeps = int(n_sweeps);
	test_params.BeamsInSweep = int(n_beams_in_sweep);
	test_params.SizeofCFMBeamAtSamples = test_params.SizeofBBeamAtSamples;
	test_params.FirstScanCFMBeam = 0;
	test_params.CFMDensity = 1;
	test_params.NumOfCFMBeams = int(n_beams);
	test_params.NumOfFirstCFMSample = 0;
	test_params.CFMFilterOrder = 0;

	size_t start_frame = 0;
	size_t end_frame = test_params.NumOfFrames - 1;
	S500_CFMFrameSet	test_frames(test_params, start_frame, end_frame);
	test_frames.b_frames.fill(complexF32(0));
	test_frames.cfm_frames.fill(complexF32(1));
	cfm_slice_t	cfm_slice(test_frame_flow);
	double v_c, v_b, v_C, a_c;
	RealFunctionF32 velocity_c(test_params.NumOfFrames), velocity_b(n_beams), velocity_C_component(n_beams, 0);
	for (size_t n_frame = 0; n_frame < size_t(test_params.NumOfFrames); ++n_frame)
	{
		for (size_t sample = 0; sample < n_samples; ++sample)
		{
			for (size_t shot = 0; shot < n_shots_in_frame; ++shot)
			{
				size_t n_beam = shot / n_shots;
				
				//Моделируются ткани, движущиеся без ускорения
				v_c = lowest_v_b_velocity + (shot * highest_v_b_velocity / 10) * (double(n_frame) / (test_params.NumOfFrames - 1));
				//velocity_c[n_frame] = (pi() / 10)* (double(n_frame) / (test_params.NumOfFrames - 1));
				velocity_c[n_frame] = lowest_v_b_velocity + highest_v_b_velocity * (double(n_frame) / (test_params.NumOfFrames - 1));
				test_frame_clutter.at(sample, shot) = polar(clutter_amp, v_c);

				//Моделируются ткани, движущиеся с ускорением
				a_c = (shot*shot*pi() / 10) * (double(n_frame) / (test_params.NumOfFrames - 1));
				test_frame_clutter_accelerated.at(sample, shot) = polar(clutter_amp, a_c);

				//Моделируется кровоток
				//v_b =  shot*pi()*(double(n_beam) / (n_beams)); // / 50
				v_b = lowest_v_b_velocity + shot * highest_v_b_velocity * (double(n_beam) / (n_beams)); // / 50
				velocity_b[n_beam] = lowest_v_b_velocity + highest_v_b_velocity * (double(n_beam) / (n_beams));
				test_frame_flow.at(sample,shot) = polar(blood_amp, v_b);

				//Моделируется компонента упругих колебаний АФП
				size_t pulse = shot - n_beam*n_shots;
				v_C = pulse *  pi();
				v_C = v_C*n_beam / (n_beams - 1);
				test_frame_C_component.at(sample,shot) = polar(oscillation_amp, v_C);
				test_frame_C_component.at(sample,shot).im = test_frame_C_component.at(sample,shot).re/2;
				size_t prf = 1000;
				velocity_C_component[n_beam] = prf*v_C/(2*pi()*(n_shots-1));
				
				//Моделируются шумы
				test_frame_noise.at(sample,shot) = complexF32(RandomGaussian(0, sigma), RandomGaussian(0, sigma));
			}
		}

		if (answer == clutter) { test_frame0 = test_frame_clutter; };
		if (answer == clutter_accelerated) { test_frame0 = test_frame_clutter_accelerated; };
		if (answer == noise) { test_frame0 = test_frame_noise; };
		if (answer == blood) { test_frame0 = test_frame_flow; };
		if (answer == oscillations) { test_frame0 = test_frame_C_component; };
		if (answer == noisy_clutter) { test_frame0 = test_frame_noise + test_frame_clutter; };
		if (answer == noisy_accelerated_clutter) { test_frame0 = test_frame_noise + test_frame_clutter_accelerated;	};
		if (answer == noisy_blood) { test_frame0 = test_frame_noise + test_frame_flow; };
		if (answer == noisy_oscillations) { test_frame0 = test_frame_noise + test_frame_C_component; };
		if (answer == cluttery_blood) { test_frame0 = test_frame_flow + test_frame_clutter; };
		
		test_frames.cfm_frames.GetSlice(cfm_slice, { n_frame, slice_mask(1), slice_mask(0) });
		CopyData(cfm_slice, test_frame0);
		//if (answer == clutter || answer == noisy_clutter) {velocity_c[n_frame] = arg(test_frame_clutter.at(n_samples / 2,n_shots_in_frame / 2) % test_frame_clutter.at(n_samples / 2 + 1,n_shots_in_frame / 2 + 1));	}
		//if (answer == blood || answer == noisy_blood) {velocity_b[n_frame] = arg(test_frame_flow.at(n_samples / 2, n_shots_in_frame / 2) % test_frame_flow.at(n_samples / 2 + 1, n_shots_in_frame / 2 + 1)); }
		
	}
	if (answer == clutter || answer == noisy_clutter ||	answer == cluttery_blood) { DisplayMathFunction(velocity_c, 0, 1, "Dependence of tissue velocity (rad) on frame number");}
	if (answer == blood || answer == noisy_blood || answer == cluttery_blood) {	DisplayMathFunction(velocity_b, 0, 1, "Dependence of blood velocity (rad) on beam number");}
	return test_frames;
}


XRAD_END