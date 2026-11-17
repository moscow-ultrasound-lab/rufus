// RawSFDataSourceCUBDL.cpp
#include "pre.h"
#include "RawSFDataSourceCUBDL.h"

// Включаем HDF5 здесь - это дает определение hid_t
#include <hdf5.h>

#include <algorithm>
#include <cmath>
#include <cstring>

XRAD_BEGIN

// ============================================================
// ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ
// ============================================================

bool RawSFDataSourceCUBDL::IsSyntheticApertureTransmit(int tx_index) const
{
    return (tx_index < (int)n_rx_elements);
}

bool RawSFDataSourceCUBDL::IsPlaneWaveTransmit(int tx_index) const
{
    return !IsSyntheticApertureTransmit(tx_index);
}

// ============================================================
// ПАРСИНГ ID ПОСЛЕДОВАТЕЛЬНОСТИ
// ============================================================

void RawSFDataSourceCUBDL::ParseSequenceID(const std::string& seq_id)
{
    // Определяем институт и параметры по префиксу
    if (seq_id.find("JHU") == 0) 
    {
        metadata.institution = "Johns Hopkins University";
        metadata.is_phantom = false;
        metadata.tissue_type = "breast";
        verasonics_sound_speed = 1540.0f;

        // JHU: частота зависит от датчика
        if (seq_id.find("JHU024") == 0 || seq_id.find("JHU025") == 0 ||
            seq_id.find("JHU026") == 0 || seq_id.find("JHU027") == 0 ||
            seq_id.find("JHU028") == 0 || seq_id.find("JHU029") == 0 ||
            seq_id.find("JHU030") == 0 || seq_id.find("JHU031") == 0 ||
            seq_id.find("JHU032") == 0 || seq_id.find("JHU033") == 0 ||
            seq_id.find("JHU034") == 0)
        {
            verasonics_center_freq = 12.5e6f;
            verasonics_sampling_freq = 40e6f;
            array_pitch_mm = 0.2f;
            // Углы для JHU
            angle_min_deg = -16.0;
            angle_max_deg = 16.0;
            n_angles = 75;
        }
        else
        {
            verasonics_center_freq = 5.5e6f;
            verasonics_sampling_freq = 40e6f;
            array_pitch_mm = 0.3f;
            angle_min_deg = -16.0;
            angle_max_deg = 16.0;
            n_angles = 75;
        }
    }
    else if (seq_id.find("MYO") == 0) 
    {
        metadata.institution = "Mayo Clinic";
        metadata.is_phantom = true;
        metadata.phantom_model = "CIRS/ATS with porcine tissue";

        verasonics_center_freq = 6.25e6f;
        verasonics_sampling_freq = 28.96e6f;
        verasonics_sound_speed = 1540.0f;
        array_pitch_mm = 0.3f;

        inc = 2.57;  // проверить - выставляем для выравнивания "качелей" плоских волн

        // MYO: -16..16 градусов, 75 углов
        angle_min_deg = -16.0;
        angle_max_deg = 16.0;
        n_angles = 75;
    }
    else if (seq_id.find("UFL") == 0) 
    {
        metadata.institution = "University of Florence";
        metadata.is_phantom = true;

        verasonics_center_freq = 8e6f;
        verasonics_sampling_freq = 78.125e6f;
        verasonics_sound_speed = 1540.0f;
        array_pitch_mm = 0.245f;

        // UFL: -16..16 градусов, 75 углов
        angle_min_deg = -16.0;
        angle_max_deg = 16.0;
        n_angles = 75;
    }
    else if (seq_id.find("EUT") == 0) 
    {
        metadata.institution = "Eindhoven University of Technology";
        metadata.is_phantom = true;

        verasonics_center_freq = 3.1e6f;
        verasonics_sampling_freq = 12.5e6f;
        verasonics_sound_speed = 1540.0f;
        array_pitch_mm = 0.254f;      // Philips S5-1

        // EUT: -16..16 градусов, 75 углов
        angle_min_deg = -16.0;
        angle_max_deg = 16.0;
        n_angles = 75;
    }
    else if (seq_id.find("INS") == 0) 
    {
        metadata.institution = "INSA Lyon / CREATIS";
        metadata.is_phantom = true;
        

        // Извлекаем номер последовательности
        int seq_num = 0;
        if (seq_id.length() > 3)
        {
            seq_num = std::stoi(seq_id.substr(3));
        }

        // INS001-INS003: L7-4
        // INS004-INS007: L12-5
        if (seq_num >= 1 && seq_num <= 13)
        {
            // L7-4
            verasonics_center_freq = 5.208e6f;
            verasonics_sampling_freq = 20.833e6f;
            array_pitch_mm = 0.298f;
            inc = 1.7;  // проверить - выставляем для выравнивания "качелей" плоских волн
        }
        else if (seq_num >= 14)
        {
            // L12-5
            verasonics_center_freq = 7.8e6f;
            verasonics_sampling_freq = 31.25e6f;
            array_pitch_mm = 0.1953f;
            inc = 1.5;  // проверить - выставляем для выравнивания "качелей" плоских волн
        }

        verasonics_sound_speed = 1540.0f;

        // INS: -16..16 градусов, 75 углов
        angle_min_deg = -16.0;
        angle_max_deg = 16.0;
        n_angles = 75;
    }
    else if (seq_id.find("OSL") == 0) 
    {
        metadata.institution = "University of Oslo";
        metadata.is_phantom = false;
        inc = 0;

        if (seq_id.find("OSL010") == 0) 
        {
            // Field II simulation
            metadata.is_phantom = true;
            metadata.phantom_model = "Field II Simulation";
            verasonics_center_freq = 5.132e6f;
            verasonics_sampling_freq = 20.8e6f;
            array_pitch_mm = 0.3f;
            angle_min_deg = -16.0;
            angle_max_deg = 16.0;
            n_angles = 75;
        }
        else if (seq_id.find("OSL011") == 0 || seq_id.find("OSL012") == 0 ||
            seq_id.find("OSL013") == 0 || seq_id.find("OSL014") == 0) 
        {
            // In vivo cardiac
            metadata.tissue_type = "cardiac";
            verasonics_center_freq = 5.2e6f;
            verasonics_sampling_freq = 20.8e6f;
            array_pitch_mm = 0.3f;
            angle_min_deg = -16.0;
            angle_max_deg = 16.0;
            n_angles = 75;
        }
        else 
        {
            // OSL001 phantom
            metadata.is_phantom = true;
            verasonics_center_freq = 5.2e6f;
            verasonics_sampling_freq = 20.8e6f;
            array_pitch_mm = 0.3f;
            angle_min_deg = -16.0;
            angle_max_deg = 16.0;
            n_angles = 75;
        }
        verasonics_sound_speed = 1540.0f;
    }
    else if (seq_id.find("TSH") == 0) 
{
        metadata.institution = "Tsinghua University";
        metadata.is_phantom = false;
        metadata.tissue_type = "brachioradialis";

        verasonics_center_freq = 7.5e6f;
        verasonics_sampling_freq = 6.25e6f;
        verasonics_sound_speed = 1540.0f;
        array_pitch_mm = 0.27f;

        // TSH: -15..15 градусов, 31 угол (особый случай!)
        angle_min_deg = -15.0;
        angle_max_deg = 15.0;
        n_angles = 31;
    }
    else 
    {
        metadata.institution = "Unknown";
        metadata.is_phantom = true;

        verasonics_center_freq = 7.8e6f;
        verasonics_sampling_freq = 40e6f;
        verasonics_sound_speed = 1540.0f;
        array_pitch_mm = 0.3f;
        angle_min_deg = -16.0;
        angle_max_deg = 16.0;
        n_angles = 75;
    }

    metadata.sequence_id = seq_id;
    metadata.center_freq_mhz = verasonics_center_freq / 1e6;
    metadata.sampling_freq_mhz = verasonics_sampling_freq / 1e6;
    metadata.pitch_mm = array_pitch_mm;
    metadata.n_plane_waves = n_angles;

    angles_rad.clear();
    angles_rad.resize(n_angles);

    for (int i = 0; i < n_angles; ++i)
    {
        // От -angle_max к +angle_max (как в статье)
        double angle_deg = angle_min_deg + (angle_max_deg - angle_min_deg) * i / (n_angles - 1);
        angles_rad[i] = angle_deg * 3.14159265358979323846 / 180.0; // градусы -> радианы
    }

    printf("\n  Set parameters for sequence %s:", seq_id.c_str());
    printf("\n    Center freq: %.2f MHz", metadata.center_freq_mhz);
    printf("\n    Sampling freq: %.2f MHz", metadata.sampling_freq_mhz);
    printf("\n    Sound speed: %.0f m/s", verasonics_sound_speed);
    printf("\n    Pitch: %.3f mm", array_pitch_mm);
    printf("\n    Angles: %d from %.1f to %.1f degrees", n_angles, angle_min_deg, angle_max_deg);
}

