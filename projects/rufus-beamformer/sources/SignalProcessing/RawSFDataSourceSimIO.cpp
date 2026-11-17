#include "pre.h"
#include "RawSFDataSourceSimIO.h"
#include "SimIOHeaders/SimIOHeaders.h"
/*!
 * \file RawSFDataSource.cpp
 * \date 2017/06/22 9:55
 *
 * \author kulberg
 *
 * \brief 
 *
 * TODO: long description
 *
 * \note
*/

XRAD_BEGIN


void	RawSFDataSourceSimIO::Init()
{
	SimErrorMode(RETURN, stderr);
	filename = GetFileNameRead("Raw SF data file", SavedGUIValue(filename), "*.dat");
	
	XRAD_ASSERT_THROW_M(SimOpen(filename.c_str(), "rb", &file) == SUCCESS, runtime_error, ssprintf("Error opening input file  (%s)", filename.c_str()));

	printf("\nOpened input file: '%s'", filename.c_str());

//	Read in descriptors


	SimCheckSumCheckEnable(true);		// always check the checksum initially
	XRAD_ASSERT_THROW_M(SimSkipHistory(file) == SUCCESS, runtime_error, "This is not a SimIO style file");

//	read the standard "Generic_Data_Descriptor" record.   Only read some of the 
//	parameters to begin with to establish how many data-species/descriptor-version 
//	descriptor records there are.   Then read info regarding the number of keyword 
//	phrases (param structures) there are in each of these descriptor records.


	SimGetDescriptor(GENERIC_DATA_DESCRIPTOR_BASE_NUM_PARAMS,
					 SIMIO::Generic_Data_Descriptor,
					 file);
	size_t num_params_in_Generic_Data_Descriptor = GENERIC_DATA_DESCRIPTOR_BASE_NUM_PARAMS +
		2*SIMIO::Num_Xtra_Descriptors;
	SimGetDescriptor(int(num_params_in_Generic_Data_Descriptor),
					 SIMIO::Generic_Data_Descriptor,
					 file);

	XRAD_ASSERT_THROW_M(!strcmp(SIMIO::Data_Species, SF_DATA_SPECIES), invalid_argument, "Not raw synthetic focus data");// strcmp is negative logic
	XRAD_ASSERT_THROW_M(SimSkipData(file) == SUCCESS, runtime_error, "Problem skipping Generic_Data_Descriptor data");// skip to next descriptor

	SimGetDescriptor(SIMIO::Num_Xtra_Desc_Params[SIMIO::SF_Data_General_Descriptor_Num],
					 SIMIO::SF_Data_General_Descriptor,
					 file);
	XRAD_ASSERT_THROW_M(SimSkipData(file) == SUCCESS, runtime_error, "Problem skipping General_Descriptor data");

	SimGetDescriptor(SIMIO::Num_Xtra_Desc_Params[SIMIO::SF_Data_Frame_Descriptor_Num],
					 SIMIO::SF_Data_Frame_Descriptor,
					 file);
	XRAD_ASSERT_THROW_M(SimSkipData(file) == SUCCESS, runtime_error, "Problem skipping Frame_Descriptor data");

	SimGetDescriptor(SIMIO::Num_Xtra_Desc_Params[SIMIO::SF_Data_TX_Event_Descriptor_Num],
					 SIMIO::SF_Data_TX_Event_Descriptor,
					 file);

	size_t n_samps_per_TX_event = SIMIO::Data_Count;

	XRAD_ASSERT_THROW_M(n_samps_per_TX_event == size_t(SIMIO::Waveforms_Per_TX_Event*SIMIO::Samps_Per_Waveform), invalid_argument, "Header information is inconsistent");

//	size_t n_bytes_per_TX_event = n_samps_per_TX_event;

	XRAD_ASSERT_THROW_M((SIMIO::Acqs_Per_Cal != 0) || (SIMIO::TX_Events_Per_Frame == SIMIO::Waveforms_Per_TX_Event), invalid_argument, "Wrong # data records");
	XRAD_ASSERT_THROW_M(!strcmp(SIMIO::Data_Genesis, EXPT_GENESIS) || !strcmp(SIMIO::Data_Genesis, SIM_GENESIS), invalid_argument, ssprintf("Data is of unknown genesis (%s)", SIMIO::Data_Genesis));


	samples_per_element = SIMIO::Samps_Per_Waveform;	// these should always be equal

	n_tx_elements = n_rx_elements = SIMIO::Waveforms_Per_TX_Event;	// always assume a square array ie:
						// as many TX's as RX's
	raw_signal_sample_rate = Hz(SIMIO::Sample_Rate);

	first_raw_sample = SIMIO::Start_Sample;	// offset from origin
	last_raw_sample = (SIMIO::Start_Sample + SIMIO::Samps_Per_Waveform);

	hilbert_samples = samples_per_element/2;// входные данные действительные до пр. Гильберта, поэтому выходная частота отсчетов вдвое ниже
	hilbert_signal_sample_rate = raw_signal_sample_rate/2;
	recommended_n_rays = 128;
	recommended_angle = degrees(45);

	probe_carrier = MHz(3);
	probe_bandwidth = MHz(1);

	sound_speed = cm_sec(1.54e5);

	array_pitch = mm(SIMIO::Element_Pitch);


//	LUT_8bit_interpolator::LoadLUT_ATL();
//	if(Decide2("LUT selection", "ATL LUT", "Our LUT", 1))
	{
		LUT_8bit_interpolator::InitLUT(0);
	}

}

