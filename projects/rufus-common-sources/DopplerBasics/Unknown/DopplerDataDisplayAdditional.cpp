#include "pre.h"

#include <XRADBasic/Sources/Utils/ExponentialBlurAlgorithms.h> 
#include <RFDataImport/S500_CFMRawDataDisplay.h>

#include "DopplerDataDisplayAdditional.h"

XRAD_BEGIN

// Ниже идет экспериментальный код неясного назначения, бывший в DopplerDataDisplay

void	Calculate_mask(const RealFunctionMD_F32 &in_data, const S500_CFMFrameSet &frames, RealFunction2D_F32 &mask, size_t &blood_area, size_t &noise_area)
{
	//GAMMEX linear array
	size_t first_beam(12);
	size_t last_beam(33);
	size_t first_sample(44);
	size_t last_sample(124);
	double margin(0.2);
/*
//2016_12_07_ABS_wire
	//Plastics area
	first_beam = 161;
	last_beam = 175;
	first_sample = 136;
	last_sample = 152;
	margin = 0.1;


	//Metal area
	first_beam = 55;
	last_beam = 75;
	first_sample = 127;
	last_sample = 141;
	margin = 0.1;
	

//2017_07_10_Breast_Phantom
	//calculi
	first_beam = 27;
	last_beam = 36;
	first_sample = 48;
	last_sample = 63;
	margin = 0.1;


	//noise
	first_beam = 56;
	last_beam = 63;
	first_sample = 0;
	last_sample = 23;
	margin = 0.1;
	
	//2017_08_09_Noise_Gammex
	first_beam = 0;
	last_beam = 24;
	first_sample = 0;
	last_sample = 519;
	margin = 0;
	
	//2017_08_09_Wire
	first_beam = 0;
	last_beam = 50;
	first_sample = 57;
	last_sample = 72;
	margin = 0;

	//2017_08_23_Gammex_linear_stationary
	first_beam = 36;
	last_beam = 54;
	first_sample = 74;
	last_sample = 133;
	margin = 0.2;
	
	//2017_08_09_ABS_radiation_force_in_spiritus
	first_beam = 0;
	last_beam = 50;
	first_sample = 87;
	last_sample = 127;
	margin = 0;
*/
	//2017_08_09_Gammex_linear_stationary
	first_beam = 38;
	last_beam = 51;
	first_sample = 88;
	last_sample = 119;
	margin = 0.2;

	RealFunctionMD_F32 mask_data(in_data);
	RealFunction2D_F32 slice;
	mask_data.GetSlice(slice, { 0, slice_mask(0), slice_mask(1) });
	ColorImageF32 color_mask;
	MakeCopy(color_mask, slice);
//	DisplayMathFunction2D(slice, "slice", frames.sco_cfm);
	double std_counter(0);
	double	beam_centre = double(first_beam + last_beam) / 2;
	double	beam_radius = double(last_beam - first_beam) / 2;
	double	sample_centre = double(first_sample + last_sample) / 2;
	double	sample_radius = double(last_sample - first_sample) / 2;
	for (size_t i = 0; i < frames.n_cfm_beams(); ++i)
	{
		double	y = double(i) - beam_centre;
		for (size_t j = 0; j < frames.n_cfm_samples(); ++j)
		{
			double x = double(j) - sample_centre;
			double	p = square(y / beam_radius) + square(x / sample_radius);
			if (p < (1 - margin))
			{
				mask.at(i,j) = 1;
				blood_area += 1;
				std_counter += slice.at(i,j);
			}
			else if (p >(1 + margin))
			{
				mask.at(i,j) = -1;
				noise_area += 1;
			}
			else mask.at(i,j) = 0;
			if ((p > (1 - margin)) && (p < (1 + margin))) color_mask.at(i,j) = ColorSampleF32(0, 255, 255);
		}
	}
	double mean_std = std_counter / blood_area;
	double signal_noise_ratio = 20*logn(mean_std+1, 10);
	DisplayMathFunction2D(color_mask, ssprintf("color_mask; blood_area = %d; noise_area = %d, signal_noise_ratio = %g dB", blood_area, noise_area, signal_noise_ratio), frames.sco_cfm);
}

//TODO неиспользуемые переменные (warning 4100)
void	CalculateMLC_1D(const RealFunctionMD_F32 &in_data, const S500_CFMFrameSet &frames, float max_threshold_value, const wstring &/*title*/, RealFunction2D_F32 &mask, size_t &blood_area, size_t &noise_area)
{
	RealFunctionMD_F32 data(in_data);
	size_t first_frame(0);
	size_t last_frame(10);
	size_t n_frames = last_frame - first_frame;

	RealFunctionF32 true_detection(size_t(max_threshold_value * 1000), 0);
	RealFunctionF32 false_detection(size_t(max_threshold_value * 1000), 0);
	for (size_t frame_no = 0; frame_no < last_frame; ++frame_no)
	{
		RealFunction2D_F32	data_slice;
		data.GetSlice(data_slice, { frame_no, slice_mask(0), slice_mask(1) });
		for (size_t beam_no = 0; beam_no < frames.n_cfm_beams(); ++beam_no)
		{
			for (size_t sample_no = 0; sample_no < frames.n_cfm_samples(); ++sample_no)
			{
				float a = 1000 * data_slice.at(beam_no,sample_no);
				float m = mask.at(beam_no,sample_no);
				if ((a > 0) || !(m == 0))
				{
					for (size_t k = 0; k < true_detection.size(); ++k)
					{
						if ((a > k) && (m == 1))
						{
							true_detection[k] += 1;
						}
						if ((a > k) && (m == -1))
						{
							false_detection[k] += 1;
						}
					}
				}
			}
		}
	}
	true_detection /= (blood_area * n_frames);
	false_detection /= (noise_area * n_frames);

	FILE	*file1 = fopen("c:\\temp\\true_detection_1D.txt", "ab+");
	//FILE	*file1 = fopen("c:\\temp\\blood_reimcor\\reimcor_blood_17_150000.txt", "ab+");
	//FILE	*file2 = fopen("c:\\temp\\false_detection_1D.txt", "ab+");
	RealFunctionF32 mlfunction(true_detection);
	mlfunction -= false_detection;
	size_t coordinate(0);
	float max_value(mlfunction[0]);
	for (size_t k = 0; k < (mlfunction.size() - 1); ++k)
	{
		fprintf(file1, "%g, ", true_detection[k]);
		//fprintf(file2, "%g, ", false_detection[k]);
		if ((mlfunction[k] < mlfunction[k + 1]) && (max_value < mlfunction[k + 1]))
		{
			coordinate = k + 1;
			max_value = mlfunction[coordinate];
		}
	}
	fclose(file1);
//	fclose(file2);
	GraphSet	gs(ssprintf(L": true_detection -= false_detection; max_value =  %g; coordinate = %d; true detection rate = %g; false datection rate = %g", max_value, coordinate, true_detection[coordinate], false_detection[coordinate]), L"probability", L"value");
	gs.AddGraphUniform(true_detection, 0, 1, L"true_detection");
	gs.AddGraphUniform(false_detection, 0, 1, L"false_detection");
	gs.AddGraphUniform(mlfunction, 0, 1, L"mlfunction");
	gs.Display();
}

