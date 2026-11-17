// RawSFDataSourceUlaOp.cpp
#include "pre.h"
#include "RawSFDataSourceUlaOp.h"
#include <sstream>
#include <algorithm>
#include <cmath>
#include "SignalProcessing/GaussSignalTools.h"

XRAD_BEGIN

// Вспомогательные методы
bool RawSFDataSourceUlaOp::IsSyntheticApertureTransmit(int tx_index) const
{
    return (tx_index < (int)n_rx_elements);
}

bool RawSFDataSourceUlaOp::IsPlaneWaveTransmit(int tx_index) const
{
    return !IsSyntheticApertureTransmit(tx_index);
}

void RawSFDataSourceUlaOp::DiagnoseFile(const std::string& filename)
{
    printf("\n=== DIAGNOSTIC: Analyzing file ===\n");

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        printf("  Cannot open file!\n");
        return;
    }

    // Размер файла
    file.seekg(0, std::ios::end);
    size_t file_size = (size_t)file.tellg();
    file.seekg(0, std::ios::beg);
    printf("  File size: %zu bytes\n", file_size);

    // Параметры
    size_t n_gates = 8192;
    size_t n_elements = 192;
    size_t n_transmits = 288;

    // Варианты ожидаемых размеров
    size_t size_12bit = n_gates * n_elements * n_transmits * 3 / 2;  // 12-bit packed
    size_t size_16bit = n_gates * n_elements * n_transmits * 2;      // 16-bit unpacked
    size_t size_8bit = n_gates * n_elements * n_transmits;           // 8-bit (маловероятно)

    printf("  Expected 12-bit packed: %zu bytes\n", size_12bit);
    printf("  Expected 16-bit unpacked: %zu bytes\n", size_16bit);
    printf("  Expected 8-bit: %zu bytes\n", size_8bit);

    // Проверяем соотношения
    if (file_size == size_12bit) {
        printf("  RESULT: File is 12-bit packed data\n");
    }
    else if (file_size == size_16bit) {
        printf("  RESULT: File is 16-bit unpacked data\n");
    }
    else if (file_size == size_8bit) {
        printf("  RESULT: File is 8-bit data\n");
    }
    else {
        // Проверяем другие возможные комбинации
        printf("\n  Trying other combinations:\n");

        // Возможные gates: 8192, 4096, 2048, 1024, 512
        size_t possible_gates[] = { 8192, 4096, 2048, 1024, 512, 256, 128 };
        size_t possible_elements[] = { 192, 128, 96, 64 };
        size_t possible_transmits[] = { 288, 384, 192, 96, 48 };

        for (size_t g : possible_gates) {
            for (size_t e : possible_elements) {
                for (size_t t : possible_transmits) {
                    size_t test_size = g * e * t * 2;
                    if (test_size == file_size) {
                        printf("    Found: gates=%zu, elements=%zu, transmits=%zu (16-bit)\n", g, e, t);
                    }
                    test_size = g * e * t * 3 / 2;
                    if (test_size == file_size) {
                        printf("    Found: gates=%zu, elements=%zu, transmits=%zu (12-bit)\n", g, e, t);
                    }
                }
            }
        }
    }

    // Смотрим первые байты для определения формата
    std::vector<uint8_t> first_bytes(100);
    file.read(reinterpret_cast<char*>(first_bytes.data()), 100);

    printf("\n  First 20 bytes (hex): ");
    for (int i = 0; i < 20; ++i) {
        printf("%02X ", first_bytes[i]);
    }
    printf("\n");
    printf("  First 20 bytes (dec): ");
    for (int i = 0; i < 20; ++i) {
        printf("%d ", first_bytes[i]);
    }
    printf("\n");

    file.close();
    fflush(stdout);
}

// ============================================================
// Чтение ULA-OP .uop файла (текстовый формат)
// ============================================================

