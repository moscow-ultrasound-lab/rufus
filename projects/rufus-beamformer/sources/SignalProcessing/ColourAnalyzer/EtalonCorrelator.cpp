#include "pre.h"
#include <XRADBasic/DataArrayIO.h>

//#error ("здесь что-то не так работает со скан-конвертером при выборе эталонов")

#include <XRADBasic/Sources/Utils/StatisticUtils.h>
#include <Attic/LABColorImage.h>
#include <XRADBasic/Sources/Utils/LeastSquares.h>

#include "SignalProcessing/GaussSignalTools.h"
#include "SignalProcessing/ColourAnalyzer/EtalonCorrelator.h"
#include <Utils/SignalFilters.h>
#include <XRADBasic/Sources/SampleTypes/HLSColorSample.h>
#include "ColourEncode.h"
#include <XRADBasic/Sources/Utils/Crayons.h>
//#include <TimeProfiler.h>
//#include <ImageUtils.h>

#include <XRADBasic/Sources/Utils/GradientPalette.h>
#include <MatrixAlgorithms/EigenVectorTools.h>


/*

запишу длЯ памЯти, при каких условиЯх получен более удачный результат.

1. ЌормализациЯ спектра;
2. ЌормализациЯ Яркости;
3. ”азоваЯ производнаЯ (так, как она сейчас там записана -- см.)
4. по таким данным набор эталонов (неважно на каком участке)
5. и далее декомпозициЯ. по одному набору эталонов (без разностей)

*/

XRAD_BEGIN

namespace
{
ColorSampleF64	desaturate(const ColorSampleF64 &s, float factor)
{
	HLSColorSample	hls(s);
	hls.S *= factor;
	return hls;
}
}//namespace


//----------------------------------------------------------------------------



//----------------------------------------------------------------------------

void	EtalonCorrelator::Colourize()
{
	FILE	*buffer_file = fopen("buffer_file.tmp", "rb");
	GrayScanConverter	param;
	size_t n_rays_read = n_rays;
	size_t n_samples_read = n_samples;
	float	divisor = 1;


	if(buffer_file)
	{
		fread(&n_rays_read, sizeof(int), 1, buffer_file);
		fread(&n_samples_read, sizeof(int), 1, buffer_file);

		if(n_rays_read != n_rays || n_samples_read != n_samples_read)
		{
			Error("Improper colourization data file ");
			return;
		}

		param.realloc(n_rays_read, n_samples_read);
		ExportScanConverterOptions(param);
		param.InitScanConverter();
		param.SetFlip(true);
//		param.SetImageTitle("Parameter");


		for(size_t i = 0; i < n_rays_read; i++) fread_numbers(sample_brightness.row(i), buffer_file, ioF32_LE);
		for(size_t i = 0; i < n_rays_read; i++) fread_numbers(param.row(i), buffer_file, ioF32_LE);
		fclose(buffer_file);

		DisplayMathFunction2D(sample_brightness, "Sample brightness", sco);
		DisplayMathFunction2D(param, "Parameter", sco);


		divisor = GetFloating("Divisor value", 1, 0.5, 10);
		param /= divisor;
	}
	else
	{
		Error("Could not read pre-computed parameters file");
		return;
	}


	const	size_t N = 7;
	GradientPalette	gp(N);
//	pb.SetColorRange(0, 4);

	double	pp[N] ={
			0.0,
			0.75,
			0.85,
			1.0,
			1.2,
			2,
			4.0
	};
	double	ds = 0.5;
	ColorSampleF32 cc[N] ={
		crayons::white(),
		crayons::white(),
		desaturate(crayons::green(), ds/2),
		crayons::yellow(),
		desaturate(crayons::red(), ds),
		desaturate(crayons::magenta(), ds),
		desaturate(crayons::blue(), ds)
	};

	for(size_t n = 0; n < N; ++n)
	{
		gp.SetColor(n, make_pair(pp[n], cc[n]));
	}


	ColorImageF32	saturation(n_rays, n_samples);

	ColorImageF32	palette(512, 64);

	for(size_t i = 0; i < 512; i++)
	{
		float	x = float(i)*2/512;
		palette.row(i).fill(gp(x));
	}

	DisplayMathFunction2D(palette, "Palette");
	for(size_t i = 0; i < n_rays; i++)
	{
		for(size_t j = 0; j < n_samples; j++)
		{
			float	p = param.at(i,j);

//			HLSColorSample	saturation_s = pb.GenerateColor(p) * sample_brightness.at(i,j)/255;
			ColorSampleF32	s = gp(p) * float(sample_brightness.at(i,j))/255.;
//			ColorSampleF32	s = crayons::red() * sample_brightness.at(i,j)/255.;
//			float	sat;
//			saturation_s.S = saturation.at(i,j);
			if(CapsLock())
			{
				printf("%d:%d/t%g/t%g/t%g\n", int(i), int(j), s.red(), s.green(), s.blue());
				fflush(stdout);
			}
			parametric_picture.at(i,j) = s;
		}
	}
//	saturation.Display("S");

	DisplayMathFunction2D(parametric_picture, "parametric picture", sco);

//	param.Display("param");
	//DebugQuit();
	throw(runtime_error("Procedure not finished"));
}