//TODO неиспользуемые переменные (warning 4100)
void ReadDetectionData_1D(const S500_CFMFrameSet &/*frames*/, float max_threshold_value, wstring title)
{
	size_t factor(1000);
	RealFunctionF32 true_detection(size_t(max_threshold_value*factor), 0);
	FILE	*f = fopen("c:\\temp\\true_detection_1D.txt", "rb");
	RealFunctionF32::iterator it = true_detection.begin();
	for (size_t i = 0; i < true_detection.size(); ++i, ++it)
	{
		fscanf(f, "%g, ", &(*it));
	}
	fclose(f);
	DisplayMathFunction(true_detection, 0, 1, "true_detection");
	RealFunctionF32 false_detection(true_detection);
	FILE	*f2 = fopen("c:\\temp\\false_detection_1D.txt", "rb");
	RealFunctionF32::iterator it2 = false_detection.begin();
	for (size_t i = 0; i < false_detection.size(); ++i, ++it2)
	{
		fscanf(f2, "%g, ", &(*it2));
	}
	fclose(f2);
	DisplayMathFunction(false_detection, 0, 1, "false_detection");
	RealFunctionF32 mlfunction(true_detection);
	mlfunction -= false_detection;
	
// 	size_t coordinate = 0;
// 	float max_value = mlfunction[0];

	size_t coordinate;
	float max_value = MaxValue(mlfunction, &coordinate);


// 	for (size_t k2 = 0; k2 < mlfunction.size(); ++k2)
// 	{
// 		if (max_value < mlfunction[k2])
// 		{
// 			coordinate = k2;
// 			max_value = mlfunction[coordinate];
// 		}
// 	}
	GraphSet	gs(title + ssprintf(L": true_detection -= false_detection; max_value =  %g; coordinate = %d; true detection rate = %g; false datection rate = %g", max_value, coordinate, true_detection[coordinate], false_detection[coordinate]), L"probability", L"value");
	gs.AddGraphUniform(true_detection, 0, 1, L"true_detection");
	gs.AddGraphUniform(false_detection, 0, 1, L"false_detection");
	gs.AddGraphUniform(mlfunction, 0, 1, L"mlfunction");
	gs.Display();
}

