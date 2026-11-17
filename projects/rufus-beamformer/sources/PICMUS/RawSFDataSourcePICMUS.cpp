#include "pre.h"
#include "RawSFDataSourcePICMUS.h"
#include <hdf5.h>
#include <cstdio>

XRAD_BEGIN

static bool PICMUS_DatasetExists(hid_t file_id, const char* path)
{
	hid_t ds = H5Dopen2(file_id, path, H5P_DEFAULT);
	if (ds < 0)
		return false;
	H5Dclose(ds);
	return true;
}

static double PICMUS_ReadScalar(hid_t file_id, const char* path)
{
	hid_t ds = H5Dopen2(file_id, path, H5P_DEFAULT);
	if (ds < 0)
		throw runtime_error(string("PICMUS: cannot open dataset ") + path);
	double val = 0;
	herr_t status = H5Dread(ds, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &val);
	H5Dclose(ds);
	if (status < 0)
		throw runtime_error(string("PICMUS: cannot read dataset ") + path);
	return val;
}

static double PICMUS_ReadScalarAny(hid_t file_id, const std::vector<std::string>& candidates, const char* human_name)
{
	for (auto& path : candidates)
	{
		if (PICMUS_DatasetExists(file_id, path.c_str()))
			return PICMUS_ReadScalar(file_id, path.c_str());
	}
	throw runtime_error(string("PICMUS: none of the candidate paths found for ") + human_name);
}

void RawSFDataSourcePICMUS::Init()
{
	//use_plane_wave_mode = true;
	// Диалог выбора файла с фильтром PICMUS (*.uff — это HDF5-контейнер)
	filename = GetFileNameRead("PICMUS HDF5 file",
		SavedGUIValue("*.uff"),
		"*.uff");

	LoadHDF5Metadata(filename);

	// --- Настройка параметров, которые ждёт остальной RUFUS ---
	raw_signal_sample_rate = Hz(picmus_sampling_freq);
	hilbert_signal_sample_rate = raw_signal_sample_rate;
	hilbert_samples = samples_per_element;

	plane_wave_flag = true;
	recommended_angle = degrees(0);
	recommended_n_rays = n_rx_elements;

	probe_carrier = Hz(picmus_sampling_freq / 8);   // грубая оценка несущей; при желании уточнить
	probe_bandwidth = probe_carrier / 2;
	sound_speed = m_sec(picmus_sound_speed);

	array_pitch = mm(array_pitch_mm);

	first_raw_sample = 0;
	last_raw_sample = samples_per_element;

	physical_length scan_width = array_pitch * n_rx_elements;
	physical_length scan_depth = (samples_per_element * sound_speed / raw_signal_sample_rate) / 2;

	printf("\n=== PICMUS Data Loaded ===");
	printf("\n  File: %s", filename.c_str());
	printf("\n  Angles (plane waves): %d", n_angles);
	printf("\n  Transmits (=angles): %zu", n_tx_elements);
	printf("\n  Receive elements: %zu", n_rx_elements);
	printf("\n  Samples per element: %zu", samples_per_element);
	printf("\n  Sample rate: %.2f MHz", raw_signal_sample_rate.MHz());
	printf("\n  Sound speed: %.0f m/s", sound_speed.m_sec());
	printf("\n  Array pitch: %.3f mm", array_pitch.mm());
	printf("\n  Scan: width = %.2f mm, depth = %.2f mm", scan_width.mm(), scan_depth.mm());
	if (!angles_rad.empty())
	{
		printf("\n  Angle range: %.2f to %.2f degrees",
			angles_rad.front() * 180.0 / 3.14159265358979323846,
			angles_rad.back() * 180.0 / 3.14159265358979323846);
	}
	fflush(stdout);
}