void RawSFDataSourceCUBDL::AnalyzeDataset(hid_t dataset_id, const std::string& path)
{
    hid_t space_id = H5Dget_space(dataset_id);
    int ndims = H5Sget_simple_extent_ndims(space_id);
    std::vector<hsize_t> dims(ndims);
    H5Sget_simple_extent_dims(space_id, dims.data(), NULL);

    layout.dims.clear();
    layout.n_frames = 1;
    layout.n_transmits = 1;
    layout.n_samples = 1;
    layout.n_elements = 1;

    printf("\n  Dataset at '%s' dimensions: ", path.c_str());
    for (int i = 0; i < ndims; ++i) {
        layout.dims.push_back((size_t)dims[i]);
        printf("[%zu]", (size_t)dims[i]);
        if (i < ndims - 1) printf(" x ");
    }
    printf("\n");

    // Определяем layout в зависимости от размерности
    if (ndims >= 4) {
        // Формат: [frame, transmit, sample, element]
        layout.n_frames = (size_t)dims[0];
        layout.n_transmits = (size_t)dims[1];
        layout.n_samples = (size_t)dims[3];
        layout.n_elements = (size_t)dims[2];
    }
    else if (ndims == 3) {                                                 // идем по этому сценарию для MYO003 
        // Формат: [transmit, sample, element]
        layout.n_transmits = (size_t)dims[0];
        layout.n_samples = (size_t)dims[2];
        layout.n_elements = (size_t)dims[1];
    }
    else if (ndims == 2) {
        // Формат: [sample, element] (один transmit)
        layout.n_transmits = 1;
        layout.n_samples = (size_t)dims[0];
        layout.n_elements = (size_t)dims[1];
    }

    // Проверяем тип данных
    hid_t dtype = H5Dget_type(dataset_id);
    H5T_class_t class_id = H5Tget_class(dtype);
    size_t type_size = H5Tget_size(dtype);

    if (class_id == H5T_INTEGER && type_size == 2) {
        layout.data_type = 1; // int16
    }
    else if (class_id == H5T_INTEGER && type_size == 1) {
        layout.data_type = 1; // int8 - будем преобразовывать
    }
    else if (class_id == H5T_FLOAT && type_size == 4) {
        layout.data_type = 2; // float32
    }
    else if (class_id == H5T_COMPOUND) {
        layout.data_type = 3; // complex
    }
    else {
        layout.data_type = 0; // unknown
    }

    H5Tclose(dtype);
    H5Sclose(space_id);

    // Вычисляем общий размер
    layout.total_size = 1;
    for (size_t d : layout.dims) {
        layout.total_size *= d;
    }


    // Используем члены базового класса
    n_tx_elements = (int)layout.n_transmits;
    n_rx_elements = (int)layout.n_elements;
    samples_per_element = (int)layout.n_samples;
    n_samples = (int)layout.n_samples;
}