void	CalculateMLC_2D(const RealFunctionMD_F32 &in_data1, const RealFunctionMD_F32 &in_data2, const S500_CFMFrameSet &frames, float max_threshold_value1, float max_threshold_value2, RealFunction2D_F32 &mask, size_t &blood_area, size_t &noise_area, size_t factor)
{
	RealFunctionMD_F32 data1(in_data1); 
	RealFunctionMD_F32 data2(in_data2);
	size_t last_frame(10);
	size_t n_frames = last_frame - 0;
	RealFunction2D_F32 true_detection(max_threshold_value1 * factor, max_threshold_value2 * factor, 0);
	RealFunction2D_F32 false_detection(true_detection);
	GUIProgressBar	progress;
	progress.start("CalculateMLC_2D", last_frame);
	for (size_t frame_no = 0; frame_no < last_frame; ++frame_no)
	{
		RealFunction2D_F32	data1_slice;
		data1.GetSlice(data1_slice, { frame_no, slice_mask(0), slice_mask(1) });
		RealFunction2D_F32	data2_slice;
		data2.GetSlice(data2_slice, { frame_no, slice_mask(0), slice_mask(1) });
		for (size_t beam_no = 0; beam_no < frames.n_cfm_beams(); ++beam_no)
		{
			for (size_t sample_no = 0; sample_no < frames.n_cfm_samples(); ++sample_no)
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
		++progress;
	}
	progress.end();
	true_detection /= (blood_area * n_frames);
	false_detection /= (noise_area * n_frames);
	
	FILE	*file1 = fopen("c:\\temp\\true_detection_2D.txt", "ab+");
	FILE	*file2 = fopen("c:\\temp\\false_detection_2D.txt", "ab+");
	RealFunction2D_F32 mlfunction(true_detection);
	mlfunction -= false_detection;
	size_t coordinate1(0), coordinate2(0);
	float max_value(mlfunction.at(0,0));
	for (size_t k1 = 0; k1 < mlfunction.vsize(); ++k1)
	{
		for (size_t k2 = 0; k2 < mlfunction.hsize(); ++k2)
		{
			fprintf(file1, "%g, ", true_detection.at(k1,k2));
			fprintf(file2, "%g, ", false_detection.at(k1,k2));
			if (max_value < mlfunction.at(k1,k2))
			{
				coordinate1 = k1;
				coordinate2 = k2;
				max_value = mlfunction.at(coordinate1,coordinate2);
			}
		}
	}
	fclose(file1);
	fclose(file2);
	DisplayMathFunction2D(true_detection, "true_detection", ScanFrameRectangle(cm(max_threshold_value1), cm(max_threshold_value2)));
	DisplayMathFunction2D(false_detection, "false_detection", ScanFrameRectangle(cm(max_threshold_value1), cm(max_threshold_value2)));
	string title = ssprintf("true_detection -= false_detection; max_value =  %g; std = %d; corelation = %d; true detection rate = %g; false datection rate = %g", max_value, coordinate1, coordinate2, true_detection.at(coordinate1,coordinate2), false_detection.at(coordinate1,coordinate2));
	DisplayMathFunction2D(mlfunction, title, ScanFrameRectangle(cm(max_threshold_value1), cm(max_threshold_value2)));
	string title2 = ssprintf(" max_value =  %g; \n STD = %g; \n Correlation = %g; \n true detection rate = %g; \n false datection rate = %g", max_value, float(coordinate1) / factor, float(coordinate2) / factor, true_detection.at(coordinate1,coordinate2), false_detection.at(coordinate1,coordinate2));
	ShowString("true_detection -= false_detection", title2);
}


void	CalculateMLC_3D(const RealFunctionMD_F32 &in_data1, const RealFunctionMD_F32 &in_data2, const RealFunctionMD_F32 &in_data3, const S500_CFMFrameSet &frames, float max_threshold_value1, float max_threshold_value2, float max_threshold_value3, RealFunction2D_F32 &mask, size_t &blood_area, size_t &noise_area, size_t factor)
{
//	size_t factor(10);// 0);
	RealFunctionMD_F32 data1(in_data1);
	RealFunctionMD_F32 data2(in_data2);
	RealFunctionMD_F32 data3(in_data3);
	size_t first_frame(3);
	size_t last_frame(4);// frames.n_frames);
	size_t n_frames = last_frame - first_frame;
	RealFunctionMD_F32 true_detection({ size_t(max_threshold_value1 * factor), size_t(max_threshold_value2 * factor), size_t(max_threshold_value3 * factor) }, 0);
	RealFunctionMD_F32 false_detection(true_detection);
	GUIProgressBar	progress;
	progress.start("CalculateMLC_3D", last_frame);
	for (size_t frame_no = first_frame; frame_no < last_frame; ++frame_no)
	{
		RealFunction2D_F32	data1_slice;
		data1.GetSlice(data1_slice, { frame_no, slice_mask(0), slice_mask(1) });
		RealFunction2D_F32	data2_slice;
		data2.GetSlice(data2_slice, { frame_no, slice_mask(0), slice_mask(1) });
		RealFunction2D_F32	data3_slice;
		data3.GetSlice(data3_slice, { frame_no, slice_mask(0), slice_mask(1) }); 
		for (size_t k1 = 0; k1 < true_detection.sizes(0); ++k1)
		{
			RealFunction2D_F32	true_detection_slice;
			true_detection.GetSlice(true_detection_slice, { k1, slice_mask(0), slice_mask(1) });
			RealFunction2D_F32	false_detection_slice;
			false_detection.GetSlice(false_detection_slice, { k1, slice_mask(0), slice_mask(1) });
			for (size_t beam_no = 0; beam_no < frames.n_cfm_beams(); ++beam_no)
			{
				for (size_t sample_no = 0; sample_no < frames.n_cfm_samples(); ++sample_no)
				{
					float a1 = factor * data1_slice.at(beam_no,sample_no);
					float a2 = factor * data2_slice.at(beam_no,sample_no);
					float a3 = factor * data3_slice.at(beam_no,sample_no);
					float m = mask.at(beam_no,sample_no);
					if ((a1 > 0) && (a2 > 0) && (a3 > 0) && !(m == 0))
					for (size_t k2 = 0; k2 < true_detection.sizes(1); ++k2)
					{
						for (size_t k3 = 0; k3 < true_detection.sizes(2); ++k3)
						{
							if ((a1 > k1) && (a2 > k2) && (a3 > k3))
							{
								if (m == 1)
								{
									true_detection_slice.at(k2,k3) += 1;
								}
								if (m == -1)
								{
									false_detection_slice.at(k2,k3) += 1;
								}
							}
						}
					}
				}
			}
		}
		++progress;
	}
	progress.end();
	true_detection /= (blood_area * n_frames);
	false_detection /= (noise_area * n_frames);

	FILE	*file1 = fopen("c:\\temp\\true_detection_3D.txt", "ab+");
	FILE	*file2 = fopen("c:\\temp\\false_detection_3D.txt", "ab+");
	RealFunctionMD_F32 mlfunction(true_detection);
	mlfunction -= false_detection;
	size_t coordinate1(0), coordinate2(0), coordinate3(0);
	float max_value = mlfunction.at({0,0,0});
	for (size_t k1 = 0; k1 < mlfunction.sizes(0); ++k1)
	{
		RealFunction2D_F32	mlfunction_slice;
		mlfunction.GetSlice(mlfunction_slice, { k1, slice_mask(0), slice_mask(1) });
		RealFunction2D_F32	true_detection_slice;
		true_detection.GetSlice(true_detection_slice, { k1, slice_mask(0), slice_mask(1) });
		RealFunction2D_F32	false_detection_slice;
		false_detection.GetSlice(false_detection_slice, { k1, slice_mask(0), slice_mask(1) });
		for (size_t k2 = 0; k2 < mlfunction.sizes(1); ++k2)
		{
			for (size_t k3 = 0; k3 < mlfunction.sizes(2); ++k3)
			{
				//true_detection.at({ k1,k2,k3 });
				fprintf(file1, "%g, ", true_detection_slice.at(k2,k3));
				fprintf(file2, "%g, ", false_detection_slice.at(k2,k3));
				if (max_value < mlfunction_slice.at(k2,k3))
				{
					coordinate1 = k1;
					coordinate2 = k2;
					coordinate3 = k3;
					max_value = mlfunction_slice.at(coordinate2,coordinate3);
				}
			}
		}
	}
	fclose(file1);
	fclose(file2);
	RealFunction2D_F32	t_slice;
	true_detection.GetSlice(t_slice, { coordinate1, slice_mask(0), slice_mask(1) });
	RealFunction2D_F32	f_slice;
	false_detection.GetSlice(f_slice, { coordinate1, slice_mask(0), slice_mask(1) });
	string title = ssprintf(" max_value =  %g; \n STD = %g; \n Correlation = %g;\n Correlation Re Im = %g; \n true detection rate = %g; \n false datection rate = %g", max_value, float(coordinate1) / factor, float(coordinate2) / factor, float(coordinate3) / factor, t_slice.at(coordinate2,coordinate3), f_slice.at(coordinate2,coordinate3));
	ShowString("true_detection -= false_detection", title);
}

//TODO неиспользуемые переменные (warning 4100)
void ReadDetectionData_3D(const S500_CFMFrameSet &/*frames*/, float max_threshold_value1, float max_threshold_value2, float max_threshold_value3, float factor)
{
//	shared_cfile	f;
//	f.open("c:\\temp\\true_detection_3D.txt", "rb");
	FILE	*f = fopen("c:\\temp\\true_detection_3D.txt", "rb");
//	size_t factor(100);
	RealFunctionMD_F32 true_detection({ size_t(max_threshold_value1 * factor), size_t(max_threshold_value2 * factor), size_t(max_threshold_value3 * factor) }, 0);
	for (size_t k1 = 0; k1 < true_detection.sizes(0); ++k1)
	{
		RealFunction2D_F32	true_detection_slice;
		true_detection.GetSlice(true_detection_slice, { k1, slice_mask(0), slice_mask(1) });
		for (size_t j = 0; j < true_detection_slice.vsize(); ++j)
		{
			RealFunctionF32 row(true_detection_slice.hsize(), 0);
			RealFunctionF32::iterator it = row.begin();
			for (size_t i = 0; i < true_detection_slice.hsize(); ++i, ++it)
			{
				fscanf(f, "%g, ", &(*it));
				float a = *it;
				true_detection_slice.at(j,i) = a;
			}
			//DisplayMathFunction(row, 0, 1, "row");
		}
	}
	fclose(f);

	FILE	*f2 = fopen("c:\\temp\\false_detection_3D.txt", "rb");
	RealFunctionMD_F32 false_detection({ size_t(max_threshold_value1 * factor), size_t(max_threshold_value2 * factor), size_t(max_threshold_value3 * factor) }, 0);
	for (size_t k1 = 0; k1 < false_detection.sizes(0); ++k1)
	{
		RealFunction2D_F32	false_detection_slice;
		false_detection.GetSlice(false_detection_slice, { k1, slice_mask(0), slice_mask(1) });
		for (size_t j = 0; j < false_detection_slice.vsize(); ++j)
		{
			RealFunctionF32 row(false_detection_slice.hsize(), 0);
			RealFunctionF32::iterator it = row.begin();
			for (size_t i = 0; i < false_detection_slice.hsize(); ++i, ++it)
			{
				fscanf(f2, "%g, ", &(*it));
				float a = *it;
				false_detection_slice.at(j,i) = a;
			}
			//DisplayMathFunction(row, 0, 1, "row");
		}
	}
	fclose(f2);

	RealFunctionMD_F32 mlfunction(true_detection);
	mlfunction -= false_detection;
	size_t coordinate1(0), coordinate2(0), coordinate3(0);
	float max_value = mlfunction.at({0,0,0});

//	fread_numbers(slice, file, ioF32_LE);
//	fwrite_numbers(slice, file, ioScalarText);

	for (size_t k1 = 0; k1 < mlfunction.sizes(0); ++k1)
	{
		RealFunction2D_F32	mlfunction_slice;
		mlfunction.GetSlice(mlfunction_slice, { k1, slice_mask(0), slice_mask(1) });
		for (size_t k2 = 0; k2 < mlfunction.sizes(1); ++k2)
		{
			for (size_t k3 = 0; k3 < mlfunction.sizes(2); ++k3)
			{
				if (max_value < mlfunction_slice.at(k2,k3))
				{
					coordinate1 = k1;
					coordinate2 = k2;
					coordinate3 = k3;
					max_value = mlfunction_slice.at(coordinate2,coordinate3);
				}
			}
		}
	}
	RealFunction2D_F32	t_slice;
	true_detection.GetSlice(t_slice, { coordinate1, slice_mask(0), slice_mask(1) });
	RealFunction2D_F32	f_slice;
	false_detection.GetSlice(f_slice, { coordinate1, slice_mask(0), slice_mask(1) });
	string title = ssprintf(" max_value =  %g; \n STD = %g; \n Correlation = %g;\n Correlation Re Im = %g; \n true detection rate = %g; \n false datection rate = %g", max_value, float(coordinate1) / factor, float(coordinate2) / factor, float(coordinate3) / factor, t_slice.at(coordinate2,coordinate3), f_slice.at(coordinate2,coordinate3));
	ShowString("true_detection -= false_detection", title);
}

//TODO неиспользуемые переменные (warning 4100)
void ReadDetectionData_2D(const S500_CFMFrameSet &/*frames*/, float max_threshold_value1, float max_threshold_value2, float factor)
{
//	size_t factor(1000);
	RealFunction2D_F32 true_detection(max_threshold_value1*factor, max_threshold_value2*factor, 0);
	FILE	*f = fopen("c:\\temp\\true_detection_2D.txt", "rb");
	for (size_t j = 0; j < true_detection.vsize(); ++j)
	{
		RealFunctionF32 row(true_detection.hsize(), 0);
		RealFunctionF32::iterator it = row.begin();
		for (size_t i = 0; i < true_detection.hsize(); ++i, ++it)
		{
			fscanf(f, "%g, ", &(*it));
			float a = *it;
			true_detection.at(j,i) = a;
		}
		//DisplayMathFunction(row, 0, 1, "row");
	}
	fclose(f);

	RealFunction2D_F32 false_detection(true_detection);
	FILE	*f2 = fopen("c:\\temp\\false_detection_2D.txt", "rb");
	for (size_t j = 0; j < false_detection.vsize(); ++j)
	{
		RealFunctionF32 row(false_detection.hsize(), 0);
		RealFunctionF32::iterator it = row.begin();
		for (size_t i = 0; i < false_detection.hsize(); ++i, ++it)
		{
			fscanf(f2, "%g, ", &(*it));
			float a = *it;
			false_detection.at(j,i) = a;
		}
	}
	fclose(f2);

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
	DisplayMathFunction2D(true_detection, "true_detection", ScanFrameRectangle(cm(max_threshold_value1), cm(max_threshold_value2)));
	DisplayMathFunction2D(false_detection, "false_detection", ScanFrameRectangle(cm(max_threshold_value1), cm(max_threshold_value2)));
	string title = ssprintf("true_detection -= false_detection; max_value =  %g; std = %d; corelation = %d; true detection rate = %g; false datection rate = %g", max_value, coordinate1, coordinate2, true_detection.at(coordinate1,coordinate2), false_detection.at(coordinate1,coordinate2));
	DisplayMathFunction2D(mlfunction, title, ScanFrameRectangle(cm(max_threshold_value1), cm(max_threshold_value2)));
}





void	DisplayDopplerDataAdditional(
	const RealFunctionMD_F32 &correlation_frames,
	const RealFunctionMD_F32 &std_frames,
	const RealFunctionMD_F32 &amplitude_frames,
	const S500_CFMFrameSet &frames)
{
	RealFunctionMD_F32 amplitude_frames_reverse(amplitude_frames);

	RealFunctionMD_F32 std(std_frames);
	enum
	{
		mlc_1d,
		mlc_2d,
		mlc_3d,
		read_file_1d,
		read_file_2d,
		read_file_3d,
		display_exit

	} option;

	try
	{
		while(true)
		{
			option = GetButtonDecision("Choose additional task",
			{
			MakeButton("mlc_1D", mlc_1d),
			MakeButton("mlc_2D", mlc_2d),
			MakeButton("mlc_3D", mlc_3d),
			MakeButton("read_file_1D", read_file_1d),
			MakeButton("read_file_2D", read_file_2d),
			MakeButton("read_file_3D", read_file_3d),
			MakeButton("Exit", display_exit)
			}
			);
			switch(option)
			{
				case mlc_1d:
				{
					RealFunction2D_F32 mask(frames.n_cfm_beams(), frames.n_cfm_samples(), 0);
					size_t blood_area(0), noise_area(0);
					Calculate_mask(std_frames, frames, mask, blood_area, noise_area);
					CalculateMLC_1D(std_frames, frames, 1, L"std", mask, blood_area, noise_area);
				}
				break;

				case mlc_2d:
				{
					RealFunction2D_F32 mask(frames.n_cfm_beams(), frames.n_cfm_samples(), 0);
					size_t blood_area(0), noise_area(0);
					Calculate_mask(std_frames, frames, mask, blood_area, noise_area);
					CalculateMLC_2D(std_frames, correlation_frames, frames, 0.5, 1, mask, blood_area, noise_area, 1000);
					//CalculateMLC_2D(amplitude_frames_reverse, correlation_frames, frames, 1, 1, mask, blood_area, noise_area, 1000);
				}
				break;

				case mlc_3d:
				{
					RealFunction2D_F32 mask(frames.n_cfm_beams(), frames.n_cfm_samples(), 0);
					size_t blood_area(0), noise_area(0);
					Calculate_mask(std_frames, frames, mask, blood_area, noise_area);
					//TODO у следующей функции аргументы float лучше превратить в double (warning 4305)
					CalculateMLC_3D(std_frames, correlation_frames, amplitude_frames_reverse, frames, float(0.5), float(0.5), float(0.6), mask, blood_area, noise_area, 100);
				}
				break;

				case read_file_1d:
					ReadDetectionData_1D(frames, 1, L"Read File");
					break;

				case read_file_2d:
					ReadDetectionData_2D(frames, 4, 1, 100);
					break;

				case read_file_3d:
					ReadDetectionData_3D(frames, 4, 1, 1, 10);
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




/*

void	DisplayDuplex(const ColorImageMD_F32 &cfm_color_frames, const S500_CFMFrameSet &frames)
{
	//int threshold = GetLong("Threshold", 100, 0, 255);
	int threshold = 1;
	RealFunctionMD_F32 decibel_frames;
	MathFunctionMD<ColorImageF32> display_frames;
	MakeCopy(decibel_frames, frames.b_frames, amplitude_to_decibel_functor<float, complexF32>());
	decibel_frames -= MaxValue(decibel_frames);
	ApplyFunction(decibel_frames, range_functor<float>(-80, 0));
	decibel_frames += 80;
	decibel_frames *= 255./80;
	MakeCopy(display_frames, decibel_frames);
	decibel_frames.realloc({0,0,0});
	StartProgress("Creating animation", frames.n_frames);
	for(size_t frame_no = 0; frame_no < frames.n_frames; ++frame_no)
	{
		MathFunctionMD<ColorImageF32>::slice_type::invariable phase_color;
		cfm_color_frames.GetSlice(phase_color, {frame_no, slice_mask(0), slice_mask(1)});
		MathFunctionMD<ColorImageF32>::slice_type frame;
		display_frames.GetSlice(frame, {frame_no, slice_mask(0), slice_mask(1)});
		for(size_t i = 0; i < frames.n_cfm_beams; ++i)
		{
			for(size_t j = 0; j < frames.n_cfm_samples; ++j)   
			{
				if(phase_color.at(i,j) >= ColorSampleF32 (threshold,threshold,threshold))
				{
					for (size_t k = 0; k < frames.cfm_density; ++k)				
					{
						size_t	row = frames.cfm_density*i + frames.first_cfm_ray+k;
						size_t	col = j + frames.first_cfm_sample;
						frame.at(row,col)=phase_color.at(i,j);
					}
				}
			}
		}
		NextProgress();
	}
	EndProgress();
	ScanConverterOptions sco(ScanFrameRectangle(cm(6), cm(6)));
	sco.pixels_per_cm = 60;
	DisplayMathFunction3D(display_frames, "Duplex Mode", sco);
}


void ApplyCriterium(RealFunctionMD_F32 &flow_frames, RealFunctionMD_F32 &criterium_frames, const double &min_threshold, const double &max_threshold, const bool &blur_flag)
{
	size_t n_frames = flow_frames.sizes()[0];
	size_t n_beams = flow_frames.sizes()[1];
	size_t n_samples = flow_frames.sizes()[2];
	StartProgress("Applying Criterium", n_frames);
	for (size_t frame_no = 0; frame_no < n_frames; ++frame_no)
	{
		RealFunctionMD_F32::slice_type flow_slice;
		flow_frames.GetSlice(flow_slice, {frame_no, slice_mask(0), slice_mask(1)});
		RealFunctionMD_F32::slice_type criterium_slice;
		criterium_frames.GetSlice(criterium_slice, {frame_no, slice_mask(0), slice_mask(1)});
		for (size_t i = 0; i < n_samples; ++i)
		{
			RealFunctionF32::iterator	it = flow_slice.col(i).begin();
			RealFunctionF32::const_iterator	criterium_it = criterium_slice.col(i).cbegin();
			for (size_t beam_no = 0; beam_no < n_beams; ++beam_no, ++it, ++criterium_it)
			{
				if ((*criterium_it < min_threshold) || (*criterium_it > max_threshold))
				{
					*it = 0;
				}
			}
		}
		NextProgress();
	}
	EndProgress();
	bool flag_blur_frames(false);
	if (blur_flag == true)
	{ 
		flag_blur_frames = YesOrNo("Blur data?", true);
	}
	if (flag_blur_frames)
	{
		double	r_time(2), r_lateral(1), r_axial(8);
		r_time = GetUnsigned("Time blur", 2, 0, 10);
		r_lateral = GetUnsigned("Lateral blur", 0, 0, 10);
		r_axial = GetUnsigned("Axial blur", 2, 0, 10);
		BiexpBlur3D(flow_frames, r_time, r_lateral, r_axial);
	}
}

 RealFunctionMD_F32 ChooseCritetium(const RealFunctionMD_F32 &correlation_frames, const RealFunctionMD_F32 &re_im_correlation_frames, const RealFunctionMD_F32 &stddev_frames_conj, const RealFunctionMD_F32 &average_frames, double &min_threshold, double &max_threshold)
 {
	 size_t	option = 0;
	 enum { correlation, re_im_correlation, average, std_conj, n_options };
	 option = GetButtonDecision("Choose Criterium", { "correlation", "re im correlation", "average", "std_conj" });
	 switch(option)
	 {
	 case correlation:
		 {
			 min_threshold = GetFloating("Choose minimmum correlation Threshold", 0.8, 0, 1);
			 max_threshold = GetFloating("Choose maximum correlation Threshold", 1.0, min_threshold, 1);
			 return correlation_frames;
		 }
		 break;
	 case re_im_correlation:
		 {
			 min_threshold = GetFloating("Choose minimmum Re Im correlation Threshold", 0.8, 0, 1);
			 max_threshold = GetFloating("Choose maximum Re Im correlation Threshold", 1.0, min_threshold, 1);
			 return re_im_correlation_frames;
		 }
		 break; 
	 case average:
		 {
			 min_threshold = GetUnsigned("Choose minimmum average Threshold", 1000000, 0, 1000000000);
			 max_threshold = GetUnsigned("Choose maximum correlation Threshold", 1000000000, min_threshold, 1000000000);
			 return average_frames;
		 }
	 break;
	 case std_conj:
		 {
			 min_threshold = GetUnsigned("Choose minimmum std_conj Threshold", 3000, 0, 10000000);
			 max_threshold = GetUnsigned("Choose maximum correlation Threshold", 10000000, min_threshold, 10000000);
			 return stddev_frames_conj;
		 }
		 break;
	 default:
		 throw canceled_operation("");
	 }
 }

 
//TODO распутывать!
void ChooseCFMMap(const RealFunctionMD_F32 &in_cfm_phase_frames, const RealFunctionMD_F32 &in_cfm_turbulence_frames, const S500_CFMFrameSet &frames,
				  const RealFunctionMD_F32 &correlation_frames, const RealFunctionMD_F32 &re_im_correlation_frames, const RealFunctionMD_F32 &stddev_frames_conj,
				  const RealFunctionMD_F32 &average_frames)
{
	RealFunctionMD_F32 cfm_phase_frames(in_cfm_phase_frames);
	RealFunctionMD_F32 cfm_turbulence_frames(in_cfm_turbulence_frames);
	ColorImageMD_F32 cfm_color_frames;
	size_t colour = 0;
	string caption = "Choose Map";
	bool flag(false);
	enum {cfm_gray, cfm_standard, cfm_enhanced, cfm_variance, b_cfm, criteria, save_frame, display_error, display_error_phantom, display_exit, n_options};
	do
	{
		colour = GetButtonDecision(caption, { "Gray", "Standard", "Enhanced", "Variance", "Duplex Mode", "Criteria", "Save Frame", "Display Error (Model)","Display Error (Phantom)","Exit" });
		try
		{
			switch(colour)
			{
			case cfm_gray:
				if (!flag)
				{
					DisplayMathFunction3D(cfm_phase_frames, "CFM Phase GrayScale", frames.sco_cfm);
				}
				else 
				{
					RealFunctionMD_F32 decibel_frames;
					ColorImageMD_F32 display_frames;
					MakeCopy(decibel_frames, cfm_phase_frames, amplitude_to_decibel_functor<float,float>());
					decibel_frames -= MaxValue(decibel_frames);
					ApplyFunction(decibel_frames, range_functor<float>(-80, 0));
					decibel_frames += 80;
					decibel_frames *= 255./80;
					MakeCopy(display_frames, decibel_frames);
					decibel_frames.realloc({0,0,0});
					DisplayDuplex(display_frames, frames);
				}
				break;

			case cfm_standard:
				{
					MakeCopy(cfm_color_frames, cfm_phase_frames, cfm_standart_palette());
					if (!flag)
					{
//						ScanConverterOptions sco(ScanFrameRectangle(cm(10), cm(10)));
// 						DisplayCFMDetailed(cfm_color_frames, n_shots, "CFM Standard Map", sco);
						DisplayMathFunction3D(cfm_color_frames, "CFM Standard Map", frames.sco_cfm);
					}
					else
						DisplayDuplex(cfm_color_frames, frames);
				}
				break;

			case cfm_enhanced:
				{
					MakeCopy(cfm_color_frames, cfm_phase_frames, cfm_enhanced_palette());
					if (!flag)
						DisplayMathFunction3D(cfm_color_frames, "CFM Enhanced Map", frames.sco_cfm);
					else
						DisplayDuplex(cfm_color_frames, frames);
				}
				break;

			case cfm_variance:
				{			
					ColorImageMD_F32 turbulence_color_frames;
					MakeCopy(turbulence_color_frames, cfm_turbulence_frames, turbulence_palette());
					MakeCopy(cfm_color_frames, cfm_phase_frames, cfm_standart_palette());
					// Накладываем карту турбулентности на карту скорости
					StartProgress("Creating CFMMode animation", frames.n_frames);
					for(size_t frame_no = 0; frame_no < frames.n_frames; ++frame_no)
					{
						ColorImageMD_F32::slice_type phase_color;
						cfm_color_frames.GetSlice(phase_color, {frame_no, slice_mask(0), slice_mask(1)});
						ColorImageMD_F32::slice_type turbulence_color;
						turbulence_color_frames.GetSlice(turbulence_color, {frame_no, slice_mask(0), slice_mask(1)});
						for(size_t i = 0; i < frames.n_cfm_samples; ++i)   //горизонталь
						{
							for(size_t j = 0; j < frames.n_cfm_beams; ++j)   //вертикаль 
							{
								turbulence_color.at(j,i)+=phase_color.at(j,i);
							}
						}
						NextProgress();
					}
					EndProgress();
					if (!flag)
						DisplayMathFunction3D(turbulence_color_frames, "CFM Variance Map", frames.sco_cfm);
					else 
						DisplayDuplex(turbulence_color_frames, frames);
				}
				break;

			case b_cfm:
				{
					flag = !flag;
					if (caption == "Choose Map") caption = "Duplex Mode";
					else caption = "Choose Map";
				}
				break;

			case criteria:
				{
				double min_threshold(0);
				double max_threshold(1);
				RealFunctionMD_F32 criterium = ChooseCritetium(correlation_frames, re_im_correlation_frames, stddev_frames_conj, average_frames, min_threshold, max_threshold);
 				ApplyCriterium(cfm_phase_frames, criterium, min_threshold, max_threshold, true);
				}
				break;

			case save_frame:
				{
					size_t frame_number = GetUnsigned("Frame number", 0, 0, cfm_phase_frames.sizes(0) - 1);
					FILE *file = fopen("q:/raw_numbers.txt", "wb");
					RealFunction2D_F32	slice;
					cfm_phase_frames.GetSlice(slice, {frame_number, slice_mask(0), slice_mask(1)});
					for (size_t i = 0; i < slice.vsize(); ++i)
					{
						for (size_t j = 0; j < slice.hsize(); ++j)
						{
							fprintf(file, "%lg ", slice.at(i,j));
						}
						fprintf(file, "\n");
					}
					fclose(file);
					ShowText("Frame Saved", ssprintf("Frame #%d is saved at q:/raw_numbers.txt", frame_number));
				}
			break;
			case display_error:
			{
				RealFunctionF32 phase(frames.n_cfm_samples, 0);
				RealFunctionF32 ideal_phase(frames.n_cfm_samples, 0);
				RealFunctionF32 mean_phase(frames.n_cfm_samples, 0);
				double err(0), err_all_frames(0);
				for (size_t frame_no = 0; frame_no < frames.n_frames; ++frame_no)
				{
					RealFunction2D_F32 frame;
					cfm_phase_frames.GetSlice(frame, { frame_no, slice_mask(0), slice_mask(1) });
					for (size_t i = 0; i < frames.n_cfm_samples; ++i)
					{
						phase[i] = frame.at(frames.n_cfm_beams / 2,i)/0.6;
						ideal_phase[i] = (double(i) / frames.n_cfm_samples*1.2 - 0.6) / 0.6;
						err = phase[i] - ideal_phase[i];
						mean_phase[i] += phase[i] / frames.n_frames;
						err_all_frames += err*err / (frames.n_cfm_samples*frames.n_frames);
					}
				}
				DisplayMathFunction(ideal_phase, 0, 1, "ideal_phase");
				DisplayMathFunction(mean_phase, 0, 1, ssprintf("mean_phase, err_all_frames(std)=%g", sqrt(err_all_frames)));
			}
			break;
			case display_error_phantom:
			{
				RealFunctionF32 phase(frames.n_cfm_beams, 0);
				RealFunctionF32 ideal_phase(frames.n_cfm_beams, 0);
				RealFunctionF32 mean_phase(frames.n_cfm_beams, 0);
				RealFunctionF32 y(frames.n_cfm_beams, 0);
				double err(0), err_all_frames(0);
				size_t n_thresholds(300);
				RealFunctionF32 pixels_in_region(n_thresholds,0);
				RealFunctionF32 all_pixels(n_thresholds,0);
				RealFunctionF32 pixels_outside_region(n_thresholds,0);
				double blood_threshold(0);
				double pow_blood = GetUnsigned("pow_blood", 1.2,0,5);
				for (size_t frame_no = 0; frame_no < frames.n_frames; ++frame_no)
				{
					RealFunction2D_F32 frame;
					cfm_phase_frames.GetSlice(frame, { frame_no, slice_mask(0), slice_mask(1) });
					for (size_t i = 0; i < frames.n_cfm_beams; ++i)
					{
						y[i] = 0.172*i*i - 8.092*i + 194;
						size_t j = int(y[i]) + 1;
						phase[i] = frame.at(i,j);
						ideal_phase[i] = 0.6*cos((double(i)*pi() / 53 + pi()*0.05));
						err = phase[i] - ideal_phase[i];
						mean_phase[i] += phase[i] / frames.n_frames;
						err_all_frames += err*err / (frames.n_cfm_beams*frames.n_frames);
					}
					for (size_t threshold = 0; threshold < n_thresholds; ++threshold)
					{
						blood_threshold = pow(double(threshold) / (n_thresholds),pow_blood);
						for (size_t i = 0; i < frames.n_cfm_beams; ++i)
						{
							size_t j = int(y[i]) + 1;
							size_t j_below = j - 40;
							size_t j_above = j + 30;
							for (size_t sample_no = j_below; sample_no < j_above; ++sample_no)
							{
								if (abs(frame.at(i,sample_no)) > blood_threshold)
								{
									pixels_in_region[threshold] += 1;
								}
							}
							for (size_t sample_no = 0; sample_no < frames.n_cfm_samples; ++sample_no)
							{
								if (abs(frame.at(i,sample_no)) > blood_threshold)
								{
									all_pixels[threshold] += 1;
								}
							}
						}
						pixels_outside_region[threshold] = all_pixels[threshold] - pixels_in_region[threshold];
					}
				}
				double flow_size_all_frames(pixels_in_region[0]);
				RealFunctionF32 true_positive_rate(pixels_in_region);
				true_positive_rate /= flow_size_all_frames;
				double frame_size_all_frames = frames.n_frames*frames.n_cfm_beams*frames.n_cfm_samples;
				double clutter_size_all_frames = frame_size_all_frames - flow_size_all_frames;
				RealFunctionF32 false_positive_rate(pixels_outside_region);
				false_positive_rate  /= clutter_size_all_frames;
				DisplayMathFunction(false_positive_rate, 0, 1, "false_positive_rate");
				DisplayMathFunction(true_positive_rate, 0, 1, "true_positive_rate");
				DisplayMathFunction(ideal_phase, 0, 1, "ideal_phase");
				DisplayMathFunction(mean_phase, 0, 1, ssprintf("mean_phase, err_all_frames(std)=%g", sqrt(err_all_frames)));
			}
			break;
			case display_exit:
				break;
			}
		}
		catch(canceled_operation &) {}
		catch(quit_application &ex){throw ex;}
		catch(exception &ex){Error(ex.what());}
		catch(...){Error("An unknown exception");}
	}while(colour != n_options-1);
}
*/


/*
void	ChoosePowerMap(const RealFunctionMD_F32 &in_cfm_power_frames, const S500_CFMFrameSet &frames, 
					   const RealFunctionMD_F32 &correlation_frames, const RealFunctionMD_F32 &re_im_correlation_frames, const RealFunctionMD_F32 &stddev_frames_conj,
					   const RealFunctionMD_F32 &average_frames)
{
	RealFunctionMD_F32 cfm_power_frames(in_cfm_power_frames);
	size_t colour = 0;
	string caption = "Choose Map";
	bool flag(false);
	enum {gray, standard, duplex_mode, critetium, display_exit, n_options};
	do
	{
		colour = GetButtonDecision(caption, { "Gray", "Standard", "Duplex Mode", "Criteria", "Exit" });
		try
		{
			switch(colour)
			{
			case gray:
				{
					RealFunctionMD_F32 decibel_frames;
					ColorImageMD_F32 display_frames;
					MakeCopy(decibel_frames, cfm_power_frames, amplitude_to_decibel_functor<float,float>());
					decibel_frames -= MaxValue(decibel_frames);
					ApplyFunction(decibel_frames, range_functor<float>(-80, 0));
					decibel_frames += 80;
					decibel_frames *= 255./80;
					MakeCopy(display_frames, decibel_frames);
					decibel_frames.realloc({0,0,0});
					if (!flag)
						DisplayMathFunction3D(display_frames, "PowerMode GrayScale");	
					else 
						DisplayDuplex(display_frames, frames);
				}
				break;
			case standard:
				{
					ColorImageMD_F32 power_color_frames;
					MakeCopy(power_color_frames, cfm_power_frames, power_palette());
					if (!flag)
						DisplayMathFunction3D(power_color_frames, "PowerMode Standard");	
					else 
						DisplayDuplex(power_color_frames, frames);
				}
				break;
			case duplex_mode:
				{
					flag = !flag;
					if (caption == "Choose Map") caption = "Duplex Mode";
					else caption = "Choose Map";
				}
				break;
			case critetium:
				{
					double min_threshold(0);
					double max_threshold(1);
					RealFunctionMD_F32 criterium = ChooseCritetium(correlation_frames, re_im_correlation_frames, stddev_frames_conj, average_frames, min_threshold, max_threshold);
					ApplyCriterium(cfm_power_frames, criterium, min_threshold, max_threshold, true);
				}
				break;
			case display_exit:
				break;
			}
		}
		catch(canceled_operation &) {}
		catch(quit_application &ex){throw ex;}
		catch(exception &ex){Error(ex.what());}
		catch(...){Error("An unknown exception");}
	}while(colour != n_options-1);
}

void	ChooseTurbulenceMap(const RealFunctionMD_F32 &in_cfm_turbulence_frames, const S500_CFMFrameSet &frames,
							const RealFunctionMD_F32 &correlation_frames, const RealFunctionMD_F32 &re_im_correlation_frames, const RealFunctionMD_F32 &stddev_frames_conj,
							const RealFunctionMD_F32 &average_frames)
{
	RealFunctionMD_F32 cfm_turbulence_frames(in_cfm_turbulence_frames);
	size_t colour = 0;
	string caption = "Choose Map";
	bool flag(false);
	enum {gray, standard, duplex_mode, criteria, display_exit, n_options};
	do
	{
		colour = GetButtonDecision(caption, { "Gray", "Standard", "Duplex Mode", "Criteria", "Exit" });
		try
		{
			switch(colour)
			{
			case gray:
				{
					RealFunctionMD_F32 decibel_frames;
					ColorImageMD_F32 display_frames;
					MakeCopy(decibel_frames, cfm_turbulence_frames, amplitude_to_decibel_functor<float,float>());
					decibel_frames -= MaxValue(decibel_frames);
					ApplyFunction(decibel_frames, range_functor<float>(-80, 0));
					decibel_frames += 80;
					decibel_frames *= 255./80;
					MakeCopy(display_frames, decibel_frames);
					decibel_frames.realloc({0,0,0});
					if (!flag)
						DisplayMathFunction3D(display_frames, "TurbulenceMode GrayScale");	
					else 
						DisplayDuplex(display_frames, frames);
				}
				break;
			case standard:
				{
					ColorImageMD_F32 turbulence_color_frames;
					MakeCopy(turbulence_color_frames, cfm_turbulence_frames, turbulence_palette());
					if (!flag)
						DisplayMathFunction3D(turbulence_color_frames, "TurbulenceMode Standard");	
					else 
						DisplayDuplex(turbulence_color_frames, frames);
				}
				break;
			case duplex_mode:
				{
					flag = !flag;
					if (caption == "Choose Map") caption = "Duplex Mode";
					else caption = "Choose Map";
				}
				break;
			case criteria:
				{
					double min_threshold(0);
					double max_threshold(1);
					RealFunctionMD_F32 criterium = ChooseCritetium(correlation_frames, re_im_correlation_frames, stddev_frames_conj, average_frames, min_threshold, max_threshold);
					ApplyCriterium(cfm_turbulence_frames, criterium, min_threshold, max_threshold, true);
				}
				break;
			case display_exit:
				break;
			}
		}
		catch(canceled_operation &) {}
		catch(quit_application &ex){throw ex;}
		catch(exception &ex){Error(ex.what());}
		catch(...){Error("An unknown exception");}
	}while(colour != n_options-1);
}
*/


#if 0
//Удаленный фрагмент из основной функции DopplerDataDisplay
			case display_CFM_mode:
			{
				DisplayMathFunction3D(cfm_phase_frames, "Phase", frames.sco_cfm);
			#if 0
				RealFunctionMD_F32 criteria_data(correlation_re_im_frames);
				RealFunctionMD_F32 velocity_data1(cfm_phase_frames);
				RealFunctionMD_F32 velocity_data2(mask_frames);
				double threshold(0.7);
				threshold = GetFloating("Choose Re-Im Correlation Threshold", 0.8, 0, 1);
				double mean_velocity1(0);
				double mean_velocity2(0);
				double counter(0);
				RealFunction2D_F32 frame(frames.n_cfm_beams, frames.n_cfm_samples, 0);
				for(size_t frame_no = 0; frame_no < frames.n_frames; ++frame_no)
				{
					RealFunction2D_F32 criteria_slice;
					criteria_data.GetSlice(criteria_slice, {0, slice_mask(0), slice_mask(1)});
					RealFunction2D_F32 velocity_slice1;
					velocity_data1.GetSlice(velocity_slice1, {0, slice_mask(0), slice_mask(1)});
					RealFunction2D_F32 velocity_slice2;
					velocity_data2.GetSlice(velocity_slice2, {0, slice_mask(0), slice_mask(1)});
					for(size_t i = 0; i < frames.n_cfm_beams; ++i)
					{
						for(size_t j = 0; j < frames.n_cfm_samples; ++j)
						{
							double a(criteria_slice.at(i, j));
							if(a >= threshold)
							{
								mean_velocity1 += velocity_slice1.at(i, j);
								mean_velocity2 += velocity_slice2.at(i, j);
								counter += 1;
							}
							frame.at(i, j) += velocity_slice1.at(i, j);
						}
					}
				}
				mean_velocity1 /= counter;
				mean_velocity2 /= counter;
				ShowText("mean_velocity", ssprintf("first estimation %g\r\nsecond estimation %g\r\nthreshold %g", mean_velocity1, mean_velocity2, threshold), true);
				frame /= frames.n_frames;
				DisplayMathFunction2D(frame, ssprintf("frame_velocity; first estimation %g, second estimation %g", mean_velocity1, mean_velocity2), frames.sco_cfm);
			#endif
			}

#endif

XRAD_END
