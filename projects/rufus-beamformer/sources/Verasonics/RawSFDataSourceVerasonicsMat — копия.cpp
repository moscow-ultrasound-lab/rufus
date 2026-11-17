// RawSFDataSourceVerasonicsMat.cpp
#include "pre.h"
#include "RawSFDataSourceVerasonicsMat.h"
#include <matio.h>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#include <fileapi.h>
#endif

XRAD_BEGIN

// ============================================================
// Вспомогательная функция для открытия .mat файла
// ============================================================
static mat_t* OpenMatFile(const string& filename)
{
    printf("Opening: %s\n", filename.c_str());

    mat_t* mat = Mat_Open(filename.c_str(), MAT_ACC_RDONLY);
    if (mat)
    {
        printf("  File opened successfully\n");
        return mat;
    }

#ifdef _WIN32
    char short_path[MAX_PATH];
    if (GetShortPathNameA(filename.c_str(), short_path, MAX_PATH))
    {
        mat = Mat_Open(short_path, MAT_ACC_RDONLY);
        if (mat)
        {
            printf("  Opened via short path: %s\n", short_path);
            return mat;
        }
    }
#endif

    printf("  Failed to open file\n");
    return NULL;
}

// ============================================================
// ЧТЕНИЕ double поля из структуры (ПУБЛИЧНЫЙ API)
// ============================================================
static bool ReadDoubleField(mat_t* mat, const char* struct_name,
    const char* field_name, double& value)
{
    matvar_t* struct_var = Mat_VarRead(mat, struct_name);
    if (!struct_var)
    {
        return false;
    }

    if (struct_var->class_type != MAT_C_STRUCT)
    {
        Mat_VarFree(struct_var);
        return false;
    }

    matvar_t* field_var = Mat_VarGetStructFieldByName(struct_var, field_name, 0);
    if (!field_var)
    {
        Mat_VarFree(struct_var);
        return false;
    }

    bool ok = false;
    if (field_var->data_type == MAT_T_DOUBLE && field_var->nbytes >= sizeof(double))
    {
        value = ((double*)field_var->data)[0];
        ok = true;
    }
    else if (field_var->data_type == MAT_T_SINGLE && field_var->nbytes >= sizeof(float))
    {
        value = (double)((float*)field_var->data)[0];
        ok = true;
    }
    else if (field_var->data_type == MAT_T_INT16 || field_var->data_type == MAT_T_UINT16)
    {
        value = (double)((int16_t*)field_var->data)[0];
        ok = true;
    }
    else if (field_var->data_type == MAT_T_INT32 || field_var->data_type == MAT_T_UINT32)
    {
        value = (double)((int32_t*)field_var->data)[0];
        ok = true;
    }

    Mat_VarFree(field_var);
    //Mat_VarFree(struct_var);
    return ok;
}

