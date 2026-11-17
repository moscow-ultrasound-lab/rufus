// RawSFDataSourceVerasonics.cpp - версия с прямым использованием HDF5 C API

#include "pre.h"
#include "RawSFDataSourceVerasonics.h"
#include <hdf5.h>

XRAD_BEGIN

// Вспомогательные методы
bool RawSFDataSourceVerasonics::IsSyntheticApertureTransmit(int tx_index) const
{
    // Для Philips S5-1: synthetic aperture transmits = первые 80
    // Для Verasonics L11-5V: все 128 transmits - synthetic aperture
    return (tx_index < (int)n_rx_elements);
}

bool RawSFDataSourceVerasonics::IsPlaneWaveTransmit(int tx_index) const
{
    return !IsSyntheticApertureTransmit(tx_index);
}

void RawSFDataSourceVerasonics::Init()
{
    filename = GetFileNameRead("Verasonics HDF5 file",
        SavedGUIValue("*.hdf5"),
        "*.hdf5");

    LoadHDF5Metadata(filename);

    // Настройка параметров
    raw_signal_sample_rate = Hz(verasonics_sampling_freq);
    hilbert_signal_sample_rate = raw_signal_sample_rate;
    hilbert_samples = samples_per_element;

    // ============================================================
    // НАСТРОЙКА ДЛЯ ЛИНЕЙНОГО ДАТЧИКА (Verasonics L11-5V, Philips S5-1)
    // ============================================================

    // Для линейного датчика: sector mode = false, rectangle mode = true
    is_linear_array = true;

    // Для линейного датчика - нет угла сектора
    recommended_angle = degrees(0);
    recommended_n_rays = n_rx_elements;

    probe_carrier = Hz(verasonics_center_freq);
    probe_bandwidth = probe_carrier / 2;
    sound_speed = m_sec(verasonics_sound_speed);

    if (n_rx_elements == 80) {  // S5-1 phantom
        // Эмпирически подобранные параметры
        array_pitch_mm = 0.0762f;  // рабочий pitch для фокусировки
        recommended_angle = degrees(45);
        is_linear_array = false;
    }
    else if (n_rx_elements == 128) {  // L11-5V carotid
        array_pitch_mm = 0.226f;      // ЭМПИРИЧЕСКИЙ pitch (не 0.3!)
        recommended_angle = degrees(0);
        is_linear_array = true;
    }

    array_pitch = mm(array_pitch_mm);

    first_raw_sample = 0;
   // samples_per_element /= 2; //В данных из базы представлен off-grid кадр на большой глубине, поэтому половина кадра - шум (пока оставим это деление для ускорения отладки)

    last_raw_sample = samples_per_element;

    // Вычисляем параметры для отображения
    physical_length scan_width = array_pitch * n_rx_elements;
    physical_length scan_depth = (samples_per_element * sound_speed / raw_signal_sample_rate) / 2;

    printf("\n=== Verasonics Data Loaded ===");
    printf("\n  File: %s", filename.c_str());
    printf("\n  Frames: %zu", n_frames);
    printf("\n  Transmits (SA only): %zu", n_tx_elements);
    printf("\n  Receive elements: %zu", n_rx_elements);
    printf("\n  Samples per element: %zu", samples_per_element);
    printf("\n  Sample rate: %.2f MHz", raw_signal_sample_rate.MHz());
    printf("\n  Center freq: %.2f MHz", probe_carrier.MHz());
    printf("\n  Sound speed: %.0f m/s", sound_speed.m_sec());
    printf("\n  Array pitch: %.3f mm", array_pitch.mm());
    printf("\n  Linear array: scan width = %.2f mm, depth = %.2f mm",
        scan_width.mm(), scan_depth.mm());
    printf("\n  Memory: %.1f MB\n",
        (double)n_tx_elements * n_rx_elements * samples_per_element * sizeof(complexF32) / 1048576);
    fflush(stdout);
}

