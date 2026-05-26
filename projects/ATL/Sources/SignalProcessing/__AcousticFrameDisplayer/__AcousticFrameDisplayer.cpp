#include "pre.h"
#include "AcousticFrameDisplayer.h"
#include <XRADIniFile.h>
#include <ImageUtils.h>

#error obsolete file


#if 0
void	DumpLinearRaster(const RealFunction2D_F32 &img, char *data_filename)
	{
	int	n_samples = 512;
	int	n_rays = img.vsize();
	double	step = double(img.hsize())/n_samples;
	RealFunctionF32	buffer(n_samples);
	
	FILE	*data_file = fopen(data_filename, "wb");
	
	for(int i = 0; i < n_rays; i++)
		{
		buffer.fill(0);
		for(int j = 0; j < n_samples-1; j++)
			{
			int	k0 = int(j*step);
			int	st2 = step;
			for(int k = 0; k < st2; k++) buffer[j] += img[i][k0+k];
			buffer[j] /= st2;
			buffer[j] = range(buffer[j], 0, 255);
			}
		fwrite_numbers(buffer, data_file, ioUI8);
		}
	fclose(data_file);
	}
#endif


//def_qmbuff(char)

/*
“стройство двумерного массива:
Y строк по X элементов. ЉаждаЯ строка -Р объект класса RealFunctionF32;
Љонструкторы и операторы [] вызываютсЯ от (y, x) и [y][x]
*/


//AcousticFrameDisplayer :: AcousticFrameDisplayer(int nr, int ns) : GrayScanConverter(nr,ns)
AcousticFrameDisplayer :: AcousticFrameDisplayer(int nr, int ns) : RealFunction2D_F32(nr,ns)
	{
	}
	
AcousticFrameDisplayer :: ~AcousticFrameDisplayer()
	{
	}
	


// void	AcousticFrameDisplayer :: SetArguments(double yMin, double yMax, double xMin, double xMax)
// 	{
// 	y0 = yMin;
// 	deltaY = (yMax - yMin)/vsize();
// 	x0 = xMin;
// 	deltaX = (xMax - xMin)/hsize();
// 	//for(y = 0; y < vsize(); y ++) matrix[y].SetArgument(xMin, xMax);
// 	}
// 	
// void	AcousticFrameDisplayer :: SetUnits(string dataU, string yU, string xU)
// 	{
// 	dataUnits = dataU;
// 	unitsY = yU;
// 	unitsX = xU;
//	}


#if 0	
void	AcousticFrameDisplayer :: LogCompress(double dynRangeShadows, double dynRangeLights)
	{
	if(dynRangeShadows)
		{
		LogCompressRangeDB(*this,dynRangeShadows, dynRangeLights);
		return;
		}
	else
		{
		xrad::LogCompress(*this);
		}
	
	double	min_val = MinValue(*this);
	double	max_val = MaxValue(*this);
		
	*this -= max_val;
	min_val -= max_val;
	max_val = 0;
	
	SetUnits("dB");
	

//	int	histogram_size = sqrt(hsize()*vsize());
	int	histogram_size = 256;
//	RealFunctionF32	histogram = ComputeComponentsHistogram(*this, histogram_size);
	RealFunctionF32	histogram(histogram_size);
	ComputeHistogram(*this, histogram, range_1d(min_val, max_val));
	if(CapsLock()) DisplayMathFunction(histogram, min_val, -min_val/histogram_size, "Histogram", "dB", "Density");
	
//	double	d0 = -FindQuantile(*this, 0.001, histogram);// в тенЯх 0.1%
// 	double	d0 = -ComputeQuantile(min_val, max_val, 0.01, histogram);// в тенЯх 1%
// 	double	d1 = -ComputeQuantile(min_val, max_val, 0.999, histogram);// в светах 0.1%	
	
	range_1d	quantiles = ComputeQuantilesRange(histogram, range_1d(min_val, max_val), range_1d(0.01, 0.999));
	
// 	double	black = range(d1, 0, -min_val);
// 	double	white = range(d0, black, -min_val);

	if(CapsLock())
		{
		quantiles.x1 = GetDouble("Dynamic range minimum(dB)", quantiles.x1, 0, -min_val);
		quantiles.x2 = GetDouble("Dynamic range maximum(dB)", quantiles.x2, quantiles.x1, -min_val);
		}

	for(int y = 0; y < vsize(); y++)
		{
		for(int x = 0; x < hsize(); x++) at(y,x) = range(at(y,x), -quantiles.x2, -quantiles.x1) + quantiles.x1;
		}
	}
#endif

#if 0
void	AcousticFrameDisplayer :: Display(const char *Title)
	{
	short	answer = 1;
	int	vertNo = hsize()/2, horizNo = vsize()/2/*, index = 1*/;
	double	Average, /*Dispersion,*/ max_val, min_val;
	
	static bool	draw_grid = false;
	static bool	flip_image = false;

	
	GrayScanConverter	SC;
	xrad::MakeCopy(SC, *this);
	SC.CopyScanConverterOptions(*this);
//	SC.SetImageTitle(Title);
	
	NormalizeImage(SC, 0,255);
	
	if(0)DumpLinearRaster(SC, "BeforeCV.raw");
	
	SC.SetBackground(80);
	SC.SetGrid(draw_grid, 160, cm(2));
	SC.InitScanConverter(512);	
	SC.BuildRaster();

	do
		{
		answer = Get_Button_Decision(
			Title,
			5,
			"Cartesian coords",
			"Horizontal profile",
			"Vertical profile",
			"Show statistics",
			"Exit display"
			);
		switch(answer)
			{
			case 0 :
				//SC.GetRaster().DisplayRaster(Title);
				Get_Checkbox_Decision("Scan converter options", 2, 
					"Draw grid", &draw_grid,
					"Flip image", &flip_image);
				
				SC.SetGrid(draw_grid, 160, cm(2));
				SC.SetFlip(flip_image);
				DisplayMathFunction2D(SC.GetRaster(), "Cartesian image");
				break;
				
				
			case 1:
				do
					{
					horizNo = GetLong("Horizontal No:", horizNo + 1, 1, vsize()) - 1;
					DisplayMathFunction(row(horizNo), x0, deltaX, Title, unitsX, dataUnits);
					}while(Yes_Or_No("Display another horizontal?", YES));
				break;
			case 2:
				do
					{
					vertNo = GetLong("Vertical No:", vertNo + 1, 1, hsize()) - 1;
					DisplayMathFunction(col(vertNo), y0, deltaY, Title, unitsY, dataUnits);
					}while(Yes_Or_No("Display another vertical?", YES));
				break;
			case 3:	
				if(Decide2("Statistics:", "Show histogram","Show moments", 0))
					{
					max_val = MaxValue(*this);
					min_val = MinValue(*this);
					Average = AverageValue(*this);
					//Dispersion = StandardDeviation();
					Show_Double("Max value:", max_val);
					Show_Double("Min value:", min_val);
					Show_Double("Average value:", Average);
					//Show_Double("Standard deviation:", Dispersion);
					}
				else
					{
					int	s = sqrt(double(vsize()*hsize()));
					double	min = MinValue(*this);
					double	max = MaxValue(*this);
					RealFunctionF32	histogram(s);
					ComputeHistogram(*this, histogram, range_1d(min, max));
					DisplayMathFunction(histogram, min, (max-min)/s, "Histogram", dataUnits, "Amount");
					//ShowHistogram(sqrt(vsize()*hsize()));
					
					}
				break;
			case 4:	break;
			}
		}while(answer != 4);
	}
#endif//0