// ============================================================
// ЧТЕНИЕ Resource.Parameters.speedOfSound (ВЛОЖЕННАЯ СТРУКТУРА)
// ============================================================
static bool ReadResourceParametersSpeedOfSound(mat_t* mat, double& value)
{
    // 1. Читаем Resource
    matvar_t* resource_var = Mat_VarRead(mat, "Resource");
    if (!resource_var)
    {
        printf("\n  [DEBUG] Resource: NOT FOUND");
        return false;
    }

    printf("\n  [DEBUG] Resource: class_type=%d", resource_var->class_type);

    if (resource_var->class_type != MAT_C_STRUCT)
    {
        printf(" (not a struct, skipping)");
        Mat_VarFree(resource_var);
        return false;
    }
    printf(" (struct OK)");

    // 2. Читаем поле Parameters из Resource
    matvar_t* params_var = Mat_VarGetStructFieldByName(resource_var, "Parameters", 0);
    if (!params_var)
    {
        printf("\n  [DEBUG] Parameters: NOT FOUND");
        Mat_VarFree(resource_var);
        return false;
    }

    printf("\n  [DEBUG] Parameters: class_type=%d", params_var->class_type);

    if (params_var->class_type != MAT_C_STRUCT)
    {
        printf(" (not a struct, skipping)");
        Mat_VarFree(params_var);
        Mat_VarFree(resource_var);
        return false;
    }
    printf(" (struct OK)");

    // 3. Проверяем, что params_var действительно структура и имеет поля
    // Используем Mat_VarGetSize для проверки
    size_t num_fields = 0;
    // Пробуем получить количество полей через публичный API
    // В matio нет прямого способа, но мы можем попробовать прочитать поле по индексу

    // 4. Читаем поле speedOfSound из Parameters
    printf("\n  [DEBUG] Reading speedOfSound from Parameters...");
    matvar_t* sos_var = Mat_VarGetStructFieldByName(params_var, "speedOfSound", 0);
    if (!sos_var)
    {
        printf(" NOT FOUND");
        Mat_VarFree(params_var);
        Mat_VarFree(resource_var);
        return false;
    }

    printf(" FOUND (data_type=%d, nbytes=%zu)", sos_var->data_type, sos_var->nbytes);

    bool ok = false;
    if (sos_var->data_type == MAT_T_DOUBLE && sos_var->nbytes >= sizeof(double))
    {
        value = ((double*)sos_var->data)[0];
        printf(", value=%.0f (double)", value);
        ok = true;
    }
    else if (sos_var->data_type == MAT_T_SINGLE && sos_var->nbytes >= sizeof(float))
    {
        value = (double)((float*)sos_var->data)[0];
        printf(", value=%.0f (single)", value);
        ok = true;
    }
    else
    {
        printf(" UNSUPPORTED TYPE");
    }

    Mat_VarFree(sos_var);
    Mat_VarFree(params_var);
    Mat_VarFree(resource_var);
    return ok;
}

// ============================================================
// ЧТЕНИЕ поля из Receive(1)
// ============================================================
static bool ReadReceiveField(mat_t* mat, const char* field_name, double& value)
{
    matvar_t* receive_var = Mat_VarRead(mat, "Receive");
    if (!receive_var)
    {
        return false;
    }

    if (receive_var->class_type != MAT_C_STRUCT)
    {
        Mat_VarFree(receive_var);
        return false;
    }

    matvar_t* first_receive = Mat_VarGetStructFieldByName(receive_var, "", 0);
    if (!first_receive)
    {
        Mat_VarFree(receive_var);
        return false;
    }

    matvar_t* field_var = Mat_VarGetStructFieldByName(first_receive, field_name, 0);
    bool ok = false;
    if (field_var)
    {
        if (field_var->data_type == MAT_T_DOUBLE && field_var->nbytes >= sizeof(double))
        {
            value = ((double*)field_var->data)[0];
            ok = true;
        }
        else if (field_var->data_type == MAT_T_SINGLE && field_var->nbytes >= sizeof(float))
        {
            value = (double)((float*)field_var->data)[0];
            ok = true;
        }
        Mat_VarFree(field_var);
    }

    Mat_VarFree(first_receive);
    Mat_VarFree(receive_var);
    return ok;
}

// ============================================================
// Инициализация
// ============================================================