//void	EtalonCorrelator :: ProcessInitDialog(){}

EtalonCorrelator::EtalonCorrelator() :RangeSpectrumAnalyzer(32)
{
	strcpy(SIMIO::Process_Name, "EtalonCorrelator");
	color_encode = &EncodeHLS;
//	color_encode = &EncodeLAB;
//	color_encode = &EncodeXYZ;

	//SetDepthUnits(CENTIMETRES);
}

EtalonCorrelator :: ~EtalonCorrelator()
{
}


void	EtalonCorrelator::InitWork()
{
	parent::InitWork();

	etalon_halfwidth = GetSigned("Etalon halfwidth", 0, 0, 5);
	etalon_length = GetSigned("Etalon length", 32, 16, 64);

	if(YesOrNo("Colourize pre-computed?", false)) Colourize();

	if(YesOrNo("Normalize spectrum?", true))
	{
		size_t spectrum_normalizer_range_length = GetSigned("Spectrum normalizer range length", 32, 16, n_samples/4);
		NormalizeSpectrum(spectrum_normalizer_range_length);
	}

	if(YesOrNo("Normalize power", true))
	{
		NormalizeBrightness();
	}

	correlation_map.realloc(n_rays, n_ranges);
}




void	EtalonCorrelator::EndWork()
{
}



//extern	bool	ATL_data;





// Decomposer



void	EtalonCorrelator::ComputeDecompositionPlane(size_t sub, const ComplexVectorF32 &basis_vector, ComplexFunction2D_F32 &decomposition)
{

//	SetCurrentFrame(sub);
	size_t ft_size = ceil_fft_length(n_samples);
	ComplexFunctionF32	data_buffer(ft_size), filter_buffer(ft_size);

	filter_buffer.CopyData(basis_vector);
	filter_buffer.roll(-int(basis_vector.size())/2);
//	filter_buffer.Display(0,1,"filter_buffer");
	FFT(filter_buffer, ftForward);

	for(size_t i = 0; i < n_rays; i++)
	{
		ComplexFunctionF32	CurrentRay;
		focused_data.GetRow(CurrentRay, {sub, i, slice_mask(0)});
//		SetCurrentRay(i);
		data_buffer.CopyData(CurrentRay);
		FFT(data_buffer, ftForward);
		data_buffer *= filter_buffer;
		FFT(data_buffer, ftReverse);
		decomposition.row(i).CopyData(data_buffer);
	}

}

namespace
{
void	RemoveSpike(RealFunction2D_F32 &im, size_t len)
{
	size_t n_rays = im.vsize();
	size_t n_samples = im.hsize();

	for(size_t j = 0; j < n_samples; ++j) im.col(j).FilterMedian(3);
	for(size_t i = 0; i < n_rays; ++i) im.row(i).FilterMedian(len);

	for(size_t j = 0; j < n_samples; ++j) im.col(j).FilterGauss(1.5);
	for(size_t i = 0; i < n_rays; ++i) im.row(i).FilterGauss(double(len)/2);
}

void	CheckOrthogonality(ComplexFunction2D_F32 &eigen_vectors)
{
	size_t es = eigen_vectors.vsize();
	ComplexFunction2D_F32 sp(es, es);

	for(size_t i = 0; i < es; ++i)
	{
		for(size_t j = 0; j < es; ++j)
			for(size_t k = 0; k < es; ++k)
			{
				sp.at(i,j) += eigen_vectors.col(i)[k] % eigen_vectors.col(j)[k];
			}
	}
	DisplayMathFunction2D(sp, "Orthogonality");
}
}