void RawSFDataSourcePICMUS::LoadHDF5Metadata(const std::string& h5_filename)
{
	hid_t file_id = -1, dataset_id = -1;

	try {
		file_id = H5Fopen(h5_filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
		if (file_id < 0)
			throw runtime_error("Cannot open HDF5 file");

		// --- размеры массива данных ---
		dataset_id = H5Dopen2(file_id, "/channel_data/data", H5P_DEFAULT);
		if (dataset_id < 0)
			throw runtime_error("Cannot open /channel_data/data");

		hid_t space_id = H5Dget_space(dataset_id);
		int ndims = H5Sget_simple_extent_ndims(space_id);
		std::vector<hsize_t> dims(ndims);
		H5Sget_simple_extent_dims(space_id, dims.data(), NULL);

		printf("\n\nPICMUS dataset dimensions: ");
		for (int i = 0; i < ndims; ++i) {
			printf("[%llu]", (unsigned long long)dims[i]);
			if (i < ndims - 1) printf(" x ");
		}

		// PICMUS: [angles x elements x samples]
		if (ndims >= 3) {
			n_angles = (int)dims[0];
			n_rx_elements = dims[1];
			samples_per_element = dims[2];
		}
		else {
			throw runtime_error("PICMUS: unexpected data rank (expected 3D)");
		}

		// Каждая плоская волна (угол) -- отдельная "передача" для остального RUFUS.
		n_tx_elements = (size_t)n_angles;
		n_frames = 1;

		H5Sclose(space_id);
		H5Dclose(dataset_id);
		dataset_id = -1;

		// --- скалярные параметры: пробуем несколько известных вариантов путей ---
		picmus_sampling_freq = (float)PICMUS_ReadScalarAny(file_id,
			{
				"/channel_data/sampling_frequency",
				"/channel_data/sequence/sequence_0001/sampling_frequency",
			}, "sampling_frequency");

		double pitch_m = PICMUS_ReadScalarAny(file_id,
			{
				"/channel_data/probe/pitch",
				"/probe/pitch",
				"/channel_data/probe_geometry/pitch",
			}, "probe pitch");
		array_pitch_mm = (float)(pitch_m * 1000.0);   // м -> мм

		picmus_sound_speed = (float)PICMUS_ReadScalarAny(file_id,
			{
				"/channel_data/sequence/sequence_0001/sound_speed",
				"/channel_data/sound_speed",
				"/sequence/sequence_0001/sound_speed",
			}, "sound_speed");

		// --- углы плоских волн (радианы) ---
		// В формате PICMUS/USTB угол КАЖДОЙ волны хранится отдельным скаляром
		// source/azimuth внутри своей группы sequence_XXXX (XXXX = 0001..n_angles),
		// а не единым массивом. Поэтому читаем по одному значению на угол.
		angles_rad.resize(n_angles);
		bool angles_ok = true;
		for (int a = 0; a < n_angles; ++a)
		{
			char path[256];
			snprintf(path, sizeof(path), "/channel_data/sequence/sequence_%04d/source/azimuth", a + 1);

			if (PICMUS_DatasetExists(file_id, path))
			{
				angles_rad[a] = PICMUS_ReadScalar(file_id, path);
			}
			else
			{
				angles_ok = false;
				angles_rad[a] = 0.0;   // безопасное значение-заглушка
			}
		}

		if (!angles_ok)
		{
			printf("\n  WARNING: some angle entries (source/azimuth) not found -- "
				"angles_rad may be incomplete!");
		}

		// защита от нуля / очевидно некорректных значений
		if (array_pitch_mm == 0) array_pitch_mm = 0.3f;   // типичный pitch PICMUS
		if (picmus_sampling_freq == 0) picmus_sampling_freq = 20e6f;
		if (picmus_sound_speed == 0) picmus_sound_speed = 1540.0f;

		// выбранный угол по умолчанию (используется только для Display()/отладки)
		if (angle_index < 0) angle_index = 0;
		if (angle_index >= n_angles) angle_index = n_angles / 2;

		H5Fclose(file_id);
	}
	catch (const std::exception&) {
		if (dataset_id >= 0) H5Dclose(dataset_id);
		if (file_id >= 0) H5Fclose(file_id);
		throw;
	}
}

void RawSFDataSourcePICMUS::LoadHDF5Data(const std::string& h5_filename)
{
	hid_t file_id = -1, dataset_id = -1;

	try {
		file_id = H5Fopen(h5_filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
		if (file_id < 0)
			throw runtime_error("Cannot open HDF5 file");

		dataset_id = H5Dopen2(file_id, "/channel_data/data", H5P_DEFAULT);
		if (dataset_id < 0)
			throw runtime_error("Cannot open /channel_data/data");

		hid_t space_id = H5Dget_space(dataset_id);
		int ndims = H5Sget_simple_extent_ndims(space_id);
		std::vector<hsize_t> dims(ndims);
		H5Sget_simple_extent_dims(space_id, dims.data(), NULL);

		hsize_t total_elements = 1;
		for (int i = 0; i < ndims; ++i) total_elements *= dims[i];

		printf("\n\nLoading PICMUS data (ALL %d angles)...", n_angles);
		printf("\n  Total elements in cube: %llu", (unsigned long long)total_elements);
		fflush(stdout);

		// Читаем ВЕСЬ куб как double за один раз (как в beamforming_main).
		std::vector<double> all_data(total_elements);
		hid_t mem_space = H5Screate_simple(1, &total_elements, NULL);

		herr_t status = H5Dread(dataset_id, H5T_NATIVE_DOUBLE, mem_space, space_id,
			H5P_DEFAULT, all_data.data());
		if (status < 0)
			throw runtime_error("Cannot read /channel_data/data");

		H5Sclose(mem_space);
		H5Sclose(space_id);

		// Смещения для доступа к [angle][element][sample] в плоском массиве:
		// index = angle*(nElems*nSamples) + element*nSamples + sample
		hsize_t stride_angle = dims[1] * dims[2];
		hsize_t stride_element = dims[2];

		// Выделяем комплексный контейнер под ВСЕ углы: [n_tx_elements=n_angles, n_rx_elements, samples]
		data.realloc({ n_tx_elements, n_rx_elements, samples_per_element });
		data.fill(complexF32(0));

		printf("\n  Allocated: %zu x %zu x %zu complex samples",
			n_tx_elements, n_rx_elements, samples_per_element);
		fflush(stdout);

		GUIProgressBar progress;
		progress.start("Loading PICMUS + Hilbert (all angles)", (size_t)n_angles * n_rx_elements);

		size_t fft_size = ceil_fft_length(samples_per_element);

		// Внешний цикл -- по углам (каждый угол = отдельная плоская волна = отдельная "передача").
		for (size_t a = 0; a < (size_t)n_angles; ++a)
		{
			for (size_t rx = 0; rx < n_rx_elements; ++rx)
			{
				// 1) собрать вещественную RF-строку для угла a, элемента rx
				ComplexFunctionF32 buffer(fft_size, complexF32(0));

				for (size_t s = 0; s < samples_per_element; ++s) {
					hsize_t idx = (hsize_t)a * stride_angle
						+ (hsize_t)rx * stride_element
						+ (hsize_t)s;
					buffer[s] = complexF32((float)all_data[idx], 0);
				}

				// 2) преобразование Гильберта: RF -> аналитический сигнал (как в Verasonics)
				FFTf(buffer, fftFwd);
				for (size_t i = 1; i < fft_size / 2; ++i)          buffer[i] *= 2.0f;   // удвоить положит. частоты
				for (size_t i = fft_size / 2 + 1; i < fft_size; ++i) buffer[i] = 0;      // обнулить отрицат. частоты
				FFTf(buffer, fftRev);

				// 3) записать IQ в контейнер (tx = номер угла a)
				typename data_volume_type::row_type row;
				data.GetRow(row, { a, rx, slice_mask(0) });
				for (size_t s = 0; s < samples_per_element; ++s)
					row[s] = buffer[s];

				progress.next();
			}
		}
		progress.end();

		printf("\n  PICMUS load + Hilbert complete (all angles)\n");
		fflush(stdout);

		H5Dclose(dataset_id);
		H5Fclose(file_id);
	}
	catch (const std::exception& e) {
		if (dataset_id >= 0) H5Dclose(dataset_id);
		if (file_id >= 0) H5Fclose(file_id);
		throw runtime_error(string("PICMUS HDF5 error: ") + e.what());
	}
}

void RawSFDataSourcePICMUS::LoadData()
{
	LoadHDF5Data(filename);
}

string RawSFDataSourcePICMUS::GetFileName() const
{
	return filename;
}

void RawSFDataSourcePICMUS::Display() const
{
	parent::DisplayUtility(data);
}

RawSFDataSourcePICMUS::row_ptr RawSFDataSourcePICMUS::GetRow(size_t tx, size_t rx)
{
	row_t* result = new row_t;

	typename data_volume_type::row_type row;
	data.GetRow(row, { tx, rx, slice_mask(0) });

	result->data.realloc(row.size());
	for (size_t i = 0; i < row.size(); ++i)
		result->data[i] = row[i];

	return row_ptr(result);
}

XRAD_END