void RawSFDataSourceVerasonicsMat::Init()
{
    filename = GetFileNameRead("Verasonics .mat file (Stanford dataset)",
        SavedGUIValue("*.mat"),
        "*.mat");

    mat_t* mat = OpenMatFile(filename);
    if (!mat)
    {
        throw runtime_error("Cannot open Verasonics .mat file");
    }

    printf("\n\n=== Reading Verasonics .mat metadata ===");

    // ---- Читаем название датчика из Trans.name (UTF-16) ----
    matvar_t* trans_var = Mat_VarRead(mat, "Trans");
    if (trans_var && trans_var->class_type == MAT_C_STRUCT)
    {
        matvar_t* name_var = Mat_VarGetStructFieldByName(trans_var, "name", 0);
        if (name_var)
        {
            if (name_var->data_type == MAT_T_UINT16)
            {
                uint16_t* name_str = (uint16_t*)name_var->data;
                size_t name_len = name_var->nbytes / sizeof(uint16_t);

                while (name_len > 0 && name_str[name_len - 1] == 0)
                    name_len--;

                // СОХРАНЯЕМ НАЗВАНИЕ В ПЕРЕМЕННУЮ
                transducer_name.clear();
                for (size_t i = 0; i < name_len; i++)
                {
                    if (name_str[i] < 128)
                        transducer_name.push_back((char)name_str[i]);
                    else
                        transducer_name.push_back('?');
                }

                printf("\n  Trans.name = '%s'", transducer_name.c_str());
            }
            else
            {
                printf("\n  Trans.name: data_type=%d, nbytes=%zu",
                    name_var->data_type, name_var->nbytes);
            }
            Mat_VarFree(name_var);
        }
       // Mat_VarFree(trans_var);
    }


    // ---- 1. Читаем параметры датчика из Trans ----
    int num_elements = 0;
    double val = 0;
    if (ReadDoubleField(mat, "Trans", "numelements", val))
    {
        num_elements = (int)val;
        printf("\n  Elements = %d", num_elements);
    }
    else
    {
        printf("\n  Trans.numelements: NOT FOUND, using default 256");
        num_elements = 256;
    }

    double pitch_mm = 0;
    if (ReadDoubleField(mat, "Trans", "spacingMm", pitch_mm))
    {
        array_pitch_mm = (float)pitch_mm;
        printf("\n  Pitch = %.4f mm", array_pitch_mm);
    }
    else
    {
        printf("\n  Trans.spacingMm: NOT FOUND, using default 0.1953 mm");
        array_pitch_mm = 0.1953f;
    }

    double freq_mhz = 0;
    if (ReadDoubleField(mat, "Trans", "frequency", freq_mhz))
    {
        verasonics_center_freq = (float)(freq_mhz * 1e6);
        printf("\n  Tx frequency = %.2f MHz", verasonics_center_freq / 1e6);
    }
    else
    {
        printf("\n  Trans.frequency: NOT FOUND, using default 7.5 MHz");
        verasonics_center_freq = 7.5e6f;
    }

    // ---- 2. Задачет начальное значение скорости звука ----
    verasonics_sound_speed = 1540.0f;
    printf("\n  Speed Of Sound = %.0f m/s", verasonics_sound_speed);


    // ---- 3. Читаем частоту дискретизации из Receive ----
    double decim_freq_mhz = 0;
    //if (ReadReceiveField(mat, "decimSampleRate", decim_freq_mhz))
    if (ReadDoubleField(mat, "Receive", "decimSampleRate", decim_freq_mhz))
    {
        verasonics_sampling_freq = (float)(decim_freq_mhz * 1e6);
        printf("\n  Decimated Sample Rate = %.3f MHz", verasonics_sampling_freq / 1e6);
    }
    if (verasonics_sampling_freq == 0)
    {
        printf("\n  Sampling frequency: NOT FOUND, using default 31.25 MHz");
        verasonics_sampling_freq = 31.25e6f;
    }
    double adc_freq_mhz = 0;
    ReadDoubleField(mat, "Receive", "ADCRate", adc_freq_mhz);
    int decim_factor = adc_freq_mhz / decim_freq_mhz;


    double samples;
    ReadDoubleField(mat, "Receive", "endSample", samples);
    samples_per_element = int(samples);
    // samples_per_element /= decim_factor;

     // ---- 4. Читаем размерность данных из RcvData ----
    matvar_t* rcvdata_var = Mat_VarRead(mat, "RcvData");
    if (rcvdata_var)
    {
        if (rcvdata_var->class_type == MAT_C_CELL)
        {
            matvar_t* data_cell = Mat_VarGetCell(rcvdata_var, 0);
            if (data_cell)
            {
                n_samples_per_frame = (int)data_cell->dims[0];
                n_rx_elements = (int)data_cell->dims[1];
                int n_frames = (data_cell->rank >= 3) ? (int)data_cell->dims[2] : 1;

                printf("\n  RcvData{1} dimensions: [%d x %d x %d]",
                    n_samples_per_frame, n_rx_elements, n_frames);

                Mat_VarFree(data_cell);
            }
        }
        //   Mat_VarFree(rcvdata_var);
    }
    else
    {
        printf("\n  RcvData: NOT FOUND");
        n_samples_per_frame = 32768;
        n_rx_elements = 128;
    }

    // ---- 5. Настройка параметров источника ----
    raw_signal_sample_rate = Hz(verasonics_sampling_freq);
    hilbert_signal_sample_rate = raw_signal_sample_rate;

    n_tx_elements = num_elements;
    //samples_per_element = n_samples_per_frame;
    hilbert_samples = samples_per_element;

    is_linear_array = true;
    recommended_angle = degrees(0);
    recommended_n_rays = n_rx_elements;

    probe_carrier = Hz(verasonics_center_freq);
    probe_bandwidth = probe_carrier / 2;
    sound_speed = m_sec(verasonics_sound_speed);

    array_pitch = mm(array_pitch_mm);

    first_raw_sample = 0;
    last_raw_sample = samples_per_element;

    printf("\n\n=== Verasonics .mat Data Source Ready ===");
    printf("\n  File: %s", filename.c_str());
    printf("\n  Transmits (TX): %zu", n_tx_elements);
    printf("\n  Active channels: %zu", n_rx_elements);
    printf("\n  Samples per element: %zu", samples_per_element);
    printf("\n  Sample rate (decimated): %.3f MHz", raw_signal_sample_rate.MHz());
    printf("\n  Center freq: %.2f MHz", probe_carrier.MHz());
    printf("\n  Sound speed: %.0f m/s", sound_speed.m_sec());
    printf("\n  Array pitch: %.4f mm", array_pitch.mm());
    printf("\n  Memory: %.1f MB\n", (double)n_tx_elements * n_rx_elements * samples_per_element * sizeof(complexF32) / 1048576);
    fflush(stdout);

    Mat_Close(mat);
}

