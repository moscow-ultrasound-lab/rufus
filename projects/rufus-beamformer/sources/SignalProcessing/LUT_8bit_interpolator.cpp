#include "pre.h"
#include "LUT_8bit_interpolator.h"
#include <XRADBasic/DataArrayIO.h>

XRAD_BEGIN


//int	*LUT_8bit_interpolator :: LUT = NULL;
DataArray<int>	LUT_8bit_interpolator :: LUT;

size_t	LUT_8bit_interpolator :: filter_order = N_Taps_LUT;
ptrdiff_t	LUT_8bit_interpolator :: centre_offset = Centre_Offset;

size_t	LUT_8bit_interpolator :: nEntriesPerPnt = 256L * N_Taps_LUT;
size_t	LUT_8bit_interpolator :: interpolation_factor = LUT_Interpolation_Factor;
size_t	LUT_8bit_interpolator :: nEntriesPerFilter = 256L * LUT_Interpolation_Factor * N_Taps_LUT;
size_t	LUT_8bit_interpolator::nEntriesPerTap = LUT_Entries_Per_Tap;

bool	LUT_8bit_interpolator::ATL_LUT = false;


//--------------------------------


FILE	*LUT_8bit_interpolator :: Open_LUT_File()
	{
	string	complete_file_name;
	string	error_message;
	FILE	*lut_fule;
	
	complete_file_name = ssprintf("%s%s", "", "Analytic.LUT");
	lut_fule = fopen(complete_file_name.c_str(), "rb");
	
	if (lut_fule == NULL) // have a chance to try again
		{
		printf("\n\n\r\aCould not find LUT '%s'", "Analytic.LUT");
		fflush(stdout);
		error_message = ssprintf("Please Open '%s'", "Analytic.LUT");
		GetFileNameRead(complete_file_name,error_message);
// 			{
// 			throw runtime_error("Can't work without look up table.");
// 			}
		lut_fule = fopen(complete_file_name.c_str(),"rb");
		if (!lut_fule)
			{
			throw runtime_error("Can't open LUT file.");
			}
		}
	return	lut_fule;
	}

//--------------------------------


void	LUT_8bit_interpolator :: LoadLUT_ATL()
	{
//	DestroyPointer(LUT);

	FILE	*lut_fule = Open_LUT_File();

	LUT.realloc(Analytic_LUT_size);
	//LUT = CreatePointer(int, Analytic_LUT_size);
	//if (!LUT) throw runtime_error("Error allocating look-up table memory");

	size_t	readCount = fread_numbers(LUT, lut_fule, ioI32_BE);
//	int	readCount = fread(LUT, sizeof(int), Analytic_LUT_size, lut_fule);
	if (readCount != Analytic_LUT_size) throw runtime_error("Error reading look-up table file");
	fclose(lut_fule);

	filter_order = N_Taps_LUT;
	centre_offset = Centre_Offset;
	interpolation_factor = LUT_Interpolation_Factor;
	nEntriesPerPnt = 256L * filter_order;
	nEntriesPerFilter = 256L * interpolation_factor * filter_order;
	nEntriesPerTap = LUT_Entries_Per_Tap; // эта константа здесь на месте (соответствует 8-битной разрЯдности входных данных)
	
//	AnalyzeLUT();
	ATL_LUT = true;
	}

//--------------------------------


void	LUT_8bit_interpolator :: AnalyzeLUT()
//	здесь можно проделать анализ таблиц атл
	{
	ComplexFunction2D_F32	hilbertFilters(interpolation_factor, filter_order);
	
	for(size_t i = 0; i < interpolation_factor; i++)
		{
		for(size_t j = 0; j < filter_order; j++)
			{
			size_t	re = Data_Offset + i*nEntriesPerPnt + j*nEntriesPerTap + (Data_Offset-1);
			size_t	im = re + nEntriesPerFilter;
			
			//int k = Data_Offset-1;
			real(hilbertFilters.at(i,j)) = LUT[re];
			imag(hilbertFilters.at(i,j)) = LUT[im];
			}
		DisplayMathFunction(hilbertFilters.row(i), 0, 1, "ATL LUT");
		}
	}




