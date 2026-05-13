#include "Pre.h"

#include "EntropyAnalyzer.h"
//#include "NonUniformMotionAnalysis.h"
//#include <SharpnessCriteria.h>

XRAD_BEGIN


EntropyAnalyzer :: EntropyAnalyzer()
	{
	}

	
void	EntropyAnalyzer :: InitWork()
	{
	Display("Original frames");
	histogram_size = 128;

	detector.realloc({size_t(n_frames), size_t(n_rays), size_t(n_samples)});
	FramesCombineParams 	detection_param;
	
//	detection_param.log_compress = true;
	detection_param.alg = single_subaperture;
//	detection_param.auto_dynamic_range = false;
	
	GUIProgressBar	progress;
	progress.start("Building detector", n_frames);
	for(size_t sub = 0; sub < n_frames; ++sub)
		{
		ComplexFunction2D_F32 CurrentFrame;
		focused_data.GetSlice(CurrentFrame, {sub, slice_mask(0), slice_mask(1)});
//		SetCurrentFrame(sub);
		RealFunction2D_F32	slice;
		index_vector	iv ={sub, slice_mask(0), slice_mask(1)};
		
		detector.GetSlice(slice, iv);
		CopyData(slice, CurrentFrame, Functors::assign_f1(cabs_functor<float, complexF32>()));
//		for(size_t i = 0; i < n_rays; ++i) slice[i].FilterGauss(5);

		CopyData(CurrentFrame, slice);

		LogCompress(slice);
		
		
		++progress;
		}
	progress.end();

	max_value = MaxValue(detector);
	min_value = MinValue(detector);
	}


void	EntropyAnalyzer :: Batch()
	{
	Analyze1DHistograms();
	}


class	power_functor
	{
	const double	p;
	public:
	power_functor(double in_p) : p(in_p){};
	template<class T1, class T2>
		T1 &operator()(T1 &x, const T2 &y) const {return x=pow(y,p);}
	};

#if 0
sharpness_criterion	GetSharpnessCriterion()
	{
	return	shc_ls_maximum;
//	return	shc_entropy;
//#error
	}
#endif

void	EntropyAnalyzer :: Analyze1DHistograms()
	{
	RealFunction2D_F32	histograms(n_frames, histogram_size);
	RealFunctionF32	entropies(n_frames);

	size_t n_bars_r = 1;
	size_t n_bars_s = 5;

	size_t bar_width_r  = 2*n_rays/(n_bars_r+1);
	size_t bar_width_s = 2*n_samples/(n_bars_s+1);
	
	size_t bar_step_r = bar_width_r/2;
	size_t bar_step_s = bar_width_s/2;
	
	size_t n_bars = n_bars_r*n_bars_s;
//	sharpness_criterion	criterion = GetSharpnessCriterion();
	
	RealFunction2D_F32	entropies2(n_frames, n_bars);
	
	GUIProgressBar	progress;
	progress.start("Computing 1D histograms", n_frames);
	
	for(size_t sub = 0; sub < n_frames; ++sub)
		{
		RealFunction2D_F32	slice;
		detector.GetSlice(slice, {sub, slice_mask(0), slice_mask(1)});
		
//		entropies[sub] = EstimageImageSharpness(slice, histograms[sub], min_value, max_value, criterion);
		
		for(size_t b = 0; b < n_bars; ++b)
			{
			size_t s0 = (b / n_bars_r) * bar_step_s;
			size_t s1 = s0 + bar_width_s;
			
			size_t r0 = (b % n_bars_r) * bar_step_r;
			size_t r1 = r0 + bar_width_r;

			RealFunction2D_F32 bar;
			bar.UseDataFragment(slice, r0, s0, r1, s1);
			
			RealFunctionF32	temp_histogram(histogram_size);
//			entropies2[sub][b] = ComputeImageEntropy(bar, min_value, max_value);
//			entropies2[sub][b] = EstimageImageSharpness(bar, temp_histogram, min_value, max_value, criterion);
			}
		
		++progress;
		}

	progress.end();
	
//#error continue from here 
	DisplayMathFunction(entropies, 0,1,"Entropies");
	if(0)
		{
		RealVectorF64	coefficients(3);
		RealFunctionF64	approx(entropies.size(), 0);
		
		DetectLSPolynomUniformGrid(entropies, coefficients);
		for(size_t i = 0; i < approx.size(); ++i)
			{
			for(size_t j = 0; j < 3; ++j)
				{
				approx[i] += coefficients[j]*pow(double(i),double(j));
				}
			}
		DisplayMathFunction(approx, 0,1,"approx");
		GraphSet	compare("Compare", "", "");
		
		compare.AddGraphUniform(entropies, 1.4, 0.2/approx.size(), "Exact");
		compare.AddGraphUniform(approx, 1.4, 0.2/approx.size(), "Approximate");
		
		compare.Display();
		}

	DisplayMathFunction2D(histograms, "Histograms");
	entropies2.transpose();
	
	for(size_t b = 0; b < n_bars; ++b)
		{
		entropies2.row(b) -= MaxValue(entropies2.row(b));
		entropies2.row(b) /= -MinValue(entropies2.row(b));
		}
	DisplayMathFunction2D(entropies2,"Entropies in fragments");
	}
	
	
	
	
	
	
	
	
void	EntropyAnalyzer :: Analyze2DHistograms()
	{}
	

	



	
XRAD_END