// ============================================================
// Преобразование Гильберта (RF -> IQ)
// ============================================================
void RawSFDataSourceVerasonicsMat::HilbertTransform()
{
    size_t n_tx = data.sizes(0);
    size_t n_rx = data.sizes(1);
    size_t n_samples = data.sizes(2);

    printf("\n  Performing Hilbert transform (RF -> IQ)...");
    fflush(stdout);

    GUIProgressBar hilbert_progress;
    hilbert_progress.start("Hilbert transform", n_tx * n_rx);

    for (size_t tx = 0; tx < n_tx; ++tx)
    {
        for (size_t rx = 0; rx < n_rx; ++rx)
        {
            ComplexFunctionF32 rf_signal(n_samples);
            for (size_t s = 0; s < n_samples; ++s)
            {
                rf_signal[s] = complexF32(data.at({ tx, rx, s }).re, 0);
            }

            size_t fft_size = ceil_fft_length(n_samples);
            ComplexFunctionF32 buffer(fft_size, complexF32(0));

            for (size_t s = 0; s < n_samples; ++s)
            {
                buffer[s] = rf_signal[s];
            }

            FFTf(buffer, fftFwd);

            // Аналитический сигнал
            for (size_t i = 1; i < fft_size / 2; ++i)
            {
                buffer[i] *= 2.0f;
            }
            for (size_t i = fft_size / 2 + 1; i < fft_size; ++i)
            {
                buffer[i] = 0;
            }
            buffer[0] /= 2.0f;
            if (fft_size % 2 == 0)
            {
                buffer[fft_size / 2] /= 2.0f;
            }

            FFTf(buffer, fftRev);

            for (size_t s = 0; s < n_samples; ++s)
            {
                data.at({ tx, rx, s }) = buffer[s];
            }

            hilbert_progress.next();
        }
    }
    hilbert_progress.end();

    printf(" Done.\n");
    fflush(stdout);
}

// ============================================================
// Загрузка данных
// ============================================================

