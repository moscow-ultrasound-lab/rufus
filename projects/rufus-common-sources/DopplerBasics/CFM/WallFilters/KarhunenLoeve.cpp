#include "pre.h"
//#include <DopplerBasics/CFM/WallFilters/KarhunenLoeve.h>
//#include <MatrixVectorGUI.h>
//#include <MatrixAlgorithms/EigenVectorTools.h>

XRAD_BEGIN

// нахождение корреляционной матрицы в первом приближении отлажено
void	ComputeCorrelationMatrix(ComplexMatrixF32 &R, ComplexFunctionMD_F32 &frame_by_shots, size_t sweep_start, size_t sweep_end)
{
	size_t	n_shots = frame_by_shots.sizes(1);
	size_t	n_samples = frame_by_shots.sizes(2);

	ComplexFunction2D_F32 slice1, slice2;
	ComplexFunctionF32	burst_ref;

	for(size_t shot1 = 0; shot1 < n_shots; ++shot1)
	{
		for(size_t shot2 = 0; shot2 < n_shots; ++shot2)
		{
			frame_by_shots.GetSlice(slice1, {slice_mask(0), shot1, slice_mask(1)});
			frame_by_shots.GetSlice(slice2, {slice_mask(0), shot2, slice_mask(1)});
			double	acc1(0), acc2(0);
			complexF64 acc(0);

			for(size_t beam = sweep_start; beam < sweep_end; ++beam)
			{
				ComplexFunctionF32::const_iterator it1 = slice1.row(beam).cbegin();
				ComplexFunctionF32::const_iterator it2 = slice2.row(beam).cbegin();

				//для экономии можно набирать не по всем лучам и отсчетам, а выборочно, например, через N
				for(size_t sample = 0; sample < n_samples; ++sample, ++it1, ++it2)
				{
					acc += (*it1 % *it2);
					acc1 += cabs2(*it1);
					acc2 += cabs2(*it2);
				}
			}
			// была ошибка по невнимательности, acc=slice1|slice2.
			// нельзя, т.к. обрабатывается только часть массива.
			// это относится в данном месте к любым попыткам заменить явный цикл
			// на использование какого-либо оператора.
			R.at(shot1,shot2) = acc/sqrt(acc1*acc2);
		}
	}
}

void	KLFilterOneShot(ComplexFunctionF32 &burst, const ComplexMatrixF64 &Vec, size_t n_components)
{
	//if(CapsLock())ShowVector("burst_before", burst);
	size_t n_shots(burst.size());
	for(size_t i = 0; i < n_components; ++i)
	{
		ComplexFunctionF64	kl_vec(Vec.col(n_shots - 1 - i));//TODO сделано неоптимально, нужно исправлять
		burst.subtract_multiply(kl_vec, sp(burst, Vec.col(n_shots-1-i)));
	}
	//if(CapsLock())ShowVector("burst_after", burst); 
}


XRAD_END