void RawSFDataSourceUlaOp::ParseUopFile(const std::string& filename)
{
    //printf("\n=== Parsing UOP file: %s ===\n", filename.c_str());
    printf("\n%s\n", filename.c_str());

    printf("\n=== Parsing UOP file ===\n");
    fflush(stdout);

    std::ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error(ssprintf("Cannot open ULA-OP parameters file: %s", filename.c_str()));
    }

    std::string line;
    UlaOpParameters params;
    params.elements_number = 192;
    params.elements_pitch = 245e-6;
    params.sound_speed = 1540.0;
    params.sampling_freq = 78.125e6;  // Правильная частота!
    params.center_freq = 8e6;
    params.samples_per_line = 8192;
    params.n_lines = 288;
    params.active_elements = 192;
    params.max_voltage = 45;

    bool in_working_set = false;

    while (std::getline(file, line)) {
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        line = line.substr(start);

        size_t end = line.find_last_not_of(" \t\r\n");
        if (end != std::string::npos) {
            line = line.substr(0, end + 1);
        }

        if (line.empty() || line[0] == '/') continue;

        if (line[0] == '[') {
            in_working_set = (line == "[WORKINGSET]");
            continue;
        }
        
        
        //---------------------------------

        if (in_working_set) {
            size_t eq_pos = line.find('=');
            if (eq_pos != std::string::npos) {
                std::string key = line.substr(0, eq_pos);
                std::string value = line.substr(eq_pos + 1);

                key.erase(0, key.find_first_not_of(" \t\r\n"));
                key.erase(key.find_last_not_of(" \t\r\n") + 1);
                value.erase(0, value.find_first_not_of(" \t\r\n"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);

                if (key == "SoundSpeed") {
                    params.sound_speed = std::stod(value);
                    printf("  SoundSpeed: %.0f m/s\n", params.sound_speed);
                }
                else if (key == "ProbeParameters") {
                    // ============================================================
                    // ГИБКИЙ ПАРСИНГ ProbeParameters
                    // ============================================================

                    // Удаляем все пробелы для упрощения
                    std::string clean_value = value;
                    clean_value.erase(std::remove_if(clean_value.begin(), clean_value.end(), ::isspace), clean_value.end());

                    // Пробуем распарсить как "elements number X, elements pitch Y, ..."
                    size_t pos = value.find("elements number");
                    if (pos != std::string::npos) {
                        // Старый формат с текстовыми метками
                        pos = value.find("number");
                        if (pos != std::string::npos) {
                            std::string num_str;
                            pos += 7;
                            while (pos < value.length() && (isdigit(value[pos]) || value[pos] == '.')) {
                                num_str += value[pos++];
                            }
                            if (!num_str.empty()) {
                                params.elements_number = std::stoi(num_str);
                                printf("  Elements: %d\n", params.elements_number);
                            }
                        }
                        pos = value.find("pitch");
                        if (pos != std::string::npos) {
                            std::string pitch_str;
                            pos += 6;
                            while (pos < value.length() && (isdigit(value[pos]) || value[pos] == '.')) {
                                pitch_str += value[pos++];
                            }
                            if (!pitch_str.empty()) {
                                double pitch_val = std::stod(pitch_str);
                                params.elements_pitch = pitch_val * 1e-7;
                                printf("  Pitch: %.3f mm\n", params.elements_pitch * 1000);
                            }
                        }
                    }
                    else {
                        // Новый формат: просто числа через запятую: "256, 1250, 0, 0"
                        // Формат: elements_number, elements_pitch, start_element, probe_delay
                        std::vector<double> numbers;
                        std::stringstream ss(clean_value);
                        std::string token;

                        while (std::getline(ss, token, ',')) {
                            if (!token.empty()) {
                                try {
                                    numbers.push_back(std::stod(token));
                                }
                                catch (...) {
                                    // Если не число, пропускаем
                                }
                            }
                        }

                        if (numbers.size() >= 4) {
                            params.elements_number = (int)numbers[0];
                            // Внимание: в новом формате pitch может быть в других единицах!
                            // Проверяем значение: если pitch > 100, то это скорее всего в 1e-7 м,
                            // иначе это может быть в мм
                            if (numbers[1] > 100) {
                                // Похоже на формат 1250 (1e-7 м) -> 0.125 мм
                                params.elements_pitch = numbers[1] * 1e-7;
                            }
                            else if (numbers[1] > 1) {
                                // Возможно, pitch в мм
                                params.elements_pitch = numbers[1] / 1000.0;
                            }
                            else {
                                // Pitch уже в метрах
                                params.elements_pitch = numbers[1];
                            }
                            params.start_element = (int)numbers[2];
                            params.probe_delay = numbers[3];

                            printf("  Elements: %d\n", params.elements_number);
                            printf("  Pitch: %.3f mm\n", params.elements_pitch * 1000);
                            printf("  Start element: %d\n", params.start_element);
                            printf("  Probe delay: %.2f\n", params.probe_delay);
                        }
                        else if (numbers.size() >= 2) {
                            // Минимальный набор: только elements и pitch
                            params.elements_number = (int)numbers[0];
                            if (numbers[1] > 100) {
                                params.elements_pitch = numbers[1] * 1e-7;
                            }
                            else if (numbers[1] > 1) {
                                params.elements_pitch = numbers[1] / 1000.0;
                            }
                            else {
                                params.elements_pitch = numbers[1];
                            }
                            printf("  Elements: %d\n", params.elements_number);
                            printf("  Pitch: %.3f mm\n", params.elements_pitch * 1000);
                        }
                        else {
                            printf("  WARNING: Could not parse ProbeParameters: %s\n", value.c_str());
                        }
                    }
                }
                else if (key == "MaxWorkingVoltage") {
                    params.max_voltage = std::stoi(value);
                }
            }
        }

        //---------------------------------

/*
        if (in_working_set) {
            size_t eq_pos = line.find('=');
            if (eq_pos != std::string::npos) {
                std::string key = line.substr(0, eq_pos);
                std::string value = line.substr(eq_pos + 1);

                key.erase(0, key.find_first_not_of(" \t\r\n"));
                key.erase(key.find_last_not_of(" \t\r\n") + 1);
                value.erase(0, value.find_first_not_of(" \t\r\n"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);

                if (key == "SoundSpeed") 
                {
                    params.sound_speed = std::stod(value);
                    printf("  SoundSpeed: %.0f m/s\n", params.sound_speed);
                }
                else if (key == "ProbeParameters") 
                {
                    size_t pos = value.find("number");
                    if (pos != std::string::npos) 
                    {
                        std::string num_str;
                        pos += 7;
                        while (pos < value.length() && isdigit(value[pos])) 
                        {
                            num_str += value[pos++];
                        }
                        if (!num_str.empty()) 
                        {
                            params.elements_number = std::stoi(num_str);
                            printf("  Elements: %d\n", params.elements_number);
                        }
                    }
                    pos = value.find("pitch");
                    if (pos != std::string::npos) {
                        std::string pitch_str;
                        pos += 6;
                        while (pos < value.length() && (isdigit(value[pos]) || value[pos] == '.')) {
                            pitch_str += value[pos++];
                        }
                        if (!pitch_str.empty()) {
                            double pitch_val = std::stod(pitch_str);
                            params.elements_pitch = pitch_val * 1e-7;
                            printf("  Pitch: %.3f mm\n", params.elements_pitch * 1000);
                        }
                    }
                }
                else if (key == "MaxWorkingVoltage") {
                    params.max_voltage = std::stoi(value);
                }
            }
        }

*/
        //--------------------------------------------


        // Ищем параметры ACQUIRERF_PRE
        if (line.find("SliceRf") != std::string::npos) {
            std::string slice_name, temp;
            int n_samples = 0, n_blocks = 0, offset = 0;
            std::string type_name;

            std::istringstream iss(line);
            iss >> slice_name;
            iss >> temp;
            if (temp == "=") {
                iss >> n_samples;
                iss >> temp;
                iss >> n_blocks;
                iss >> temp;
                iss >> offset;
                iss >> temp;
                iss >> type_name;

                if (n_samples > 0) {
                    params.samples_per_line = n_samples;
                    printf("  Samples per line: %d\n", params.samples_per_line);
                }
                if (n_blocks > 0) {
                    params.n_blocks = n_blocks;
                    printf("  Blocks: %d\n", params.n_blocks);
                }
            }
        }

        if (line.find("DemodulationFrequency") != std::string::npos) {
            std::istringstream iss(line);
            std::string key, equals;
            double value;
            int dummy;
            iss >> key >> equals >> value >> dummy;
            if (equals == "=" && value > 0) {
                params.center_freq = value;
                printf("  Center frequency: %.2f MHz\n", value / 1e6);
            }
        }
    }

    file.close();
    ulaop_params = params;

    //printf("\n=== ULA-OP Parameters ===\n");
    //printf("  Elements: %d\n", params.elements_number);
    //printf("  Pitch: %.4f mm\n", params.elements_pitch * 1000);
    //printf("  Sound speed: %.0f m/s\n", params.sound_speed);
    //printf("  Center freq: %.2f MHz\n", params.center_freq / 1e6);
    printf("  Sampling freq: %.2f MHz\n", params.sampling_freq / 1e6);
    //printf("  Samples per line: %d\n", params.samples_per_line);
    //printf("  Blocks: %d\n", params.n_blocks);
    fflush(stdout);
}

