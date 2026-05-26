#include "pre.h"
#include "BeamformerTesting.h"
#include <XRADBasic/DataArrayIO.h>

XRAD_BEGIN

//----------------------------------------------------------------------
//
//	генерируютсЯ данные длЯ проверки цифрового бимформера, разрабатываемого в изомеде.
//	ни длЯ чего другого эта функциЯ не нужна.
//	функциЯ масштабирует данные атл, чтобы получить частоту на выходе 40 мгц и разрЯдность
//	как в бимформере.
//
//----------------------------------------------------------------------


void	generate_beamformer_test_data(SyntheticApertureFocuser &fx)
	{
	int	sample;	
		
	const	int	adc_width = GetSigned("Emulated ADT width", 12, 8, 16);
	const	int	sample_width = 16;
	const	float	max_adc = pow(2., double(adc_width-1));


//	fx.SetDepthUnits(MILLIMETRES);
//	fx.SetFrequencyUnits(M_HERZ);

	float	new_carrier = 40;//MHz
	float	old_carrier = 12;//MHz
	float	carrier_ratio = new_carrier/old_carrier;
	
	int	n_samples_original = fx.n_raw_samples;
	int	n_samples_skip_original = fx.first_raw_sample;
	int	n_samples_new = int ((float)n_samples_original*carrier_ratio);
	int	n_samples_skip = int ((float)n_samples_skip_original*carrier_ratio);
	int	n_elements = fx.n_elements;
	
//	Show_Double("Carrier ratio", carrier_ratio);
//	Show_Long("New samples number", n_samples_new);
//	Show_Long("Start sample new", n_samples_skip);
	
//	Show_Double("Array Pitch (mm)", fx.array_Pitch);
	
	ComplexFunction2D_F32	result(n_elements, n_samples_new);
	
	int	el1 = 24;
	bool	dc_offset_correction = YesOrNo("Perform DC offset correction?", false);
	bool	unsigned_data = YesOrNo("Make data unsigned?", true);
	int	data_endian = Decide2("Data endian", "Low", "Big", 1);
	int	data_complexity = Decide2("Data complexity", "Real", "Complex", 0);
	
	fx.SetTXElement(el1);
	
	StartProgress("Calculating beamformer test data", n_elements);
	for(int sub = 0; sub < n_elements; sub ++)
		{
		fx.SetRXElement(sub);			
		for (sample = 0; sample < n_samples_new; sample ++)
			{
			for (int el2 = 0; el2 < fx.n_tx_elements; el2 ++){				
				float	interpRawSample = float(sample)/carrier_ratio;
				result[sub][sample] += fx.raw_waveform.in(interpRawSample);					
				}
			}
		if(dc_offset_correction)
			{
			ComplexFunctionF32	f(result[sub]);
			f.FilterGauss(3*carrier_ratio);
			result[sub] -= f;
			}
		NextProgress();
		}
	EndProgress();
	
	
	double	multiplier = max_adc/cabs(MaxValue(result));
	result *= multiplier;
	if(unsigned_data) result += complexF32(max_adc, max_adc);
	DisplayMathFunction2D(result, "Beamformer test data");

	string	filename;
	
	try
		{
		GetFileNameWrite(filename, "Result file name");
		}
	catch(canceled_operation &)
		{
		return;
		}
	
	char	data_filename[256];
	char	setup_filename[256];
	sprintf(data_filename, "%s.raw", filename.c_str());
	sprintf(setup_filename, "%s.txt", filename.c_str());
	
	FILE	*data_file = fopen(data_filename, "wb");
	FILE	*setup_file = fopen(setup_filename, "w");
	
	if(data_complexity) for(int sub = 0; sub < n_elements; sub ++)
		{
		if(data_endian)
			fwrite_numbers(result[sub], data_file, ioComplexI16_BE);
		else
			fwrite_numbers(result[sub], data_file, ioComplexI16_LE);
		}
	else{
		RealFunctionF32	f;
		f.UseData((float *)(&result.at(0,0)), n_samples_new*n_elements, 2);
		
		if(data_endian) fwrite_numbers(f, data_file, ioI16_BE);
		else	fwrite_numbers(f, data_file, ioI16_LE);
		}
		
	fprintf(setup_file, "Data file name = \"%s\"\n", data_filename);
	fprintf(setup_file, "ADC width = %d bits\n", adc_width);
	fprintf(setup_file, "Sample width = %d bits\n", sample_width);
	fprintf(setup_file, "Data endian = %d\n", data_endian);
	fprintf(setup_file, "Data complexity = %d\n", data_complexity);

	fprintf(setup_file, "Recorded sample width = %d bits\n", sample_width);
	
	fprintf(setup_file, "Unsigned data = %d\n", unsigned_data);

	fprintf(setup_file, "Elements number = %d\n", n_elements);
	fprintf(setup_file, "Element pitch = %g cm\n", fx.array_Pitch.cm()/10);
	fprintf(setup_file, "Sample rate = %g MHz\n", new_carrier);
	fprintf(setup_file, "Skip samples = %d\n", n_samples_skip);
	fprintf(setup_file, "Samples number = %d\n", n_samples_new);
	
	
	fclose(data_file);
	fclose(setup_file);
	}


XRAD_END