void RawSFDataSourceVerasonicsMat::LoadMatData(const std::string& filename)
{
    mat_t* mat = OpenMatFile(filename);
    if (!mat)
    {
        throw runtime_error("Cannot open Verasonics .mat file for data loading");
    }

    // ---- 1. Читаем RcvData ----
    matvar_t* rcvdata_var = Mat_VarRead(mat, "RcvData");
    if (!rcvdata_var)
    {
        Mat_Close(mat);
        throw runtime_error("RcvData not found");
    }

    if (rcvdata_var->class_type != MAT_C_CELL)
    {
        Mat_VarFree(rcvdata_var);
        Mat_Close(mat);
        throw runtime_error("RcvData is not a cell array");
    }

    matvar_t* data_var = Mat_VarGetCell(rcvdata_var, 0);
    if (!data_var)
    {
        Mat_VarFree(rcvdata_var);
        Mat_Close(mat);
        throw runtime_error("RcvData{1} not found");
    }

    size_t n_samples = data_var->dims[0];
    size_t n_channels = data_var->dims[1];
    size_t n_frames = (data_var->rank >= 3) ? data_var->dims[2] : 1;

    printf("\n  Loading data: %zu samples x %zu channels x %zu frames",
        n_samples, n_channels, n_frames);
    fflush(stdout);

    // Проверяем тип данных
    bool is_int16 = (data_var->data_type == MAT_T_INT16 || data_var->data_type == MAT_T_UINT16);
    bool is_int8 = (data_var->data_type == MAT_T_INT8 || data_var->data_type == MAT_T_UINT8);
    bool is_double = (data_var->data_type == MAT_T_DOUBLE);
    bool is_single = (data_var->data_type == MAT_T_SINGLE);

    if (!is_int16 && !is_int8 && !is_double && !is_single)
    {
        Mat_VarFree(data_var);
        Mat_VarFree(rcvdata_var);
        Mat_Close(mat);
        throw runtime_error(ssprintf("Unsupported data type: %d", data_var->data_type));
    }

    // ---- 2. Выделяем память для данных ----
    size_t tx_count = n_tx_elements;
    size_t rx_count = n_tx_elements;
    n_samples = samples_per_element;
    //n_samples = 2944;

    //data.realloc({ tx_count, rx_count, n_samples });
    data.realloc({ tx_count, rx_count, n_samples });
    
    data.fill(complexF32(0));

    //size_t j = 0;
    float val(0);
    size_t index_scr(0);
    //int16_t* src = (int16_t*)data_var->data + samples_per_frame;
    int16_t* src = (int16_t*)data_var->data;
    size_t n_samples_original = n_samples;
   
    size_t count(0);
    size_t confidence_interval(100);
    //if ((transducer_name.find("L12-5") != std::string::npos) || (transducer_name.find("L12-3v") != std::string::npos))
    {
        for (size_t s = 0; s < n_samples; ++s)
        {
            if ((float)src[s] == 0)
            {
                for (size_t j = 0; j < confidence_interval; ++j)
                {

                    if ((float)src[s + j] == 0)
                    {
                        count++;
                    }
                    else
                    {
                        count = 0;
                    }

                }
                if (count > confidence_interval - 1)
                {
                    n_samples = s;
                }
            }
        }
        data.realloc({ tx_count, rx_count, n_samples });
        data.fill(complexF32(0));
        printf("\n  n_samples: %zu", n_samples);
    }
        
    size_t n(1);
    if ((transducer_name.find("L12-3v") != std::string::npos))
    {
        n = 2;
    }
    data.realloc({ n * tx_count, rx_count, n_samples });
    //data_volume_type d1(data), d2(data), d3(data), d4(data), d5(data), d6(data), data_full(data);

    // ---- 3. Копируем данные ----
    GUIProgressBar progress;
    progress.start("Loading Verasonics .mat data", data.sizes(0)* data.sizes(1));
    for (size_t tx = 0; tx < data.sizes(0); ++tx)
    {
        if (!(tx == 0) && (tx % 2 == 0))
        {
            if (transducer_name.find("L12-5") != std::string::npos)
            {
                index_scr += 7 * n_samples_original * n_channels + 16 * n_samples_original + 2048;
            }
            else  if (transducer_name.find("L12-3v") != std::string::npos)
            {
                //if (tx == 128) index_scr += 224 * (0.5 * (4 * n_samples_original * tx_count + 76 * n_samples_original + 2048) + (n_samples_original * tx_count));
                //else
                {
                    index_scr += 4 * n_samples_original * tx_count + 76 * n_samples_original + 2048;
                }
            }
        }

        if (!(tx == 0) && transducer_name.find("C5-2v") != std::string::npos)
        {
            index_scr += 87 * n_samples_original + 1408;
        }
        for (size_t rx = 0; rx < data.sizes(1); ++rx)
        {
            for (size_t s = 0; s < data.sizes(2); ++s)
            {
                val = (float)src[index_scr];
                if ((transducer_name.find("L12-5") != std::string::npos) || (transducer_name.find("L12-3v") != std::string::npos))
                {
                    //data_full.at({ tx, rx, s }) = complexF32(val, 0);
                    if (rx % 2 == 0)
                    {
                        if (tx % 2 == 0)
                        {
                            data.at({ size_t(tx / 2), size_t(rx / 2), s }) = complexF32(val, 0);
                            //d1.at({ size_t(tx / 2), size_t(rx / 2), s }) = complexF32(val, 0);
                            //d5.at({ size_t(tx / 2), rx, s }) = complexF32(val, 0);
                        }
                        else
                        {
                            data.at({ size_t(tx / 2), data.sizes(1)/2 + size_t(rx / 2), s }) = complexF32(val, 0);
                            //d2.at({ size_t(tx / 2), size_t(rx / 2), s }) = complexF32(val, 0);
                            //d6.at({ size_t(tx / 2), rx, s }) = complexF32(val, 0);
                        }
                        
                    }
                    else
                    {
                        if (tx % 2 == 0)
                        {
                            data.at({ data.sizes(0)/2 + size_t(tx / 2), size_t(rx / 2), s }) = complexF32(val, 0);
                            //d3.at({ size_t(tx / 2), size_t(rx / 2), s }) = complexF32(val, 0);
                            //d5.at({ size_t(tx / 2), rx, s }) = complexF32(val, 0);
                        }
                        else
                        {
                            data.at({ data.sizes(0)/2 + size_t(tx / 2), data.sizes(1)/2 + size_t(rx / 2), s }) = complexF32(val, 0);
                            //d4.at({ size_t(tx / 2), size_t(rx / 2), s }) = complexF32(val, 0);
                            //d6.at({ size_t(tx / 2), rx, s }) = complexF32(val, 0);
                        }
                    }
                }
                
                else
                {
                    data.at({ tx, rx, s}) = complexF32(val, 0);
                }
                index_scr++;
            }
            index_scr += n_samples_original - n_samples;
            progress.next();
        }
    }

    progress.end();


    if (transducer_name.find("L12-3v") != std::string::npos)
    {
        data_volume_type data_buffer(data);
        data.realloc({ tx_count, rx_count, n_samples });
        for (size_t tx = 0; tx < n_tx_elements; ++tx)
        {
            if (tx < n_channels)
            {
                data.GetSlice({ tx, slice_mask(0), slice_mask(1) }).CopyData(data_buffer.GetSlice({ tx, slice_mask(0), slice_mask(1) }));
            }
            else
            {
                data.GetSlice({ tx, slice_mask(0), slice_mask(1) }).CopyData(data_buffer.GetSlice({ tx_count + tx, slice_mask(0), slice_mask(1) }));
            }
        }
        
    }


    Mat_VarFree(data_var);
    Mat_Close(mat);


    printf("\n  Data loaded: %zu x %zu x %zu", tx_count, rx_count, n_samples);
    fflush(stdout);

    DisplayMathFunction3D(data, "data");
    
    // ---- 4. Преобразование Гильберта ----
    HilbertTransform();
    DisplayMathFunction3D(data, "HilbertTransform");

}

// ============================================================
// Реализация виртуальных методов
// ============================================================

void RawSFDataSourceVerasonicsMat::LoadData()
{
    LoadMatData(filename);
}

string RawSFDataSourceVerasonicsMat::GetFileName() const
{
    return filename;
}

void RawSFDataSourceVerasonicsMat::Display() const
{
    parent::DisplayUtility(data);
}

RawSFDataSourceVerasonicsMat::row_ptr RawSFDataSourceVerasonicsMat::GetRow(size_t tx, size_t rx)
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

XRAD_END