
XRAD_BEGIN

template<class ARR3D>
void	RawSFDataSource::DisplayUtility(const ARR3D &data) const
{
	{
		size_t	answer = 0;
//		size_t	sample = samples_per_element/2;
		size_t	TX_el = data.sizes(0)/2;
		ComplexFunction2D_F32	display_slice;
		typename ARR3D::slice_type_invariable	raw_slice;

		physical_length	aperture_width = array_pitch*n_tx_elements;
		physical_length scan_depth = 0.5*(samples_per_element * sound_speed / raw_signal_sample_rate);

		do
		{
			answer = GetButtonDecision("Raw SF data display",
			{"3D view", "Row signal", "Aperture signal","Exit"});
			try
			{
				switch(answer)
				{
					case 0:
					{
						ComplexFunctionMD_F32 display_elements;
						display_elements.MakeCopy(data);


						DisplayMathFunction3D(display_elements, "Raw signals", ScanFrameRectangle(aperture_width, scan_depth));
					}
					break;
					case 1:
					{
						TX_el = GetSigned("TX element no", TX_el, 0, n_tx_elements - 1);

						data.GetSlice(raw_slice, {TX_el, slice_mask(0), slice_mask(1)});
						display_slice.MakeCopy(raw_slice);
						DisplayMathFunction2D(display_slice, ssprintf("TX(%d)/RX element signal", TX_el).c_str(), ScanFrameRectangle(aperture_width, scan_depth));
						// размеры условные, потом уточнить
					}
					break;

					case 2:
					{
						typename ARR3D::slice_type_invariable	slice;
						RealFunction2D_F64	buffer(n_tx_elements, n_rx_elements, 0);

// 						sample = GetSigned("sample slice no", sample, 0, samples_per_element - 1);

						Functors::absolute_value	avf;

						for(size_t s = 0; s < samples_per_element; ++s)
						{
							data.GetSlice(slice, {slice_mask(0), slice_mask(1), s});
							for(auto i = 0; i < n_tx_elements; ++i)
							{
								for(auto j = 0; j < n_rx_elements; ++j)
								{
									buffer.at(i,j) += avf(slice.at(i,j));
								}
							}
						}

						DisplayMathFunction2D(buffer, "Average magnitude per elements", ScanFrameRectangle(aperture_width, aperture_width));
					}
					break;
				}
			}
			catch(canceled_operation &)
			{
			}
		} while(answer != 3);
	}
}



XRAD_END