// ============================================================
// Чтение .uos файла
// ============================================================

void RawSFDataSourceUlaOp::ParseUosFile(const std::string& filename)
{
    //printf("\n=== Parsing UOS file: %s ===\n", filename.c_str());
    printf("\n=== Parsing UOS file ===\n");
    fflush(stdout);

    std::ifstream file(filename);
    if (!file.is_open()) {
        printf("  UOS file not found, using defaults\n");
        // Используем значения по умолчанию
        ulaop_params.block_length = 8192;
        ulaop_params.n_blocks = 384;
        rff_info.NBoards = 6;
        return;
    }

    std::string line;
    bool in_info = false;
    long long total_size = 0;
    int n_blocks = 0;
    int block_length = 0;
    int block_size = 0;
    int n_boards = 0;

    while (std::getline(file, line)) {
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        line = line.substr(start);

        if (line.find("[Info]") != std::string::npos) {
            in_info = true;
            continue;
        }
        if (!in_info) continue;
        if (line.empty() || line[0] == '[') break;

        size_t eq_pos = line.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = line.substr(0, eq_pos);
            std::string value = line.substr(eq_pos + 1);

            key.erase(0, key.find_first_not_of(" \t\r\n"));
            key.erase(key.find_last_not_of(" \t\r\n") + 1);
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);

            if (key == "TotalSize") {
                total_size = std::stoll(value);
                printf("  TotalSize: %lld\n", total_size);
            }
            else if (key == "NBlocks") {
                n_blocks = std::stoi(value);
                printf("  NBlocks: %d\n", n_blocks);
            }
            else if (key == "BlockLength") {
                block_length = std::stoi(value);
                printf("  BlockLength: %d\n", block_length);
            }
            else if (key == "BlockSize") {
                block_size = std::stoi(value);
                printf("  BlockSize: %d\n", block_size);
            }
            else if (key == "NBoards") {
                n_boards = std::stoi(value);
                printf("  NBoards: %d\n", n_boards);
            }
        }
    }
    file.close();

    if (total_size > 0) {
        ulaop_params.total_size = total_size;
        ulaop_params.n_blocks = n_blocks;
        ulaop_params.block_length = block_length;
        rff_info.NBoards = n_boards;
    }
    else {
        // Значения по умолчанию
        ulaop_params.block_length = 8192;
        ulaop_params.n_blocks = 384;
        rff_info.NBoards = 6;
    }
}

// ============================================================
// Распаковка 12-битных данных из 8-битных
// ============================================================

