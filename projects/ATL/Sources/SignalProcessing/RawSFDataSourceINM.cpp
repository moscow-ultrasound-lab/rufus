#include "pre.h"
#include <XRADBasic/DataArrayIO.h>
#include <XRADSystem/Sources/TextFile/text_file.h>

#include "RawSFDataSourceINM.h"

/*!
 * \file RawSFDataSourceINM.cpp
 * \date 2017/06/23 10:57
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

//#error продолжаем адаптировать файл к данным от Васюкова

size_t	SamplesPerTXEvent(wstring filename)
{
	text_file_reader	file(filename);
	DataArray<char>	buffer(file.size());
//	file.read_numbers(buffer, ioI8);
	float	fbuffer;
	size_t	count(0);
	size_t	count1(0);
	while(true)
	{
		count1 = file.scanf_("%f,", &fbuffer);
		if(count1!=1)
		{
			return count;
		}
		++count;
	}
}

void	RawSFDataSourceINM::Init()
{
	foldername = GetFolderNameRead(L"TransEcho folder");
	vector<wstring> subfolders;
	GetDirectoryContent(filenames, subfolders, foldername);

//	file.open(foldername, "rb");

	n_rx_elements = filenames.size();
	n_tx_elements = filenames.size();
//	samples_per_element = 1536;
	samples_per_element = SamplesPerTXEvent(foldername + L"/" + filenames[0])/n_tx_elements;
	raw_signal_sample_rate = MHz(12);
	//	first_raw_sample = SIMIO::Start_Sample;	// offset from origin
	//	last_raw_sample = (SIMIO::Start_Sample + SIMIO::Samps_Per_Waveform);

	first_raw_sample = 0;//180;
	last_raw_sample = samples_per_element + first_raw_sample;

	// преобразование Гильберта уже выполнено
	hilbert_samples = samples_per_element/2;
	hilbert_signal_sample_rate = raw_signal_sample_rate/2;


	probe_carrier = MHz(3);
	probe_bandwidth = MHz(1);//?

	sound_speed = cm_sec(1.54e5);// по умолчанию
	array_pitch = mm(0.28);

	recommended_angle = degrees(30);//45
	recommended_n_rays = 128;// проверялось на достаточность по поперечному спектру

	files.resize(n_tx_elements);
	for(size_t i = 0; i < n_tx_elements; ++i) files[i].open(foldername + L"/" + filenames[i]);
}


void RawSFDataSourceINM::LoadData()
{
	data.realloc({n_tx_elements, n_rx_elements, samples_per_element});
	
	for(size_t i = 0; i < n_tx_elements; ++i)
	{
		files[i].seek(0, SEEK_SET);
		for(size_t j = 0; j < n_tx_elements; ++j)
		{
			row_type row;
			data.GetRow(row, {i, j, slice_mask(0)});

			for(auto &it: row)
			{
				float	fbuffer;
				files[i].scanf_("%f,", &fbuffer);
				it = range(fbuffer-128, -128, 127);
			}

		}
	}
	DisplayMathFunction3D(RealFunctionMD_F32(data), "Just loaded");
}



string	RawSFDataSourceINM::GetFileName() const
{
	return convert_to_string(foldername);
}


//-----------------------------------------------------------
//
//	анализ битых элементов. сейчас вводятся вручную
//
//TODO	Предусмотреть автоматический поиск по провалам мощности
// 

// void xrad::RawSFDataSourceINM::SetActiveElements(size_t /*tx_el*/, size_t /*rx_el*/)
// {
// //	data.GetRow(current, {tx_el, rx_el, slice_mask(0)});
// }








XRAD_END

