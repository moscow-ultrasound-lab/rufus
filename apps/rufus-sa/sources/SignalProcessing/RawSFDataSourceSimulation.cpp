#include "pre.h"
#include <XRADBasic/DataArrayIO.h>
#include <XRADSystem/Sources/TextFile/text_file.h>

#include "RawSFDataSourceTransEcho.h"
#include "RawSFDataSourceSimulation.h"
#include "SyntheticApertureFocuserAina.h"

XRAD_BEGIN


void	RawSFDataSourceSimulation::Init()
{

	printf("\nSie haben die Simulation gestartet");
	size_t n_samps_per_TX_event = 1e3;
	samples_per_element = n_samps_per_TX_event;
	n_tx_elements = n_rx_elements = 64;
	//raw_signal_sample_rate = Hz(3e6);
	//raw_signal_sample_rate = Hz(5e6);
	probe_carrier = MHz(3);

	raw_signal_sample_rate = probe_carrier * 10;
	first_raw_sample = 0;
	last_raw_sample = first_raw_sample + samples_per_element;
	hilbert_samples = samples_per_element / 2;// входные данные действительные до пр. Гильберта, поэтому выходная частота отсчетов вдвое ниже
	hilbert_signal_sample_rate = raw_signal_sample_rate / 2;
	//recommended_n_rays = 128;
	//recommended_angle = degrees(45);
	probe_bandwidth = MHz(1);
	sound_speed = cm_sec(1.54e5);
	array_pitch = mm(18.27 / n_tx_elements);

	plane_wave_flag = true;

}

void RawSFDataSourceSimulation::LoadData()
{

	data.realloc({ n_tx_elements, n_rx_elements, samples_per_element });
	data.fill(complexF32(0));
	size_t	answer(0);
	answer = Decide("Simulate", { "Plane wave", "Spherical wave"}, SavedGUIValue(answer));
	switch (answer)
	{
		case 0:
		{
			double sigma(1e3);
			ComplexFunction2D_F32	slice;
			for (size_t i = 0; i < data.sizes(2); ++i)
			{
				//complexF64	factor = polar(1, 1);
				data.GetSlice(slice, { slice_mask(0), slice_mask(1), i });
				//slice *= factor;
				for (size_t j = 0; j < slice.vsize(); ++j)
				{
					for (size_t k = 0; k < slice.hsize(); ++k)
					{
						slice.at(j, k) = complexF32(RandomGaussian(0, sigma), RandomGaussian(0, sigma));
						slice.at(j, k) += polar(100 * sigma, i *  two_pi() * probe_carrier.MHz() / raw_signal_sample_rate.MHz());//i*0.1
					}
				}
			}
		}
		break;
		case 1:
		{
/*			int number_of_samples = data.sizes(2);
			int number_of_rays = 128;
			int number_of_transmit_elements = data.sizes(1);
			int number_of_receive_elements = data.sizes(0);
			auto kerf = array_pitch;
			int angle_range = 90;
			double dfi = angle_range / number_of_rays;
			//int first_sample = first_raw_sample;
		//	int	last_sample = last_raw_sample;
	//		int n_lines = number_of_samples;
			auto step_depth = 0.5*sound_speed / raw_signal_sample_rate;
			auto start_depth = 0.5*(int)first_raw_sample*sound_speed / raw_signal_sample_rate;


			
			ComplexFunction2D_F32	slice;
			GUIProgressBar	progress;
			std::mutex	m;
			progress.start("Daten generieren", number_of_transmit_elements);
			{
				for (int transmit_element = 0; transmit_element < number_of_transmit_elements; ++transmit_element)
				{
					auto	transmit_element_position = (transmit_element - number_of_transmit_elements / 2)*kerf.cm();
					for (int receive_element = 0; receive_element < number_of_receive_elements; ++receive_element)
					{
						auto	receive_element_position = double(receive_element - number_of_receive_elements / 2)*kerf.cm();
						data.GetSlice(slice, { slice_mask(0), (size_t)transmit_element, slice_mask(1) });
						for (int ray = 0; ray < number_of_rays; ++ray)
						{
							auto	fi = (ray - number_of_rays / 2)*dfi*pi()/180;
							for (int sample = 0; sample < number_of_samples; sample++)
							{
									auto	depth = (start_depth + sample*step_depth).cm();
									auto	transmit_path = depth - transmit_element_position*sin(fi) + square(transmit_element_position) / (2 * depth);
									auto	receive_path = depth - receive_element_position*sin(fi) + square(receive_element_position) / (2 * depth);
									auto	path = cm(receive_path + transmit_path);
									auto	t = path / sound_speed;

									size_t	sample_no = size_t(raw_signal_sample_rate * t);

									if(sample_no < number_of_samples)
									{
										slice.at(receive_element, sample_no - (int)first_raw_sample) += polar(1e5, double(sample*5));
									}
							}
						}
					}
					++progress;
				}
			}
			progress.end();
*/		}
		break;
	}
	

	//	if (CapsLock())
	{
	//	DisplayMathFunction3D(data, "simulated data");
	}
}

XRAD_END