void RawSFDataSourceUlaOp::Unpack12BitData(const std::vector<uint8_t>& packed_data,
    std::vector<int16_t>& unpacked_data,
    size_t n_gates,
    size_t n_elements_32,
    size_t n_transmits)
{
    // Формат: 48 байт = 16 элементов * 3 байта (2x12-bit)
    // Первые 32 байта: младшие 8 бит
    // Байты 33-48: старшие 4 бита (по 2 элемента в байте)

    size_t elements_per_block = 16;
    size_t bytes_per_block = 48;  // 16 * 3

    unpacked_data.resize(n_gates * n_elements_32 * n_transmits);

    for (size_t t = 0; t < n_transmits; ++t) {
        for (size_t g = 0; g < n_gates; ++g) {
            for (size_t e16 = 0; e16 < elements_per_block; ++e16) {
                size_t base_idx = t * n_gates * bytes_per_block + g * bytes_per_block + e16 * 3;

                uint8_t b0 = packed_data[base_idx];
                uint8_t b1 = packed_data[base_idx + 1];
                uint8_t b2 = packed_data[base_idx + 2];

                // Первый 12-битный элемент: b0 + (b1 & 0x0F) << 8
                int16_t val1 = (int16_t)(b0 | ((b1 & 0x0F) << 8));
                // Второй 12-битный элемент: b2 + (b1 >> 4) << 8
                int16_t val2 = (int16_t)(b2 | ((b1 >> 4) << 8));

                size_t idx1 = t * n_gates * n_elements_32 + g * n_elements_32 + e16 * 2;
                size_t idx2 = idx1 + 1;

                unpacked_data[idx1] = val1;
                unpacked_data[idx2] = val2;
            }
        }
    }
}

// ============================================================
// FIR фильтр (как в MATLAB: порядок 81, полоса 1.87-14.19 МГц)
// ============================================================

void RawSFDataSourceUlaOp::ApplyFIRFilter(ComplexFunctionF32& signal)
{
    size_t n = signal.size();
    if (n == 0) return;

    // Параметры из MATLAB
    double fs = 78.125e6;      // 78.125 МГц
    double fc_low = 1.87e6;    // 1.87 МГц
    double fc_high = 14.19e6;  // 14.19 МГц
    int order = 81;

    size_t fft_size = 1;
    while (fft_size < n + order) fft_size <<= 1;

    ComplexFunctionF32 buffer(fft_size, complexF32(0));
    for (size_t i = 0; i < n; ++i) {
        buffer[i] = signal[i];
    }

    FFTf(buffer, fftFwd);

    double freq_step = fs / fft_size;
    double transition = 0.5e6;  // 0.5 МГц переходная зона

    for (size_t i = 0; i < fft_size; ++i) {
        double freq = (i <= fft_size / 2) ? i * freq_step : (fft_size - i) * freq_step;
        double gain = 1.0;

        // Полосовой фильтр с плавными переходами
        if (freq < fc_low || freq > fc_high) {
            gain = 0.0;
        }

        // Плавный спад на нижней границе
        if (freq > fc_low - transition && freq < fc_low) {
            gain = 0.5 * (1 + cos(pi() * (freq - (fc_low - transition)) / transition));
        }
        // Плавный спад на верхней границе
        if (freq > fc_high && freq < fc_high + transition) {
            gain = 0.5 * (1 + cos(pi() * (freq - fc_high) / transition));
        }

        buffer[i] *= (float)gain;
    }

    FFTf(buffer, fftRev);

    int delay = order / 2;
    for (size_t i = 0; i < n; ++i) {
        if (i + delay < n) {
            signal[i] = buffer[i + delay];
        }
        else {
            signal[i] = complexF32(0, 0);
        }
    }
}

// ============================================================
// Чтение данных из .rff256 файла (ПРАВИЛЬНАЯ ВЕРСИЯ!)
// ============================================================

void RawSFDataSourceUlaOp::Unpack12BitDataSimple(const std::vector<uint8_t>& packed_data,
    std::vector<int16_t>& unpacked_data,
    size_t n_gates,
    size_t n_elements,
    size_t n_transmits)
{
    // Упрощенная распаковка: каждый 12-битный элемент занимает 3 байта
    // 2 элемента = 3 байта (12 бит + 12 бит = 24 бита = 3 байта)

    size_t elements_per_block = 2;
    size_t bytes_per_block = 3;
    size_t total_elements = n_gates * n_elements * n_transmits;

    unpacked_data.resize(total_elements);

    size_t idx = 0;
    for (size_t t = 0; t < n_transmits; ++t) {
        for (size_t g = 0; g < n_gates; ++g) {
            for (size_t e = 0; e < n_elements; e += 2) {
                size_t base_idx = t * n_gates * (n_elements * 3 / 2) + g * (n_elements * 3 / 2) + (e / 2) * 3;

                if (base_idx + 2 >= packed_data.size()) break;

                uint8_t b0 = packed_data[base_idx];
                uint8_t b1 = packed_data[base_idx + 1];
                uint8_t b2 = packed_data[base_idx + 2];

                // Первый 12-битный элемент: b0 + (b1 & 0x0F) << 8
                int16_t val1 = (int16_t)(b0 | ((b1 & 0x0F) << 8));
                // Второй 12-битный элемент: b2 + (b1 >> 4) << 8
                int16_t val2 = (int16_t)(b2 | ((b1 >> 4) << 8));

                unpacked_data[idx++] = val1;
                if (idx < total_elements) {
                    unpacked_data[idx++] = val2;
                }
            }
        }
    }

    printf("  Unpacked %zu samples\n", idx);
}

// ============================================================
// Чтение данных из .rff256 файла
// ============================================================