void	RemoveEdgeArtifacts(RealFunction2D_F32 &im)
{
	double	average_value = AverageValue(im);
	for(size_t j = im.hsize()-60; j < im.hsize(); ++j)im.col(j).fill(average_value);
	for(size_t j = 0; j < 30; ++j)im.col(j).fill(average_value);
}


void	EtalonCorrelator::PerformDecomposition(const ComplexMatrixF32 &eigen_vectors, DataArray<RealFunction2D_F32> &sb)
{
	size_t es = eigen_vectors.vsize();
//	DataArray<RealFunction2D_F32> sb(es);
	sb.realloc(es);


	GUIProgressBar	progress;
	progress.start("Components analysis", es*n_frames);

	ComplexFunction2D_F32	decomposition(n_rays, n_samples);
	RealFunction2D_F32	normalizer(n_rays, n_samples);
	for(size_t n = 0; n < es; ++n)
	{
		sb[n].realloc(n_rays, n_samples);
		for(size_t sub = 0; sub < n_frames; sub++)
		{
			ComputeDecompositionPlane(sub, eigen_vectors.col(n), decomposition);

			for(size_t i = 0; i < n_rays; i++)
			{
				ComplexFunctionF32::iterator	dc = decomposition.row(i).begin();
				RealFunctionF32::iterator	sbi = sb[n].row(i).begin();
				for(size_t j = 0; j < n_samples; ++j, ++dc, ++sbi)
				{
//					sb[n].at(i,j) += cabs(decomposition.at(i,j));
					*sbi += cabs(*dc);
				}
			}
			++progress;
		}

	}
	progress.end();
	//EndProgress();

	for(size_t sub = 0; sub < n_frames; sub++)
	{
//		SetCurrentFrame(sub);
		for(size_t i = 0; i < n_rays; i++)
		{
//			SetCurrentRay(i);
			ComplexFunctionF32	CurrentRay;
			focused_data.GetRow(CurrentRay, {sub, i, slice_mask(0)});
			ComplexFunctionF32::iterator	pb = CurrentRay.begin();
			RealFunctionF32::iterator	norm = normalizer.row(i).begin();
			for(size_t j = 0; j < n_samples; ++j, ++pb, ++norm)
			{
				*norm += cabs(*pb);
			}
		}
	}
	static	float	scale_factor = 4;
	float	blur_scale = float(es)/scale_factor;
	for(size_t i = 0; i < n_rays; i++) normalizer.row(i).FilterGauss(blur_scale, 0.3, extrapolation::by_last_value);
	for(size_t j = 0; j < n_samples; j++) normalizer.col(j).FilterGauss(3, 0.3, extrapolation::by_last_value);


	progress.start("Normalizing decomposition", es);
	for(size_t n = 0; n < es; ++n)
	{
	/*
	sb[n] /= AverageValue(sb[n]);

//		for(size_t j = 1780; j < n_samples; ++j)sb[n].col(j).fill(1);
		for(size_t j = n_samples-60; j < n_samples; ++j)sb[n].col(j).fill(1);
		for(size_t j = 0; j < 30; ++j)sb[n].col(j).fill(1);
		*/
		RemoveEdgeArtifacts(sb[n]);
		if(CapsLock()) DisplayMathFunction2D(sb[n], "Decomposition");

		CutHistogramEdges(sb[n], range1_F64(0.001, 0.999));
		NormalizeImage(sb[n], 0, 255);
		++progress;
	}
	progress.end();



	sample_brightness.fill(0);
	for(size_t sub = 0; sub < n_frames; sub++)
	{
//		SetCurrentFrame(sub);
		for(size_t j = 0; j < n_rays; j++)
		{
			ComplexFunctionF32	CurrentRay;
			focused_data.GetRow(CurrentRay, {sub, j, slice_mask(0)});
//			SetCurrentRay(j);
			for(size_t i = 0; i < n_samples; i++)
			{
				sample_brightness.at(j,i) += cabs(CurrentRay[i]);
			}
		}
	}

	LogCompress(sample_brightness);
	CutHistogramEdges(sample_brightness, range1_F64(0.001, 0.999));
	NormalizeImage(sample_brightness, 0, 255);


	FILE	*buffer_file = fopen("buffer_file.tmp", "wb");
	if(buffer_file)
	{
		fwrite(&n_rays, sizeof(int), 1, buffer_file);
		fwrite(&n_samples, sizeof(int), 1, buffer_file);

		for(size_t i = 0; i < n_rays; i++) fwrite_numbers(sample_brightness.row(i), buffer_file, ioF32_LE);
		fclose(buffer_file);
	}


	CutHistogramEdges(sample_brightness, range1_F64(0.001, 0.999));
	NormalizeImage(sample_brightness, 0, 255);
//	sample_brightness.SetImageTitle("Accumulation");	
}