void RawSFDataSourceVerasonics::LoadHDF5Metadata(const std::string& h5_filename)
{
    hid_t file_id = -1, dataset_id = -1;

    try {
        file_id = H5Fopen(h5_filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
        if (file_id < 0) {
            throw runtime_error("Cannot open HDF5 file");
        }

        dataset_id = H5Dopen2(file_id, "/data/raw_data", H5P_DEFAULT);
        if (dataset_id < 0) {
            throw runtime_error("Cannot open /data/raw_data");
        }

        hid_t space_id = H5Dget_space(dataset_id);
        int ndims = H5Sget_simple_extent_ndims(space_id);
        std::vector<hsize_t> dims(ndims);
        H5Sget_simple_extent_dims(space_id, dims.data(), NULL);

        printf("\n\nRaw dataset dimensions: ");
        for (int i = 0; i < ndims; ++i) {
            printf("[%llu]", (unsigned long long)dims[i]);
            if (i < ndims - 1) printf(" x ");
        }

        if (ndims >= 4) {
            n_frames = dims[0];
            n_tx_elements = dims[1];
            samples_per_element = dims[2];
            n_rx_elements = dims[3];
        }

        H5Sclose(space_id);

        // Читаем атрибуты
        hid_t scan_group = H5Gopen2(file_id, "/scan", H5P_DEFAULT);
        if (scan_group >= 0) {
            auto read_attr = [&](const char* name, float& val, float default_val) {
                hid_t attr = H5Aopen(scan_group, name, H5P_DEFAULT);
                if (attr >= 0) {
                    double dval;
                    H5Aread(attr, H5T_NATIVE_DOUBLE, &dval);
                    val = (float)dval;
                    H5Aclose(attr);
                    printf("\n  Found attribute: %s = %g", name, val);
                }
                else {
                    val = default_val;
                    printf("\n  Using default for %s: %g", name, default_val);
                }
            };

            read_attr("sampling_frequency", verasonics_sampling_freq, 40e6f);
            read_attr("center_frequency", verasonics_center_freq, 7.8e6f);
            read_attr("sound_speed", verasonics_sound_speed, 1540.0f);

            // Пробуем прочитать pitch из geometry
            if (scan_group >= 0) {
                hid_t geom_ds = H5Dopen2(scan_group, "probe_geometry", H5P_DEFAULT);
                if (geom_ds >= 0) {
                    hid_t geom_space = H5Dget_space(geom_ds);
                    hsize_t geom_dims[2];
                    H5Sget_simple_extent_dims(geom_space, geom_dims, NULL);

                    if (geom_dims[0] >= 2 && geom_dims[1] >= 3) {
                        std::vector<double> geom_flat(geom_dims[0] * geom_dims[1]);
                        H5Dread(geom_ds, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, geom_flat.data());

                        double x0 = geom_flat[0];
                        double x1 = geom_flat[geom_dims[1]];
                        array_pitch_mm = (float)(fabs(x1 - x0) * 1000.0);
                        printf("\n  Array pitch from geometry: %.3f mm", array_pitch_mm);
                    }
                    H5Sclose(geom_space);
                    H5Dclose(geom_ds);
                }
            }

            H5Gclose(scan_group);
        }

        // Фильтрация transmits - берем только synthetic aperture
        transmits.clear();

        if (n_tx_elements > n_rx_elements) {
            printf("\n  Detected %zu extra transmits (plane waves), keeping only first %zu synthetic aperture",
                n_tx_elements - n_rx_elements, n_rx_elements);
            for (size_t i = 0; i < n_rx_elements; ++i) {
                transmits.push_back((int)i);
            }
        }
        else {
            printf("\n  All %zu transmits are synthetic aperture", n_tx_elements);
            for (size_t i = 0; i < n_tx_elements; ++i) {
                transmits.push_back((int)i);
            }
        }

        printf("\n  Selected %zu transmits for processing", transmits.size());
        n_tx_elements = transmits.size();

        if (array_pitch_mm == 0) {
            array_pitch_mm = 0.3f;  // default для Verasonics L11-5V
        }

        H5Dclose(dataset_id);
        H5Fclose(file_id);
    }
    catch (const std::exception& e) {
        if (dataset_id >= 0) H5Dclose(dataset_id);
        if (file_id >= 0) H5Fclose(file_id);
        throw;
    }
}
void RawSFDataSourceVerasonics::LoadHDF5Data(const std::string& h5_filename)
{
    hid_t file_id = -1, dataset_id = -1;

    try {
        file_id = H5Fopen(h5_filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
        if (file_id < 0) {
            throw runtime_error("Cannot open HDF5 file");
        }

        dataset_id = H5Dopen2(file_id, "/data/raw_data", H5P_DEFAULT);
        if (dataset_id < 0) {
            throw runtime_error("Cannot open /data/raw_data");
        }

        hid_t space_id = H5Dget_space(dataset_id);
        int ndims = H5Sget_simple_extent_ndims(space_id);
        std::vector<hsize_t> dims(ndims);
        H5Sget_simple_extent_dims(space_id, dims.data(), NULL);

        printf("\n\nLoading data...");
        printf("\n  Dataset dimensions: [");
        for (int i = 0; i < ndims; ++i) {
            printf("%llu", (unsigned long long)dims[i]);
            if (i < ndims - 1) printf(" x ");
        }
        printf("]");
        fflush(stdout);

        hsize_t total_elements = 1;
        for (int i = 0; i < ndims; ++i) {
            total_elements *= dims[i];
        }
        printf("\n  Total elements: %llu", (unsigned long long)total_elements);

        std::vector<short> all_data(total_elements);
        hid_t mem_space = H5Screate_simple(1, &total_elements, NULL);
        hid_t dtype = H5T_NATIVE_SHORT;

        herr_t status = H5Dread(dataset_id, dtype, mem_space, space_id, H5P_DEFAULT, all_data.data());
        if (status < 0) {
            throw runtime_error("Cannot read dataset");
        }

        printf("\n  Successfully read %llu short elements", (unsigned long long)total_elements);

        H5Sclose(mem_space);
        H5Sclose(space_id);

        hsize_t stride_frame = dims[1] * dims[2] * dims[3] * dims[4];
        hsize_t stride_transmit = dims[2] * dims[3] * dims[4];
        hsize_t stride_sample = dims[3] * dims[4];
        hsize_t stride_element = dims[4];

        printf("\n  Strides: frame=%llu, transmit=%llu, sample=%llu, element=%llu",
            (unsigned long long)stride_frame,
            (unsigned long long)stride_transmit,
            (unsigned long long)stride_sample,
            (unsigned long long)stride_element);

        data.realloc({ transmits.size(), n_rx_elements, samples_per_element });
        data.fill(complexF32(0));

        printf("\n  Allocated volume: %zu x %zu x %zu = %zu complex samples",
            transmits.size(), n_rx_elements, samples_per_element,
            transmits.size() * n_rx_elements * samples_per_element);
        fflush(stdout);

        // Временный буфер для хранения реальных данных перед преобразованием Гильберта
        // Размер: (transmits) x (elements) x (samples)
        std::vector<std::vector<std::vector<float>>> real_data;
        real_data.resize(transmits.size(),
            std::vector<std::vector<float>>(n_rx_elements,
                std::vector<float>(samples_per_element, 0)));

        GUIProgressBar progress;
        progress.start("Loading Verasonics data", transmits.size() * n_rx_elements);

        size_t total_copied = 0;
        float max_val = 0;
        float min_val = 0;

        // 1-й проход: загружаем реальные данные
        for (size_t tx_idx = 0; tx_idx < transmits.size(); ++tx_idx) {
            size_t tx_src = transmits[tx_idx];

            for (size_t rx = 0; rx < n_rx_elements; ++rx) {
                for (size_t s = 0; s < samples_per_element; ++s) {
                    hsize_t idx = (frame_index * stride_frame) +
                        (tx_src * stride_transmit) +
                        (s * stride_sample) +
                        (rx * stride_element) + 0;

                    if (idx < total_elements) {
                        float val = (float)all_data[idx];
                        if (val > max_val) max_val = val;
                        if (val < min_val) min_val = val;
                        real_data[tx_idx][rx][s] = val;
                        total_copied++;
                    }
                }
                progress.next();
            }
        }
        progress.end();

        printf("\n  Load complete: %zu samples copied", total_copied);
        printf("\n  Value range: [%g, %g]", min_val, max_val);
        fflush(stdout);

        // 2-й проход: преобразование Гильберта (RF -> IQ)
        printf("\n  Performing Hilbert transform (RF -> IQ)...");
        fflush(stdout);

        GUIProgressBar hilbert_progress;
        hilbert_progress.start("Hilbert transform", transmits.size() * n_rx_elements);

        for (size_t tx_idx = 0; tx_idx < transmits.size(); ++tx_idx) {
            for (size_t rx = 0; rx < n_rx_elements; ++rx) {
                // Создаем комплексный сигнал из реального
                ComplexFunctionF32 rf_signal(samples_per_element);
                for (size_t s = 0; s < samples_per_element; ++s) {
                    rf_signal[s] = complexF32(real_data[tx_idx][rx][s], 0);
                }

                // Выполняем преобразование Гильберта через FFT
                size_t fft_size = ceil_fft_length(samples_per_element);
                ComplexFunctionF32 buffer(fft_size, complexF32(0));

                // Копируем сигнал в буфер
                for (size_t s = 0; s < samples_per_element; ++s) {
                    buffer[s] = rf_signal[s];
                }

                // Прямое FFT
                FFTf(buffer, fftFwd);

                // Обнуляем отрицательные частоты (преобразование Гильберта)
                // Для получения аналитического сигнала:
                // - Умножаем положительные частоты на 2
                // - Обнуляем отрицательные частоты
                for (size_t i = 1; i < fft_size / 2; ++i) {
                    buffer[i] *= 2.0f;
                }
                for (size_t i = fft_size / 2 + 1; i < fft_size; ++i) {
                    buffer[i] = 0;
                }
                // Постоянная составляющая (i=0) остается без изменений

                // Обратное FFT
                FFTf(buffer, fftRev);

                // Сохраняем IQ данные
                typename data_volume_type::row_type row;
                data.GetRow(row, { tx_idx, rx, slice_mask(0) });

                for (size_t s = 0; s < samples_per_element; ++s) {
                    row[s] = buffer[s];
                }

                hilbert_progress.next();
            }
        }
        hilbert_progress.end();

        printf("\n  Hilbert transform complete");
        printf("\n  Memory used: %.1f MB\n",
            transmits.size() * n_rx_elements * samples_per_element * sizeof(complexF32) / 1048576.0);
        fflush(stdout);

        H5Dclose(dataset_id);
        H5Fclose(file_id);
    }
    catch (const std::exception& e) {
        if (dataset_id >= 0) H5Dclose(dataset_id);
        if (file_id >= 0) H5Fclose(file_id);
        throw runtime_error(string("HDF5 error: ") + e.what());
    }
}

void RawSFDataSourceVerasonics::LoadData()
{
    LoadHDF5Data(filename);
}

string RawSFDataSourceVerasonics::GetFileName() const
{
    return filename;
}

void RawSFDataSourceVerasonics::Display() const
{
    parent::DisplayUtility(data);
}

RawSFDataSourceVerasonics::row_ptr RawSFDataSourceVerasonics::GetRow(size_t tx, size_t rx)
{
    row_t* result = new row_t;

    typename data_volume_type::row_type row;
    data.GetRow(row, { tx, rx, slice_mask(0) });

    result->data.realloc(row.size());
    for (size_t i = 0; i < row.size(); ++i) {
        result->data[i] = row[i];
    }

    return row_ptr(result);
}

XRAD_END