// ============================================================
// ЧТЕНИЕ ДАННЫХ ИЗ HDF5
// ============================================================

void RawSFDataSourceCUBDL::ReadDatasetGeneric(hid_t dataset_id, std::vector<int16_t>& buffer)
{
    hid_t dtype = H5Dget_type(dataset_id);
    H5T_class_t class_id = H5Tget_class(dtype);

    // Определяем размер буфера
    hid_t space_id = H5Dget_space(dataset_id);
    hsize_t total_elements = 1;
    int ndims = H5Sget_simple_extent_ndims(space_id);
    std::vector<hsize_t> dims(ndims);
    H5Sget_simple_extent_dims(space_id, dims.data(), NULL);

    for (int i = 0; i < ndims; ++i) {
        total_elements *= dims[i];
    }

    buffer.resize((size_t)total_elements);

    // Выбираем тип для чтения
    if (class_id == H5T_INTEGER) {
        if (H5Tget_size(dtype) == 2) {
            // Читаем как int16
            H5Dread(dataset_id, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer.data());
        }
        else if (H5Tget_size(dtype) == 1) {
            // Читаем как int8 и преобразуем
            std::vector<int8_t> temp((size_t)total_elements);
            H5Dread(dataset_id, H5T_NATIVE_CHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, temp.data());
            for (size_t i = 0; i < (size_t)total_elements; ++i) {
                buffer[i] = (int16_t)temp[i];
            }
        }
        else {
            // Читаем как int32
            std::vector<int32_t> temp((size_t)total_elements);
            H5Dread(dataset_id, H5T_NATIVE_INT32, H5S_ALL, H5S_ALL, H5P_DEFAULT, temp.data());
            for (size_t i = 0; i < (size_t)total_elements; ++i) {
                buffer[i] = (int16_t)temp[i];
            }
        }
    }
    else if (class_id == H5T_FLOAT) {
        // Читаем как float и преобразуем в int16
        std::vector<float> temp((size_t)total_elements);
        H5Dread(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, temp.data());
        // Находим максимум для нормализации
        float max_val = 0;
        for (size_t i = 0; i < (size_t)total_elements; ++i) {
            if (fabs(temp[i]) > max_val) max_val = fabs(temp[i]);
        }
        if (max_val > 0) {
            for (size_t i = 0; i < (size_t)total_elements; ++i) {
                buffer[i] = (int16_t)(temp[i] / max_val * 32767.0f);
            }
        }
    }
    else {
        throw runtime_error("Unsupported data type for dataset");
    }

    H5Tclose(dtype);
    H5Sclose(space_id);

    printf("  Read %zu elements\n", (size_t)total_elements);
}