double	CovariationCoefficient(const RealFunction2D_F32& im1, const RealFunction2D_F32& im2)
{
	double	m1 = AverageValue(im1);
	double	m2 = AverageValue(im2);
	double	cc(0);
	double	divisor1(0), divisor2(0);

	for(size_t i = 0; i < im1.vsize(); ++i)
	{
		for(size_t j = 0; j < im1.hsize(); ++j)
		{
			double	v1 = im1.at(i,j) - m1;
			double	v2 = im2.at(i,j) - m2;

			cc += v1 * v2;
			divisor1 += square(v1);
			divisor2 += square(v2);
		}
	}
	if(cc) return (cc / sqrt(divisor1*divisor2));
	else return 0;
}



void	AnalyzeDecomposition(DataArray<RealFunction2D_F32> &sb)
{
	size_t s = sb.size();
	ComplexMatrixF32	correlations(s, s);
	ComplexMatrixF32	eigen_vectors(s, s);
	ComplexFunctionF32	a1(s), a2(s), a3(s);
	RealVectorF32	lambda(s);
	Pause();
	DataArray<RealFunction2D_F32> sbc(sb);


	for(size_t i = 0; i < s; ++i)
	{
		sb[i] -= AverageValue(sb[i]);
		for(size_t j = 0; j < s; ++j)
		{
			correlations.at(i,j) = CovariationCoefficient(sb[i], sb[j]);
		}
		a1[i] = AverageValue(correlations.row(i));
	}
	DisplayMathFunction(a1, 0, 1, "a1");
	DisplayMathFunction2D(correlations, "CCs");

	bool	result = eigenvectors_hermit(correlations, eigen_vectors, lambda);
	if(!result)
	{
		Error("Eigen vectors not found");
	}
	else
	{	
		DisplayMathFunction2D(eigen_vectors, "EV");
		DisplayMathFunction(lambda, 0, 1, "lambda");
	}


	for(size_t i = 0; i < s; ++i)
	{
		sb[i].fill(0);
		for(size_t j = 0; j < s; ++j)
		{
			sb[i] += sbc[j]*real(correlations.at(i,j));
			//sb[i] += sbc[j]*real(eigen_vectors.at(j,i));
			//sb[i] += sbc[j]*real(eigen_vectors.at(i,j));
		}
		RemoveEdgeArtifacts(sb[i]);
	}

	for(size_t i = 0; i < s; ++i)
	{
		correlations.at(i,i) = 0;
		a2[i] = AverageValue(correlations.row(i));
	}

	DisplayMathFunction(a2, 0, 1, "a2");
	DisplayMathFunction2D(correlations, "CCs, no main diagonal");
}





void	EtalonCorrelator::PrepareResultingImages()
{
//	SetFrequencyUnits(M_HERZ);
//	float	f_factor = sample_rate.MHz()/etalon_length;

	do_once{
		sample_brightness.fill(0);
		for(size_t sub = 0; sub < n_frames; sub++)
			{
//			SetCurrentFrame(sub);
			for(size_t j = 0; j < n_rays; j++)
				{
//				SetCurrentRay(j);
				ComplexFunctionF32	CurrentRay;
				focused_data.GetRow(CurrentRay, {sub, j, slice_mask(0)});
				for(size_t i = 0; i < n_samples; i++)
					{
					sample_brightness.at(j,i) += cabs(CurrentRay[i]);
					}
				}
			}

		LogCompress(sample_brightness);
		CutHistogramEdges(sample_brightness, range1_F64(0.001, 0.999));
		NormalizeImage(sample_brightness, 0,255);
		DisplayMathFunction2D(sample_brightness, "sample brightness", sco);
	}

	DisplayMathFunction2D(correlation_map, "Correlation");

	GrayScanConverter	correlation_displayer;
	correlation_displayer.MakeCopy(correlation_map);
	correlation_displayer.SetFlip(true);
	ExportScanConverterOptions(correlation_displayer);
	CutHistogramEdges(correlation_displayer, range1_F64(0.1, 1));
	NormalizeImage(correlation_displayer, 0, 255);

//	correlation_displayer.SetImageTitle("correlation.pct");

	DisplayMathFunction2D(correlation_displayer, "correlation", sco);

	LogCompress(range_brightness);
	CutHistogramEdges(range_brightness, range1_F64(0.001, 0.999));
	NormalizeImage(range_brightness, 0, 255);


	DisplayMathFunction2D(range_brightness, "range brightness", sco);
}