void	LUT_8bit_interpolator :: InitLUT(int )
	{
//	UniversalInterpolator<FilterKernelReal> &im = interpolators::hilbert_im;
//	UniversalInterpolator<FilterKernelReal> &re = interpolators::hilbert_re;
	ATL_LUT = false;
#if 0
//	DataArray2D<DataArray<complexF32> >	*hilbertFilters = RealFunctionF32 :: InitHilbertInterpolator(fSize);
	// здесь брать interpolators::hilbert_re, hilbert_im
	
	interpolation_factor = hilbertFilters->vsize();
	filter_order = hilbertFilters->hsize();
	nEntriesPerPnt = 256L * filter_order;
	nEntriesPerFilter = 256L * interpolation_factor * filter_order;
	centre_offset = filter_order/2;
	
	int	LUT_Size = 2L * filter_order * interpolation_factor * 256L;
	
	//LUT = CreatePointer(int, LUT_Size);
	LUT.realloc(LUT_Size);
	
	for(int i = 0; i < interpolation_factor; i++)
		{
		for(int j = 0; j < filter_order; j++)
			{
			//int	*re = LUT + Data_Offset + i*nEntriesPerPnt + j*nEntriesPerTap;
			//int	*im = LUT + Data_Offset + nEntriesPerFilter + i*nEntriesPerPnt + j*nEntriesPerTap;
			int	re_start = Data_Offset + i*nEntriesPerPnt + j*nEntriesPerTap;
			int	im_start = re_start + nEntriesPerFilter;
			for(int k = -Data_Offset; k < Data_Offset; k++)
				{
				LUT[re_start+k] = k*(128.*256.*256.*real((*hilbertFilters).at(i,j)));
				LUT[im_start+k] = k*(128.*256.*256.*imag((*hilbertFilters).at(i,j)));
				//re[k] = k*(128.*256.*256.*real((*hilbertFilters).at(i,j)));
				//im[k] = k*(128.*256.*256.*imag((*hilbertFilters).at(i,j)));
				}
			}
		}
	
	delete hilbertFilters;
#endif
	}

complexF32 LUT_8bit_interpolator :: interpolate(double x) const
	{
	//2017_06_21 изменена фаза преобразования Гильберта. Ранее (по наследству от интерполятора АТЛ) она соответствовала "левой" части спектра, в результате чего частоты шли в обратном порядке
	if(!ATL_LUT)
	{
 		complexF32	result;
		result.re = in(x, &interpolators::hilbert_re);
		result.im = in(x, &interpolators::hilbert_im);
		return result;
	}
	else
	{
		ptrdiff_t	s = size();

		ptrdiff_t	x0 = floor(x);
		ptrdiff_t	dx = ptrdiff_t(double(x-x0)*interpolation_factor);

		if(!LUT.size()) throw runtime_error("LUT_8bit_interpolator :: operator() -- LUT not initialized");

		if((x0 >= s-centre_offset-1) || (x0 < centre_offset)) return complexF32(0, 0);


		ptrdiff_t table_r = Data_Offset + dx*nEntriesPerPnt;
		ptrdiff_t table_i = table_r + nEntriesPerFilter;

		int	re(0), im(0);
		const_iterator	it = begin() + (x0 - centre_offset);

		for(size_t i = 0; i < filter_order; ++i, ++it)
		{
// 			re += LUT[table_r + *it];
// 			im += LUT[table_i + *it];
			re += int(*it) * int(LUT[table_r + 127]);
			im += int(*it) * int(LUT[table_i + 127]);

			table_r += nEntriesPerTap;
			table_i += nEntriesPerTap;
		}
		return complexF32(re, -im);
	}
	}
	
XRAD_END