void RawSFDataSourceUlaOp::ReadRffData(const std::string& filename)
{
    printf("\n=== Reading RF data ===\n");
    fflush(stdout);

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw runtime_error(ssprintf("Cannot open RF file: %s", filename.c_str()));
    }

    /*
    // Размеры из диагностики
    size_t nGates = 8192;
    size_t nElements = 192;
    size_t nTransmits = 288;
    */

    size_t nGates = ulaop_params.samples_per_line;
    size_t nElements = ulaop_params.elements_number;
    size_t nTransmits = ulaop_params.elements_number;
    nTransmits = 288;

    //printf("  Gates: %zu\n", nGates);
    //printf("  Elements: %zu\n", nElements);
    //printf("  Transmits: %zu\n", nTransmits);

    // Читаем как int16
    size_t total_samples = nGates * nElements * nTransmits;
    std::vector<int16_t> raw_data(total_samples);

    //file.read(reinterpret_cast<char*>(raw_data.data()), total_samples * sizeof(int16_t));
    file.read(reinterpret_cast<char*>(raw_data.data()), total_samples * sizeof(int8_t));
    file.close();

    printf("  Read %zu samples\n", total_samples);

    // Нормализация
    float max_val = 0;
    for (size_t i = 0; i < total_samples; ++i) {
        float val = std::abs((float)raw_data[i]);
        if (val > max_val) max_val = val;
    }
    if (max_val == 0) max_val = 1.0f;
   // printf("  Max value: %.3f\n", max_val);

    // Используем только синтетическую апертуру (первые 192 transmits)
    size_t nTransmitsToUse = nElements;

    // Перевыделяем данные: [gates x elements x transmits]
    data.realloc({ nTransmitsToUse, nElements, nGates / 2 });
    
    
    data.fill(complexF32(0));

    ComplexFunctionMD_F32 original_data(data);
    size_t n_chennels = 8;
    size_t n_chennel_groups = 3;
    size_t n_ative_chennel_groups = 2;
    size_t tx_no = 0;
    size_t frame_no = 0;
    //int n_sets = GetFloating("n_sets", MakeGUIValue(1, saved_default_value), 1, nElements * nTransmits); 
    int n_sets = nElements * nTransmitsToUse / (n_chennels * n_ative_chennel_groups);
    
    
    printf("  %zu x %zu / (%zu x %zu) , %zu\n", nElements, nTransmitsToUse, n_chennels, n_ative_chennel_groups, nGates);

    GUIProgressBar read_progress;
    read_progress.start("Reading raw data", n_sets);

    for (size_t set_no = 0; set_no < n_sets; ++set_no)
    {
        for (size_t pulse_no = 0; pulse_no < (n_chennels * n_ative_chennel_groups); ++pulse_no)
        {
            for (size_t sample_no = 0; sample_no < nGates / 2; ++sample_no)
            {
                size_t src_idx = n_chennels * n_chennel_groups * (sample_no + set_no * nGates / 2) + pulse_no;
                {
                    original_data.at({ frame_no, tx_no , sample_no }) = (float)raw_data[src_idx];
                }
            }
            tx_no++;
            if (tx_no == nElements)
            {
                tx_no = 0;
                frame_no++;
            }
        }
        read_progress.next();
    }
    read_progress.end();


    DisplayUtility(original_data);
    // меняем местами первую и нулевую группу
    ComplexFunctionMD_F32 rearranged_data(original_data);
    size_t element_no(0);


    if(ulaop_params.elements_number == 192)
    {
        GUIProgressBar repack_progress;
        repack_progress.start("Switching first and second boards", original_data.sizes(0));

        for (size_t frame_no = 0; frame_no < original_data.sizes(0); ++frame_no)
        {
            for (size_t tx_no = 0; tx_no < nElements / 3; ++tx_no)
            {
                if (tx_no < nElements / 6)
                {
                    element_no = tx_no + nElements / 6;
                }
                else
                {
                    element_no = tx_no - nElements / 6;
                }
                for (size_t sample_no = 0; sample_no < nGates / 2; ++sample_no)
                {
                    {
                        rearranged_data.at({ frame_no, element_no, sample_no }) = original_data.at({ frame_no, tx_no , sample_no });
                    }
                }

            }
            repack_progress.next();
        }
        repack_progress.end();
    }

    // каждый 8-ой импульс кладем рядом
    //ComplexFunctionMD_F32 rerearranged_data(rearranged_data);
    //size_t element_no(0);
    GUIProgressBar rearrange_progress;
    rearrange_progress.start("Rearranging raw data", data.sizes(0));

    for (size_t frame_no = 0; frame_no < data.sizes(0); ++frame_no)
    {
        for (size_t block_no = 0; block_no < nElements / (n_ative_chennel_groups * n_chennels); ++block_no)
        {

            for (size_t pulse_no = 0; pulse_no < n_ative_chennel_groups * n_chennels; ++pulse_no)
            {
                if (pulse_no < n_chennels)
                {
                    element_no = n_ative_chennel_groups * (pulse_no + block_no * n_chennels);
                }
                else
                {
                    element_no = n_ative_chennel_groups * ((pulse_no - n_chennels) + block_no * n_chennels) + 1;
                }
                for (size_t sample_no = 0; sample_no < nGates / 2; ++sample_no)
                {
                    {
                        data.at({ frame_no, element_no, sample_no }) = rearranged_data.at({ frame_no, pulse_no + block_no * n_chennels * n_ative_chennel_groups, sample_no });
                    }
                }

            }
        }
        rearrange_progress.next();
    }
    rearrange_progress.end();

     // DisplayUtility(data);

  //}



  // Обновляем параметры
    ulaop_params.samples_per_line = (int)nGates/2;
    n_tx_elements = (int)nTransmitsToUse;
    n_rx_elements = (int)nElements;
    samples_per_element = (int)nGates/2;

    printf("  Final data size: [%zu x %zu x %zu]\n", data.sizes(0), data.sizes(1), data.sizes(2));
    printf("  Memory: %.2f MB\n", (double)data.sizes(0) * data.sizes(1) * data.sizes(2) * sizeof(complexF32) / 1048576);
    fflush(stdout);
}