void	EtalonCorrelator::PhaseAnalyze()
{
	size_t factor = 8;
	size_t ft_size = ceil_fft_length(n_samples);
	size_t ft_size_interp = ft_size * factor;
	size_t n_samples_interp = n_samples*factor;
//	size_t ray = n_rays/2;
//	size_t sub = n_frames/2;
	ComplexFunctionF32	ft_buffer(ft_size), ft_buffer_interp(ft_size_interp);
	RealFunctionF32	phase_deriv(n_samples_interp);
	RealFunctionF32	phase_function_interp(n_samples_interp);
	RealFunctionF32	phase_function(n_samples);
	RealFunctionF32	phase_function_smooth(n_samples);

//#define m1 for(size_t i = 0; i < n_samples_interp; i++) phase_function_interp[i] = polar(1, phase_deriv[i]*factor);
//#define m1 phase_function_interp.fill(1);\
//	for(size_t i = 1; i < n_samples_interp; i++) phase_function_interp[i] = phase_function_interp[i-1] * polar(1, -phase_deriv[i]);

	GUIProgressBar	progress;
	progress.start("Processing phase", n_rays*n_frames);
	for(size_t sub = 0; sub < n_frames; sub++)
	{
//		SetCurrentFrame(sub);
		for(size_t ray = 0; ray < n_rays; ++ray)
		{
//			SetCurrentRay(ray);
			ComplexFunctionF32	CurrentRay;
			focused_data.GetRow(CurrentRay, {sub, ray, slice_mask(0)});
			ft_buffer.CopyData(CurrentRay);
			FFT(ft_buffer, ftForward);
			ft_buffer_interp.CopyData(ft_buffer);
			FFT(ft_buffer_interp, ftReverse);

			for(size_t i = 1; i < n_samples_interp; i++)
			{
				phase_deriv[i] = arg(ft_buffer_interp[i-1] % ft_buffer_interp[i]);
			}
			phase_deriv.FilterMedian(factor);
			for(size_t i = 0; i < factor; i++) phase_deriv[n_samples_interp-1-i] = phase_deriv[n_samples_interp - factor];

			phase_deriv.FilterGauss(factor/2, 0.3, extrapolation::by_last_value);

			for(size_t i = 0; i < n_samples_interp; i++) phase_function_interp[i] = (phase_deriv[i])*factor;


			//phase_function_interp.FilterGauss(factor/2, 0.3, extrapolation::by_last_value);
//			phase_function_interp.FilterGauss(8*factor, 0.3, extrapolation::by_last_value);
			for(size_t i = 0; i < n_samples; i++)
			{
				phase_function_smooth[i] = phase_function[i] = phase_function_interp[i*factor];
			}
			phase_function_smooth.FilterMedian(64);
			phase_function.CopyData(phase_function_smooth);
			phase_function.FilterGauss(16, 0.3, extrapolation::by_last_value);
			phase_function -= phase_function_smooth;
			ApplyFunction(phase_function, fabs_functor<float,float>());
			phase_function.FilterGauss(4, 0.3, extrapolation::by_last_value);

			if(CapsLock()) DisplayMathFunction(phase_function, 0, 1, "Phase deriv up");
			real(CurrentRay).CopyData(phase_function);
			imag(CurrentRay).fill(0);

			++progress;
		}
	}
	progress.end();
	//EndProgress();

	if(1)
	{
		for(size_t sub = 0; sub < n_frames; sub++)
		{
//			SetCurrentFrame(sub);
			for(size_t j = 0; j < n_rays; j++)
			{
//				SetCurrentRay(j);
				ComplexFunctionF32	CurrentRay;
				focused_data.GetRow(CurrentRay, {sub, j, slice_mask(0)});
				for(size_t i = 0; i < n_samples; i++)
				{
					sample_brightness.at(j,i) += real(CurrentRay[i]);
				}
			}
		}

//		sample_brightness.logCompress();	
		CutHistogramEdges(sample_brightness, range1_F64(0.001, 0.999));
		NormalizeImage(sample_brightness, 0, 255);
		DisplayMathFunction2D(sample_brightness, "FM picture", sco);
	}
}