void RawSFDataSourceSimIO::DeadElements()
{
	dead_elements_list.realloc(2);
	dead_elements_list[0] = 11;
	dead_elements_list[1] = 27;
}

void RawSFDataSourceSimIO::LoadData()
{
	XRAD_ASSERT_THROW_M(file!=NULL, invalid_argument, "RawSFDataSourceSimIO::LoadData(), NULL file");
	SimErrorMode(RETURN, stderr);	// enable automatic error handling


	// возможно было бы сделать автоопределение?


	SimSkipHistory(file);	// rewind to 1st descriptor record
	SimSkipData(file);		// skip Generic_Data_Descriptor record
	SimSkipData(file);		// skip General_Descriptor record
	SimSkipData(file);		// skip Frame_Descriptor record
	SimCheckSumCheckEnable(false);		// turn off since takes too long !

	data.realloc({n_tx_elements, n_rx_elements, samples_per_element});
	DataArray<char>	buffer(n_rx_elements*samples_per_element);

//	SetActiveElements(0, 0);
	if(SIMIO::Acqs_Per_Cal != 0) // need to skip calibration pulses
	{
		for(size_t total_record=0, record_to_load=0; record_to_load < n_tx_elements; ++total_record)
		{
			SimGetDescriptor(SIMIO::Num_Xtra_Desc_Params[SIMIO::SF_Data_TX_Event_Descriptor_Num],
							 SIMIO::SF_Data_TX_Event_Descriptor,
							 file);

			if(total_record % (SIMIO::Acqs_Per_Cal + 1))
			{
				//чтение основных данных
				SimGetData(n_rx_elements*samples_per_element,
						   buffer.data(), // читаем через голову структуры прямо в массив по адресу
						   file);
				if(record_to_load < n_tx_elements)
				{
					DataArray<char>::iterator b = buffer.begin();
					for(size_t k = 0; k < n_rx_elements; ++k, b += samples_per_element)
					{
// 						SetActiveElements(record_to_load, k);
// 						copy(b, b+samples_per_element, current.begin());
//						copy(b, b+samples_per_element, dynamic_cast<row_t &>(*row).data.begin());
						auto	row = GetRow(record_to_load, k);
						dynamic_cast<row_t &>(*row).copy(b, b+samples_per_element);
					}
				}
				++record_to_load;
			}
			else
			{
				// пропуск калибровочных. по виду данных можно предположить, что они излучаются центральным элементом
				SimSkipData(file);// skip this record
			}
		}
	}
	else
	{
		for(size_t i=0; i < n_tx_elements; i++)
		{
			SimGetDescriptor(SIMIO::Num_Xtra_Desc_Params[SIMIO::SF_Data_TX_Event_Descriptor_Num],
							 SIMIO::SF_Data_TX_Event_Descriptor,
							 file);
			SimGetData(n_rx_elements*samples_per_element,
					   buffer.data(), // читаем через голову структуры прямо в массив по адресу
					   file);
			DataArray<char>::iterator b = buffer.begin();
			for(size_t k = 0; k < n_rx_elements; ++k, b += samples_per_element)
			{
// 				SetActiveElements(i, k);
// 				copy(b, b+samples_per_element, current.begin());
				auto	row = GetRow(i, k);
				dynamic_cast<row_t &>(*row).copy(b, b+samples_per_element);
			}
		}
	}

	DeadElements();
}


/*
void RawSFDataSourceSimIO::LoadOtherSource(RawSFDataSource	&original)
{

data.realloc({ original.n_tx_elements, original.n_rx_elements, original.samples_per_element });
	for (size_t i = 0; i < original.n_tx_elements; ++i)
	{
		for (size_t j = 0; j < original.n_rx_elements; ++j)
		{
			row_type row;
			data.GetRow(row, { i, j, slice_mask(0) });
			auto other_row = original.GetRow(i, j);
			for (size_t sample = 0; sample < original.samples_per_element; ++sample)
			{
				row[sample] = other_row->in(sample);
			}
		}
	}

	DeadElements();
}
*/


RawSFDataSourceSimIO::~RawSFDataSourceSimIO()
{
	if(file) SimClose(file);
}

string	RawSFDataSourceSimIO::GetFileName() const
{
	return filename;
}


//-----------------------------------------------------------
//
//	функции "обхода" битых элементов. самый примитивный вариант
//	под конкретную ситуацию в имеющихсЯ данных (испорчены 11 и 27 элементы):
//	не рассматриваютсЯ случаи "два битых подрЯд" и "битый на краю решетки"
//
//TODO	Предусмотреть автоматический поиск по провалам мощности
// 

bool RawSFDataSourceSimIO::IsDeadElement(size_t el_no)
{
	auto	it = dead_elements_list.begin();
	for(size_t i = 0; i < dead_elements_list.size(); ++i, ++it)
	{
		if(el_no == *it) return true;
	}
	return false;
}

size_t RawSFDataSourceSimIO::NextNonDeadElement(size_t el_no)
{
	if(!IsDeadElement(el_no))
	{
		return el_no;
	}
	return el_no + 1;
}

size_t RawSFDataSourceSimIO::PreviousNonDeadElement(size_t el_no)
{
	if(!IsDeadElement(el_no))
	{
		return el_no;
	}
	return el_no - 1;
}


// void xrad::RawSFDataSourceSimIO::SetActiveElements(size_t /*tx_el*/, size_t /*rx_el*/)
// {
// //	data.GetRow(current, {tx_el, rx_el, slice_mask(0)});
// }


XRAD_END