// ============================================================
// Основные методы
// ============================================================

void RawSFDataSourceUlaOp::Init()
{
    printf("\n=== ULA-OP Data Source Init ===\n");
    fflush(stdout);

    uop_filename = GetFileNameRead("ULA-OP parameters file (.uop)", SavedGUIValue("*.uop"), "*.uop");

    //rff_filename = GetFileNameRead("ULA-OP RF data file (.rff256)",         SavedGUIValue("*.rff256"),         "*.rff256");
    rff_filename = uop_filename;
    size_t pos = rff_filename.find(".uop");
    if (pos != std::string::npos) 
    {
        rff_filename.replace(pos, 4, ".rff256");
    }
    // Парсим .uop файл
    ParseUopFile(uop_filename);

    // Парсим .uos файл
    std::string uos_filename = rff_filename;
    pos = uos_filename.find(".rff256");
    if (pos != std::string::npos) 
    {
        uos_filename.replace(pos, 7, ".uos");
    }
    ParseUosFile(uos_filename);

    // Настройка параметров источника данных
    raw_signal_sample_rate = Hz(ulaop_params.sampling_freq)/2;
    //raw_signal_sample_rate = Hz(ulaop_params.sampling_freq);
    hilbert_signal_sample_rate = raw_signal_sample_rate;
    hilbert_samples = ulaop_params.samples_per_line;

    // ============================================================
    // НАСТРОЙКА ДЛЯ ЛИНЕЙНОГО ДАТЧИКА
    // ============================================================

    // Для линейного датчика: sector mode = false, rectangle mode = true
    is_linear_array = true;
    
    // Для линейного датчика - нет угла сектора
    recommended_angle = degrees(0);
    recommended_n_rays = 192;

    probe_carrier = Hz(ulaop_params.center_freq);
    probe_bandwidth = probe_carrier / 2;
    sound_speed = m_sec(ulaop_params.sound_speed);

    array_pitch = mm((float)(ulaop_params.elements_pitch * 1000));

    n_tx_elements = ulaop_params.elements_number;
    n_rx_elements = n_tx_elements;
    samples_per_element = ulaop_params.samples_per_line;

    first_raw_sample = 0;
    last_raw_sample = samples_per_element;

   // printf("\n=== ULA-OP Data Source Ready ===\n");
   // printf("  Elements: %d\n", n_rx_elements);
    //printf("  Transmits: %d\n", n_tx_elements);
   // printf("  Samples per element: %d\n", samples_per_element);
  //  printf("  Pitch: %.3f mm\n", array_pitch.mm());
  //  printf("  Sound speed: %.0f m/s\n", sound_speed.m_sec());
  //  printf("  Center freq: %.2f MHz\n", probe_carrier.MHz());
  //  printf("  Sampling freq: %.2f MHz\n", raw_signal_sample_rate.MHz());
    //printf("  Memory estimate: %.1f MB\n",
    //    (double)n_tx_elements * n_rx_elements * samples_per_element * sizeof(complexF32) / 1048576);
    fflush(stdout);
}

// ============================================================
// ПРОЕКТИРОВАНИЕ FIR ПОЛОСОВОГО ФИЛЬТРА (аналог fir1 в MATLAB)
// ============================================================

void RawSFDataSourceUlaOp::DesignBandpassFilter(int order, double fc_low, double fc_high,
    double fs, std::vector<double>& b)
{
    // Проектирование FIR фильтра методом окна Хэмминга
    // Аналог fir1(order, [fc_low, fc_high]/(fs/2), 'bandpass')

    b.resize(order + 1);

    double f1 = fc_low / (fs / 2.0);  // Нормированная нижняя частота
    double f2 = fc_high / (fs / 2.0); // Нормированная верхняя частота

    int M = order;
    int M2 = M / 2;

    // Окно Хэмминга
    std::vector<double> window(M + 1);
    for (int n = 0; n <= M; ++n) {
        window[n] = 0.54 - 0.46 * cos(2.0 * pi() * n / M);
    }

    // Идеальная импульсная характеристика полосового фильтра
    for (int n = 0; n <= M; ++n) {
        double n_centered = n - M2;
        if (n_centered == 0) {
            b[n] = 2.0 * (f2 - f1);
        }
        else {
            b[n] = (sin(2.0 * pi() * f2 * n_centered) - sin(2.0 * pi() * f1 * n_centered)) /
                (pi() * n_centered);
        }
        // Применяем окно
        b[n] *= window[n];
    }

    // Нормализация для сохранения усиления
    double sum = 0;
    for (double coeff : b) sum += coeff;
    for (double& coeff : b) coeff /= sum;

    //printf("  Bandpass filter designed: order=%d, fc=[%.2f, %.2f] MHz\n",
     //   order, fc_low / 1e6, fc_high / 1e6);
    fflush(stdout);
}

double RawSFDataSourceUlaOp::EstimateCutoffFrequencies(const ComplexFunctionF32& tx_signal, double fs, double threshold_db, double& fc_low, double& fc_high)
{
    double max_val = -1e9;
    fc_low = 1e6;
    fc_high = 7e6;  
    return max_val;
}

// ============================================================
// ПРИМЕНЕНИЕ ФИЛЬТРА К СИГНАЛУ
// ============================================================