namespace
{
void	DisplayEigenVectors(const ComplexFunction2D_F32 &ev, const char* title)
{
	ComplexFunction2D_F32	eigen_vectors(ev);
	for(size_t n = 0; n < ev.vsize(); ++n)
	{
		FFT(eigen_vectors.col(n), ftForward);
	}
	eigen_vectors.transpose();
	DisplayMathFunction2D(eigen_vectors, title);
}
}


void	EtalonCorrelator::PrepareEtalonSet(EtalonSet &etalon_set, const char *comment)
{
	char	text_buffer[256];
	sprintf(text_buffer, "How to obtain '%s'?", comment);
	bool	repeat = true;

	while(repeat)
	{
		size_t answer = GetButtonDecision(text_buffer, /*4,*/{"Load", "Edit", "Load&Edit", "Exit"});
		if(answer == 3) return;
		do
		{
			if(answer == 0 || answer == 2)
			{
				try
				{
					string	filename;
					GetFileNameRead(filename, "Etalons file");
					etalon_set.LoadEtalons(filename.c_str());
				}
				catch(canceled_operation &){
				}
			}
			if(answer == 1 || answer == 2 || !etalon_set.n_etalons) // если нет ни одного эталона, включаем принудительное их редактирование
				etalon_set.EditEtalons(*this);
		//	AnalyzeEtalons();
			answer = 1;
			repeat = false;
		} while(YesOrNo("Continue editing etalons?", false));


		if(YesOrNo("Optimize etalons?", false))
		{
			etalon_set.CalculateCorrelationMap(*this, correlation_map);
	//		PrepareResultingImages();
			etalon_set.OptimizeEtalons(*this, correlation_map);
			repeat = true;
		}
		if(YesOrNo("Save etalons?", false))
		{
			try
			{
				string filename;
				GetFileNameWrite(filename, "Save etalons file");
				etalon_set.SaveEtalons(filename.c_str());
				repeat = false;
			}
			catch(canceled_operation &){
			}
		}
	}
}

void	EtalonCorrelator::PrepareCorrelationBasis(size_t hw, size_t es, ComplexMatrixF32& eigen_vectors)
{
	EtalonSet	etalon_set;

	etalon_set.SetEtalonDimensions(es, hw);
	etalon_set.n_etalon_volumes = n_ranges;//пока совпадают
	etalon_set.etalon_volume_step = float(n_samples)/etalon_set.n_etalon_volumes; //NB: float, not int
	PrepareEtalonSet(etalon_set, "Etalon set");
	//CheckOrthogonality(eigen_vectors);
	etalon_set.ComputeBasis(eigen_vectors);

	ComplexFunctionF32	shell;
	for(size_t i = 0; i < etalon_length; ++i)
	{
		shell.UseData(eigen_vectors.col(i));
		ApplyWindowFunction(shell, blackman_nuttall_window());
	}
	DisplayEigenVectors(eigen_vectors, "Basis");
}

void	EtalonCorrelator::Batch()
{
//	PhaseAnalyze();	
	//while(1)
	{

		if(YesOrNo("Perform decomposition", true))
		{

			ComplexMatrixF32	eigen_vectors_1(etalon_length, etalon_length);
//			ComplexFunction2D_F32	eigen_vectors_2(etalon_length, etalon_length);


			if(1)
			{
				PrepareCorrelationBasis(etalon_halfwidth, etalon_length, eigen_vectors_1);
		//		PrepareCorrelationBasis(etalon_halfwidth, etalon_length, etalon_set_1, eigen_vectors_2);
			}

			DataArray<RealFunction2D_F32> sb1;

			PerformDecomposition(eigen_vectors_1, sb1);
			AnalyzeDecomposition(sb1);


			for(size_t n = 0; n < etalon_length; ++n)
			{
				ApplyFunction(sb1[n], fabs_functor<float,float>());
//				RemoveSpike(sb1[n], 3);
//				LogCompress(sb1[n]);
				CutHistogramEdges(sb1[n], range1_F64(0.01, 0.99));
			}


			for(size_t n = 0; n < etalon_length; ++n)
			{
				sample_brightness.CopyData(sb1[n]);

		//		sample_brightness.CutHistogramEdges(0.001, 0.001);
		//		sample_brightness.NormalizeImage(0,255);
				DisplayMathFunction2D(sample_brightness, ssprintf("decomposition # %d", n).c_str(), sco);
			}
		}
	}
}



XRAD_END
