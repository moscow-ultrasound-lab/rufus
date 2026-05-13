#ifndef __colour_analyzer_utils_h
#define __colour_analyzer_utils_h


XRAD_BEGIN

// функционалы

double	average_non_coherent(const ComplexFunctionF32 &f);
double	average_square_non_coherent(const ComplexFunctionF32 &f);
double	deviation_non_coherent(const ComplexFunctionF32 &f);
double	relative_deviation_non_coherent(const ComplexFunctionF32 &f);



void	ComputeLocalSpectrum(ComplexFunction2D_F32 &ft_buffer/*, RealFunctionF32 &ft_accumulator, RealFunctionF32 &ft_accumulator_filtered*/);
void	ComputeAverageSpectrum(const ComplexFunction2D_F32 &ft_buffer, RealFunctionF32 &ft_accumulator, RealFunctionF32 &ft_accumulator_filtered);

void	CutTreshold(RealFunctionF32 &f, double treshold);
void	RangeData(RealFunction2D_F32 &m, double mn, double mx);
double	CountMaxima1(const ComplexFunctionF32 &f);
double	CountMaxima(const ComplexFunction2D_F32 &f);
void	DisplayIntermediateResults(const RealFunctionF32 &ft_accumulator, const RealFunctionF32 &ft_accumulator_filtered, int n_rays_average);

void	FindMaximaPosition(const ComplexFunction2D_F32 &ft_buffer, float &m1, float &m2);





XRAD_END

#endif //__colour_analyzer_utils_h