void RawSFDataSourceUlaOp::ApplyFilterToData(const std::vector<double>& b,
    ComplexFunctionF32& signal)
{
    size_t N = signal.size();
    size_t M = b.size();

    if (N == 0 || M == 0) return;

    ComplexFunctionF32 filtered(N, complexF32(0));

    // Прямая свертка (Direct Form I)
    for (size_t n = 0; n < N; ++n) {
        complexF32 sum(0, 0);
        size_t k_max = std::min(n + 1, M);
        for (size_t k = 0; k < k_max; ++k) {
            sum += signal[n - k] * (float)b[k];
        }
        filtered[n] = sum;
    }

    // Компенсируем задержку фильтра (M/2 семплов)
    int delay = (int)M / 2;
    for (size_t n = 0; n < N - delay; ++n) {
        signal[n] = filtered[n + delay];
    }
    // Заполняем конец нулями
    for (size_t n = N - delay; n < N; ++n) {
        signal[n] = complexF32(0, 0);
    }
}

// ============================================================
// ПРИМЕНЕНИЕ ПОЛОСОВОЙ ФИЛЬТРАЦИИ КО ВСЕМ ДАННЫМ (ИСПРАВЛЕННАЯ)
// ============================================================

void RawSFDataSourceUlaOp::ApplyBandpassFilterToAllData()
{
    if (!filter_enabled || filter_coefficients.empty()) {
        printf("  Filtering disabled or no filter coefficients\n");
        return;
    }

    printf("\n=== Applying Bandpass Filter ===\n");
    printf("  Filter order: %d\n", filter_order);
    printf("  Cutoff: %.2f - %.2f MHz\n", filter_fc_low / 1e6, filter_fc_high / 1e6);
    fflush(stdout);

    size_t n_tx = data.sizes(0);
    size_t n_rx = data.sizes(1);
    size_t n_samples = data.sizes(2);

    GUIProgressBar filter_progress;
    filter_progress.start("Applying bandpass filter", n_tx * n_rx);

    for (size_t t = 0; t < n_tx; ++t) {
        for (size_t e = 0; e < n_rx; ++e) {
            // Извлекаем сигнал
            ComplexFunctionF32 signal(n_samples);
            for (size_t s = 0; s < n_samples; ++s) {
                signal[s] = data.at({ t, e, s });
            }

            // ПРИМЕНЯЕМ ФИЛЬТР (РАСКОММЕНТИРОВАНО!)
            ApplyFilterToData(filter_coefficients, signal);

            // Сохраняем обратно
            for (size_t s = 0; s < n_samples; ++s) {
                data.at({ t, e, s }) = signal[s];
            }

            filter_progress.next();
        }
    }
    filter_progress.end();

   // printf("  Bandpass filter applied to all data\n");
    fflush(stdout);
}

// ============================================================
// ПРИМЕНЕНИЕ TGC (TIME GAIN COMPENSATION)
// ============================================================

void RawSFDataSourceUlaOp::ApplyTGCToAllData()
{
    if (!tgc_enabled) {
        printf("  TGC disabled\n");
        return;
    }

    printf("\n=== Applying TGC ===\n");
    printf("  Gain: %.1f dB/cm\n", tgc_gain_db_per_cm);
    printf("  Start depth: %.1f cm\n", tgc_start_depth_cm);
    fflush(stdout);

    size_t n_tx = data.sizes(0);
    size_t n_rx = data.sizes(1);
    size_t n_samples = data.sizes(2);

    double fs = raw_signal_sample_rate.Hz();
    double c = sound_speed.m_sec() * 100; // в см/с

    // Предварительно вычисляем коэффициенты TGC для каждого семпла
    std::vector<float> tgc_coeff(n_samples);
    double alpha = tgc_gain_db_per_cm / (20.0 * log10(exp(1.0))); // переводим дБ/см в непер/см
    alpha /= 20.0; // дополнительный фактор из-за round-trip

    for (size_t s = 0; s < n_samples; ++s) {
        double depth_cm = s * 0.5 * (c / fs);
        if (depth_cm < tgc_start_depth_cm) {
            tgc_coeff[s] = 1.0f;
        }
        else {
            // Экспоненциальная компенсация затухания
            tgc_coeff[s] = (float)exp(alpha * (depth_cm - tgc_start_depth_cm));
        }
        // Ограничиваем максимальное усиление (чтобы не усилить шум)
        if (tgc_coeff[s] > 100.0f) tgc_coeff[s] = 100.0f;
    }

    printf("  TGC range: %.2f - %.2f\n", tgc_coeff[0], tgc_coeff[n_samples - 1]);
    fflush(stdout);

    GUIProgressBar tgc_progress;
    tgc_progress.start("Applying TGC", n_tx * n_rx);

    for (size_t t = 0; t < n_tx; ++t) {
        for (size_t e = 0; e < n_rx; ++e) {
            for (size_t s = 0; s < n_samples; ++s) {
                data.at({ t, e, s }) *= tgc_coeff[s];
            }
            tgc_progress.next();
        }
    }
    tgc_progress.end();

   // printf("  TGC applied to all data\n");
    fflush(stdout);
}

// ============================================================
// ПОЛОСОВАЯ ФИЛЬТРАЦИЯ
// ============================================================