// ============================================================
// ЗАГРУЗКА МЕТАДАННЫХ
// ============================================================

void RawSFDataSourceCUBDL::LoadHDF5Metadata(const std::string& h5_filename)
{
    hid_t file_id = -1, dataset_id = -1;

    try 
    {
        file_id = H5Fopen(h5_filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
        if (file_id < 0) 
        {
            throw runtime_error("Cannot open HDF5 file");
        }

        // Пытаемся найти данные в разных путях
        const char* data_paths[] = {
            "/data/raw_data",
            "/raw_data",
            "/channel_data",
            "/RFdata",
            "/RcvData"
        };

        bool found = false;
        for (const char* path : data_paths) {
            dataset_id = H5Dopen2(file_id, path, H5P_DEFAULT);
            if (dataset_id >= 0) {
                printf("\n  Found dataset at: %s", path);
                AnalyzeDataset(dataset_id, path);
                found = true;
                break;
            }
        }

        if (!found) 
        {
            throw runtime_error("No dataset found in HDF5 file");
        }
        
        // Читаем атрибуты из корня
        hid_t root_group = H5Gopen2(file_id, "/", H5P_DEFAULT);
        if (root_group >= 0) 
        {
            // Пытаемся прочитать sequence_id
            hid_t seq_attr = H5Aopen(root_group, "sequence_id", H5P_DEFAULT);
            if (seq_attr >= 0) 
            {
                char seq_buf[256] = { 0 };
                H5Aread(seq_attr, H5T_STRING, seq_buf);
                sequence_number = std::string(seq_buf);
                ParseSequenceID(sequence_number);
                H5Aclose(seq_attr);
            }

            H5Gclose(root_group);
        }

        // Настройка параметров - используем члены базового класса
        raw_signal_sample_rate = Hz(verasonics_sampling_freq);
        hilbert_signal_sample_rate = raw_signal_sample_rate;
        hilbert_samples = samples_per_element;

        // is_linear_array - член базового класса RawSFDataSource
        this->is_linear_array = true;
        recommended_angle = degrees(0);
        recommended_n_rays = n_rx_elements;

        probe_carrier = Hz(verasonics_center_freq);
        probe_bandwidth = probe_carrier / 2;
        sound_speed = m_sec(verasonics_sound_speed);


        array_pitch = mm(array_pitch_mm);

        first_raw_sample = 0;
        last_raw_sample = samples_per_element;




        printf("\n=== READING ANGLES FROM CUBDL FILE ===");
        fflush(stdout);
        bool angles_loaded = false;
        hid_t angles_ds = H5Dopen2(file_id, "/angles", H5P_DEFAULT);
        if (angles_ds >= 0)
        {
            hid_t angles_space = H5Dget_space(angles_ds);
            int ndims = H5Sget_simple_extent_ndims(angles_space);
            std::vector<hsize_t> dims(ndims);
            H5Sget_simple_extent_dims(angles_space, dims.data(), NULL);

            printf("\n  /angles dataset dimensions: ");
            for (int d = 0; d < ndims; ++d)
            {
                printf("[%llu]", (unsigned long long)dims[d]);
                if (d < ndims - 1) printf(" x ");
            }

            // Вычисляем общее количество углов
            hsize_t total_angles = 1;
            for (int d = 0; d < ndims; ++d)
            {
                total_angles *= dims[d];
            }

            printf("\n  Total angles in dataset: %llu", (unsigned long long)total_angles);

            if (total_angles > 0 && total_angles == (hsize_t)n_tx_elements)
            {
                // Читаем углы
                std::vector<double> angles(total_angles);
                herr_t status = H5Dread(angles_ds, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, angles.data());

                if (status >= 0)
                {
                    angles_rad.resize(total_angles);
                    for (size_t i = 0; i < total_angles; ++i)
                    {
                        angles_rad[i] = angles[i];
                    }
                    angles_loaded = true;

                    printf("\n  Successfully read %llu angles from /angles", (unsigned long long)total_angles);
                    printf("\n  First angle: %.4f rad (%.2f deg)", angles[0], angles[0] * 180.0 / 3.14159265358979323846);
                    printf("\n  Last angle: %.4f rad (%.2f deg)", angles[total_angles - 1], angles[total_angles - 1] * 180.0 / 3.14159265358979323846);
                }
                else
                {
                    printf("\n  ERROR: Failed to read /angles dataset");
                }
            }
            else if (total_angles > 0 && total_angles != (hsize_t)n_tx_elements)
            {
                printf("\n  WARNING: angles count (%llu) != n_tx_elements (%d)",
                    (unsigned long long)total_angles, n_tx_elements);
                printf("\n  Using only first %d angles", n_tx_elements);

                // Если углов больше чем нужно - берем первые n_tx_elements
                std::vector<double> angles(total_angles);
                H5Dread(angles_ds, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, angles.data());

                angles_rad.resize(n_tx_elements);
                for (size_t i = 0; i < (size_t)n_tx_elements && i < total_angles; ++i)
                {
                    angles_rad[i] = angles[i];
                }
                angles_loaded = true;
            }

            H5Sclose(angles_space);
            H5Dclose(angles_ds);
        }
        else
        {
            printf("\n  /angles dataset not found, trying other locations...");

            // Пробуем /channel_data/angles
            hid_t channel_group = H5Gopen2(file_id, "/channel_data", H5P_DEFAULT);
            if (channel_group >= 0)
            {
                hid_t ch_angles_ds = H5Dopen2(channel_group, "angles", H5P_DEFAULT);
                if (ch_angles_ds >= 0)
                {
                    printf("\n  Found /channel_data/angles");
                    // ... аналогичное чтение ...
                    H5Dclose(ch_angles_ds);
                }
                H5Gclose(channel_group);
            }

            // Пробуем /scan/angles
            hid_t scan_group = H5Gopen2(file_id, "/scan", H5P_DEFAULT);
            if (scan_group >= 0)
            {
                hid_t sc_angles_ds = H5Dopen2(scan_group, "angles", H5P_DEFAULT);
                if (sc_angles_ds >= 0)
                {
                    printf("\n  Found /scan/angles");
                    // ... аналогичное чтение ...
                    H5Dclose(sc_angles_ds);
                }
                H5Gclose(scan_group);
            }
        }

        printf("\n\n=== ANGLES LOADING COMPLETE ===");
        fflush(stdout);

        // Вывод информации об углах
        if (!angles_rad.empty()) 
        {
            printf("\n  Angle range: %.2f to %.2f degrees",
                angles_rad.front() * 180.0 / 3.14159265358979323846,
                angles_rad.back() * 180.0 / 3.14159265358979323846);
        }

        H5Dclose(dataset_id);
        H5Fclose(file_id);
    }
    catch (const std::exception& e) 
    { 
        if (dataset_id >= 0) H5Dclose(dataset_id); 
        if (file_id >= 0) H5Fclose(file_id); 
        throw;
    }
}

void RawSFDataSourceCUBDL::hilberttransform()
{
    // Выполняем преобразование Гильберта (RF -> IQ)
    printf("\n  Performing Hilbert transform...");
    fflush(stdout);

    GUIProgressBar hilbert_progress;
    hilbert_progress.start("Hilbert transform", (size_t)n_tx_elements* (size_t)n_rx_elements);

    for (size_t tx = 0; tx < (size_t)n_tx_elements; ++tx) {
        for (size_t rx = 0; rx < (size_t)n_rx_elements; ++rx) {
            size_t n_samp = (size_t)samples_per_element;

            ComplexFunctionF32 rf_signal(n_samp);
            for (size_t s = 0; s < n_samp; ++s) {
                rf_signal[s] = data.at({ tx, rx, s });
            }

            size_t fft_size = ceil_fft_length(n_samp);
            ComplexFunctionF32 fft_buffer(fft_size, complexF32(0));

            for (size_t s = 0; s < n_samp; ++s) {
                fft_buffer[s] = rf_signal[s];
            }

            FFTf(fft_buffer, fftFwd);

            // Аналитический сигнал
            for (size_t i = 1; i < fft_size / 2; ++i) {
                fft_buffer[i] *= 2.0f;
            }
            for (size_t i = fft_size / 2 + 1; i < fft_size; ++i) {
                fft_buffer[i] = 0;
            }
            fft_buffer[0] /= 2.0f;
            if (fft_size % 2 == 0) {
                fft_buffer[fft_size / 2] /= 2.0f;
            }

            FFTf(fft_buffer, fftRev);

            for (size_t s = 0; s < n_samp; ++s) {
                data.at({ tx, rx, s }) = fft_buffer[s];
            }

            hilbert_progress.next();
        }
    }
    hilbert_progress.end();

    printf(" Done.\n");
}

// ============================================================
// ЗАГРУЗКА ДАННЫХ (ОСНОВНАЯ ВЕРСИЯ)
// ============================================================

void RawSFDataSourceCUBDL::LoadHDF5Data(const std::string& h5_filename)
{
    hid_t file_id = -1, dataset_id = -1;

    try {
        file_id = H5Fopen(h5_filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
        if (file_id < 0) {
            throw runtime_error("Cannot open HDF5 file");
        }

        // Находим датасет
        const char* data_paths[] = {
            "/data/raw_data",
            "/raw_data",
            "/channel_data",
            "/RFdata",
            "/RcvData"
        };

        bool found = false;
        std::string dataset_path;
        for (const char* path : data_paths) {
            dataset_id = H5Dopen2(file_id, path, H5P_DEFAULT);
            if (dataset_id >= 0) {
                dataset_path = path;
                printf("\n  Loading dataset from: %s", path);
                found = true;
                break;
            }
        }

        if (!found) {
            throw runtime_error("No dataset found in HDF5 file");
        }

        // Получаем размеры
        hid_t space_id = H5Dget_space(dataset_id);
        int ndims = H5Sget_simple_extent_ndims(space_id);
        std::vector<hsize_t> dims(ndims);
        H5Sget_simple_extent_dims(space_id, dims.data(), NULL);

        // Определяем размеры для данных
        size_t n_frames = 1;
        size_t n_transmits = 1;
        size_t n_samples_data = 1;
        size_t n_elements = 1;

        if (ndims >= 4) {
            n_frames = dims[0];
            n_transmits = dims[1];
            n_elements = dims[2];
            n_samples_data = dims[3];
        }
        else if (ndims == 3) {
            n_transmits = dims[0];
            n_samples_data = dims[2];
            n_elements = dims[1];
        }
        else if (ndims == 2) {
            n_samples_data = dims[0];
            n_elements = dims[1];
        }

        printf("\n  Data dimensions: frames=%zu, transmits=%zu, elements=%zu, samples=%zu",
            n_frames, n_transmits, n_elements, n_samples_data);

        // Обновляем параметры
        n_tx_elements = (int)n_transmits;
        n_rx_elements = (int)n_elements;
        samples_per_element = (int)n_samples_data;
        n_samples = (int)n_samples_data;

        // Выделяем память
        data.realloc({ (size_t)n_tx_elements, (size_t)n_rx_elements, (size_t)samples_per_element });
        data.fill(complexF32(0));
        //auto data_raw(data);
        // Читаем данные
        std::vector<int16_t> buffer;
        ReadDatasetGeneric(dataset_id, buffer);
        // Копируем данные в структуру
        size_t samples_to_use = std::min((size_t)samples_per_element, n_samples_data);
        size_t transmits_to_use = std::min((size_t)n_tx_elements, n_transmits);
        size_t elements_to_use = std::min((size_t)n_rx_elements, n_elements);

        size_t j(0), i(0);
        GUIProgressBar progress;
        progress.start("Loading CUBDL data", transmits_to_use * elements_to_use);

        for (size_t tx = 0; tx < transmits_to_use; ++tx) 
        {
            for (size_t rx = 0; rx < elements_to_use; ++rx) 
            {
                for (size_t s = 0; s < samples_to_use; ++s) 
                {
                    if (i < samples_to_use) if (tx < transmits_to_use / 2) i = s + double(tx * inc); else i = s + (transmits_to_use - tx) * inc; else i = 0;

                    //data.at({ tx, rx, s }) = complexF32((float)buffer[j], 0);
                    data.at({ tx, rx, i }) = complexF32((float)buffer[j], 0);
                    j++;
                }
                progress.next();
            }
        }
        progress.end();

        H5Sclose(space_id);

        H5Dclose(dataset_id);
        H5Fclose(file_id);


        //DisplayMathFunction3D(data, "data");

        hilberttransform();
       
    //    DisplayMathFunction3D(data, "data after HilberTransform");


        printf("\n=== CUBDL Data Loaded ===");
        printf("\n  File: %s", filename.c_str());
        printf("\n  Sequence: %s", sequence_number.c_str());
        printf("\n  Institution: %s", metadata.institution.c_str());
        printf("\n  Transmits: %d", n_tx_elements);
        printf("\n  Elements: %d", n_rx_elements);
        printf("\n  Samples: %d", samples_per_element);
        printf("\n  Sample rate: %.2f MHz", raw_signal_sample_rate.MHz());
        printf("\n  Center freq: %.2f MHz", probe_carrier.MHz());
        printf("\n  Sound speed: %.0f m/s", sound_speed.m_sec());
        printf("\n  Pitch: %.3f mm", array_pitch.mm());
        printf("\n  Memory: %.1f MB\n", (double)n_tx_elements * n_rx_elements * samples_per_element * sizeof(complexF32) / 1048576);
        fflush(stdout);
        if (!angles_rad.empty())
        {
            printf("\n  Angle range: %.2f to %.2f degrees",
                angles_rad.front() * 180.0 / 3.14159265358979323846,
                angles_rad.back() * 180.0 / 3.14159265358979323846);
        }
        fflush(stdout);
    }
    catch (const std::exception& e) 
    {
        if (dataset_id >= 0) H5Dclose(dataset_id);
        if (file_id >= 0) H5Fclose(file_id);
        throw runtime_error(string("HDF5 error: ") + e.what());
    }
}

// ============================================================
// ОСНОВНЫЕ МЕТОДЫ
// ============================================================

void RawSFDataSourceCUBDL::Init()
{
    filename = GetFileNameRead("CUBDL HDF5 file", SavedGUIValue("*.hdf5"), "*.hdf5");

    // Парсим sequence ID из имени файла
    size_t pos = filename.find_last_of("/\\");
    std::string basename = (pos != std::string::npos) ? filename.substr(pos + 1) : filename;
    pos = basename.find(".");
    if (pos != std::string::npos) 
    {
        std::string seq_id = basename.substr(0, pos);
        if (seq_id.length() >= 3) 
        {
            sequence_number = seq_id;
            ParseSequenceID(sequence_number);
        }
    }

    LoadHDF5Metadata(filename);

    // Настройка параметров - используем члены базового класса
    raw_signal_sample_rate = Hz(verasonics_sampling_freq);
    hilbert_signal_sample_rate = raw_signal_sample_rate;
    hilbert_samples = samples_per_element;

    
    recommended_angle = degrees(0);
    recommended_n_rays = n_rx_elements;

    probe_carrier = Hz(verasonics_center_freq);
    probe_bandwidth = probe_carrier / 2;
    sound_speed = m_sec(verasonics_sound_speed);

    array_pitch = mm(array_pitch_mm);

    first_raw_sample = 0;
    last_raw_sample = samples_per_element;

    printf("\n\n=== CUBDL Data Source Ready ===");
    printf("\n  File: %s", filename.c_str());
    printf("\n  Sequence: %s", sequence_number.c_str());
    printf("\n  Institution: %s", metadata.institution.c_str());
    printf("\n  Transmits: %d", n_tx_elements);
    printf("\n  Elements: %d", n_rx_elements);
    printf("\n  Samples: %d", samples_per_element);
    printf("\n  Sample rate: %.2f MHz", raw_signal_sample_rate.MHz());
    printf("\n  Center freq: %.2f MHz", probe_carrier.MHz());
    printf("\n  Sound speed: %.0f m/s", sound_speed.m_sec());
    printf("\n  Pitch: %.3f mm", array_pitch.mm());
    printf("\n  Memory: %.1f MB\n",
        (double)n_tx_elements * n_rx_elements * samples_per_element * sizeof(complexF32) / 1048576);
    fflush(stdout);
}

void RawSFDataSourceCUBDL::LoadData()
{
    LoadHDF5Data(filename);
}

string RawSFDataSourceCUBDL::GetFileName() const
{
    return filename;
}

void RawSFDataSourceCUBDL::Display() const
{
    parent::DisplayUtility(data);
}

RawSFDataSourceCUBDL::row_ptr RawSFDataSourceCUBDL::GetRow(size_t tx, size_t rx)
{
    row_t* result = new row_t;

    typename data_volume_type::row_type row;
    data.GetRow(row, { tx, rx, slice_mask(0) });

    result->data.realloc(row.size());
    for (size_t i = 0; i < row.size(); ++i) 
    {
        result->data[i] = row[i];
    }

    return row_ptr(result);
}

// ============================================================
// LEGACY МЕТОД ДЛЯ ОБРАТНОЙ СОВМЕСТИМОСТИ
// ============================================================

void RawSFDataSourceCUBDL::LoadHDF5DataLegacy(const std::string& h5_filename)
{
    // Эта версия использует подход, аналогичный вашему существующему коду
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

        printf("\n  Dataset dimensions: ");
        for (int i = 0; i < ndims; ++i) {
            printf("[%llu]", (unsigned long long)dims[i]);
            if (i < ndims - 1) printf(" x ");
        }
        printf("\n");

        // Для CUBDL данных часто используется layout: [transmit, sample, element]
        if (ndims >= 3) {
            n_tx_elements = (int)dims[0];
            samples_per_element = (int)dims[1];
            n_rx_elements = (int)dims[2];
        }
        else {
            throw runtime_error("Unexpected dataset dimensions");
        }

        // Читаем атрибуты
        hid_t scan_group = H5Gopen2(file_id, "/scan", H5P_DEFAULT);
        if (scan_group >= 0) {
            auto read_attr = [&](const char* name, float& val, float default_val) {
                hid_t attr = H5Aopen(scan_group, name, H5P_DEFAULT);
                if (attr >= 0) {
                    double dval;
                    H5Aread(attr, H5T_NATIVE_DOUBLE, &dval);
                    val = (float)dval;
                    printf("\n  Found attribute: %s = %g", name, val);
                    H5Aclose(attr);
                }
                else {
                    val = default_val;
                }
            };

            read_attr("sampling_frequency", verasonics_sampling_freq, 40e6);
            read_attr("center_frequency", verasonics_center_freq, 7.8e6);
            read_attr("sound_speed", verasonics_sound_speed, 1540.0);
            read_attr("pitch", array_pitch_mm, 0.3);

            H5Gclose(scan_group);
        }

        // Выделяем память
        data.realloc({ (size_t)n_tx_elements, (size_t)n_rx_elements, (size_t)samples_per_element });
        data.fill(complexF32(0));

        // Читаем данные
        std::vector<short> all_data(layout.total_size);
        H5Dread(dataset_id, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT, all_data.data());

        // Копируем данные
        for (size_t tx = 0; tx < (size_t)n_tx_elements; ++tx) {
            for (size_t rx = 0; rx < (size_t)n_rx_elements; ++rx) {
                for (size_t s = 0; s < (size_t)samples_per_element; ++s) {
                    size_t idx = tx * (size_t)samples_per_element * (size_t)n_rx_elements +
                        s * (size_t)n_rx_elements + rx;
                    if (idx < all_data.size()) {
                        data.at({ tx, rx, s }) = complexF32((float)all_data[idx], 0);
                    }
                }
            }
        }

        H5Sclose(space_id);
        H5Dclose(dataset_id);
        H5Fclose(file_id);
    }
    catch (const std::exception& e) {
        if (dataset_id >= 0) H5Dclose(dataset_id);
        if (file_id >= 0) H5Fclose(file_id);
        throw;
    }
}

XRAD_END