void RawSFDataSourceUlaOp::BandPassFiltering()
{
    double fs = raw_signal_sample_rate.Hz();
    double center_freq = probe_carrier.Hz();
    double bandwidth = center_freq * 0.6;

    if (data.sizes(0) > 0 && data.sizes(1) > 0) {
        size_t n_samples = data.sizes(2);
        ComplexFunctionF32 test_signal(n_samples);
        for (size_t s = 0; s < n_samples; ++s) {
            test_signal[s] = data.at({ 0, 0, s });
        }

        double fc_low, fc_high;
        EstimateCutoffFrequencies(test_signal, fs, filter_threshold_db, fc_low, fc_high);

        if (fc_low == 0 || fc_high == 0 || fc_high > fs / 2) {
            fc_low = std::max(center_freq - bandwidth / 2, 0.0);
            fc_high = std::min(center_freq + bandwidth / 2, fs / 2);
            printf("  Using probe parameters: fc=%.2f MHz, BW=%.2f MHz\n",
                center_freq / 1e6, bandwidth / 1e6);
        }

        filter_fc_low = fc_low;
        filter_fc_high = fc_high;

        DesignBandpassFilter(filter_order, fc_low, fc_high, fs, filter_coefficients);

        if (filter_enabled && !filter_coefficients.empty()) {
            ApplyBandpassFilterToAllData();
        }
    }
}

// ============================================================
// HILBERT TRANSFORM (ИСПРАВЛЕННАЯ ВЕРСИЯ)
// ============================================================

void RawSFDataSourceUlaOp::HilbertTransform()
{
    printf("\n=== Applying Hilbert transform ===\n");
    size_t n_tx = data.sizes(0);
    size_t n_rx = data.sizes(1);
    size_t n_samples = data.sizes(2);

    GUIProgressBar hilbert_progress;
    hilbert_progress.start("Hilbert transform", n_tx * n_rx);

    for (size_t t = 0; t < n_tx; ++t) {
        for (size_t e = 0; e < n_rx; ++e) {
            ComplexFunctionF32 signal(n_samples);
            for (size_t s = 0; s < n_samples; ++s) {
                signal[s] = data.at({ t, e, s });
            }

            size_t fft_size = ceil_fft_length(n_samples);
            ComplexFunctionF32 buffer(fft_size, complexF32(0));
            for (size_t s = 0; s < n_samples; ++s) {
                buffer[s] = signal[s];
            }

            FFTf(buffer, fftFwd);

            // Создаем аналитический сигнал
            // Умножаем положительные частоты на 2, обнуляем отрицательные
            for (size_t i = 1; i < fft_size / 2; ++i) {
                buffer[i] *= 2.0f;
            }
            // Сохраняем постоянную составляющую (0 Гц)
            buffer[0] /= 2.0f;
            // Обнуляем отрицательные частоты
            for (size_t i = fft_size / 2 + 1; i < fft_size; ++i) {
                buffer[i] = 0;
            }
            // Сохраняем частоту Найквиста
            if (fft_size % 2 == 0) {
                buffer[fft_size / 2] /= 2.0f;
            }

            FFTf(buffer, fftRev);

            for (size_t s = 0; s < n_samples; ++s) {
                data.at({ t, e, s }) = buffer[s];
            }

            hilbert_progress.next();
        }
    }
    hilbert_progress.end();
    printf("  Hilbert transform complete\n");
}

// ============================================================
// LOAD DATA 
// ============================================================

void RawSFDataSourceUlaOp::LoadData()
{
    //printf("\n=== Loading Data ===\n");
    fflush(stdout);

    if (rff_filename.empty()) {
        throw runtime_error("RFF filename not set");
    }


    tgc_gain_db_per_cm = 40;
 
    if (CapsLock())
    {
        if (YesOrNo("Apply TGC?", MakeGUIValue(false, saved_default_value))) 
        { 
            tgc_enabled = true;
            //double tgc_gain_db_per_cm = 4.0;  // дБ/см (как в MATLAB)
            tgc_gain_db_per_cm = GetFloating("tgc_gain_db_per_cm", MakeGUIValue(4, saved_default_value), 0, 300); 
            //double tgc_start_depth_cm = 0.5;   // Начинаем компенсацию с 0.5 см
            tgc_start_depth_cm = GetFloating("tgc_start_depth_cm", MakeGUIValue(0.5, saved_default_value), 0, 100); 
        }
        else
        {
            tgc_enabled = false;
        }
    }


    // Читаем сырые данные
    ReadRffData(rff_filename);
    //printf("data sizes: %zu x %zu x %zu\n", data.sizes(0), data.sizes(1), data.sizes(2));

    // 1. Полосовая фильтрация (как в MATLAB)
    BandPassFiltering();

    // 2. TGC (как в MATLAB: tgc=4 дБ/см)
    ApplyTGCToAllData();

    // 3. Hilbert transform (RF -> IQ)
    HilbertTransform();

    printf("\n=== Data Loading Complete ===\n");
    printf("  TX transmits: %d\n", n_tx_elements);
    printf("  RX elements: %d\n", n_rx_elements);
    printf("  Samples: %d\n", samples_per_element);
    printf("  Filter: %s, order=%d, fc=[%.2f, %.2f] MHz\n",
        filter_enabled ? "ON" : "OFF", filter_order,
        filter_fc_low / 1e6, filter_fc_high / 1e6);
    printf("  TGC: %s, %.1f dB/cm\n",
        tgc_enabled ? "ON" : "OFF", tgc_gain_db_per_cm);
    printf("  Memory: %.2f MB\n",
        (double)n_tx_elements * n_rx_elements * samples_per_element * sizeof(complexF32) / 1048576);
    fflush(stdout);
}


string RawSFDataSourceUlaOp::GetFileName() const
{
    return rff_filename;
}

void RawSFDataSourceUlaOp::Display() const
{
    parent::DisplayUtility(data);
}

RawSFDataSourceUlaOp::row_ptr RawSFDataSourceUlaOp::GetRow(size_t tx, size_t rx)
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