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
// Чтение данных из .rff256 файла
// ============================================================

void ReadingRawData(const std::string& filename, ComplexFunctionMD_F32& data, int n_chennels, int n_chennel_groups, int n_ative_chennel_groups, size_t file_size)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) 
    {
        throw runtime_error(ssprintf("Cannot open RFF file: %s", filename.c_str()));
    }
    // Читаем как int16
    std::vector<int16_t> raw_data(file_size);

    file.read(reinterpret_cast<char*>(raw_data.data()), file_size * sizeof(int8_t));
    file.close();
    //printf(" Total number of samples: %zu \n", file_size);
    size_t tx_no = 0;
    size_t frame_no = 0;

    int n_sets = data.sizes(0) * data.sizes(1) / (n_chennels * n_ative_chennel_groups);

    GUIProgressBar read_progress;
    read_progress.start("Reading raw data", n_sets);

    for (size_t set_no = 0; set_no < n_sets; ++set_no)
    {
        for (size_t pulse_no = 0; pulse_no < (n_chennels * n_ative_chennel_groups); ++pulse_no)
        {
            for (size_t sample_no = 0; sample_no < data.sizes(2); ++sample_no)
            {
                size_t src_idx = n_chennels * n_chennel_groups * (sample_no + set_no * data.sizes(2)) + pulse_no;
                {
                    data.at({ frame_no, tx_no , sample_no }) = (float)raw_data[src_idx];
                }
            }
            tx_no++;
            if (tx_no == data.sizes(1))
            {
                tx_no = 0;
                frame_no++;
            }
        }
        read_progress.next();
    }
    read_progress.end();
    //DisplayUtility(data);
}

void SwitchBoards(ComplexFunctionMD_F32& data_output, ComplexFunctionMD_F32 data_original, const size_t& i, const size_t& j, size_t ula_n_channels_per_board)
{
    printf("\n Swapping board %zu with %zu", i, j);
    {
        for (size_t tx_no = 0; tx_no < ula_n_channels_per_board; ++tx_no)
        {
            size_t idx_i = tx_no + i * ula_n_channels_per_board;
            size_t idx_j = tx_no + j * ula_n_channels_per_board;
            data_output.GetSlice({ slice_mask(0), idx_i, slice_mask(1) }).CopyData(data_original.GetSlice({ slice_mask(0), idx_j, slice_mask(1) }));
        }
    }
 //   DisplayMathFunction3D(data_output, "data_output");
 //   DisplayMathFunction3D(data_original, "data_original");
}

void ExtractExcitationMap(ComplexFunctionMD_F32 data_original)  // Debug function to deternime Tx elements 
{
    printf("\n Extracting Excitation Map");
    size_t n_elements = data_original.sizes(0);
    size_t n_samples = data_original.sizes(2);

    RealFunction2D_F64 excitation_map(n_elements, n_elements, 0);

    for (size_t i = 0; i < n_elements; ++i)
    {
        for (size_t j = 0; j < n_elements; ++j)
        {
            for (size_t sample_no = 0; sample_no < n_samples; ++sample_no)
            {
                excitation_map.at(i, j) += cabs(data_original.at({ i, j, sample_no }));
            }
         }
    }
    DisplayMathFunction2D(excitation_map, "excitation_map");

    if(YesOrNo("Find positions of the transmit elements?", MakeGUIValue(false, saved_default_value)))
    {
        size_t max_pos(0);  
        for (size_t i = 0; i < n_elements; ++i)
        {
            RealFunctionF64 vector(excitation_map.col(i));
            double max(0);
            for (size_t j = 0; j < n_elements; ++j)
            {
                double a = vector[j];
                if (a > max)
                {
                    max = a;
                    max_pos = j;
                }
            }
            //printf("\nelement=%zu, max_pos=%zu", i, max_pos);
            printf("%zu,", max_pos);
        }
    }
   //   DisplayMathFunction3D(data_output, "data_output");
   //   DisplayMathFunction3D(data_original, "data_original");
}


void RearrangeData(ComplexFunctionMD_F32& data, int n_boards, int n_ative_chennel_groups)
{
    // каждый 8-ой импульс кладем рядом
    ComplexFunctionMD_F32 rearranged_data(data);
    size_t element_no(0);
    size_t n_blocks = data.sizes(1) / (n_ative_chennel_groups * n_boards);

    GUIProgressBar rearrange_progress;
    rearrange_progress.start("Rearranging raw data", n_blocks);
    {
        for (size_t block_no = 0; block_no < n_blocks; ++block_no)
        {

            for (size_t pulse_no = 0; pulse_no < n_ative_chennel_groups * n_boards; ++pulse_no)
            {
                if (pulse_no < n_boards)
                {
                    element_no = n_ative_chennel_groups * (pulse_no + block_no * n_boards);
                }
                else
                {
                    element_no = n_ative_chennel_groups * ((pulse_no - n_boards) + block_no * n_boards) + 1;
                }    
                rearranged_data.GetSlice({ slice_mask(0), element_no, slice_mask(1) }).CopyData(data.GetSlice({ slice_mask(0), pulse_no + block_no * n_boards * n_ative_chennel_groups, slice_mask(1) }));
            }
            rearrange_progress.next();
        }
    }
    rearrange_progress.end();
    data.MakeCopy(rearranged_data);
}

// ============================================================
// Чтение данных из .rff256 файла (ПОЛНАЯ ИСПРАВЛЕННАЯ ВЕРСИЯ)
// ============================================================

void RawSFDataSourceUlaOp::ReadRffData(const std::string& filename)
{
    printf("\n=== Reading RF data ===\n");
    fflush(stdout);

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) 
    {
        throw runtime_error(ssprintf("Cannot open RF file: %s", filename.c_str()));
    }

    size_t nGates = ulaop_params.samples_per_line;
    size_t nElements = ulaop_params.elements_number;

    size_t file_size = ulaop_params.total_size;

    printf(" Gates: %zu\n", nGates);
    printf(" Elements: %zu\n", nElements);

    // Используем только синтетическую апертуру (первые 192 transmits)
    size_t nTransmitsToUse = nElements;

    // Перевыделяем данные: [gates x elements x transmits]
    data.realloc({ nTransmitsToUse, nElements, nGates / 2 });

    data.fill(complexF32(0));

    size_t n_boards = 8;
    size_t n_chennel_groups = 3;
    size_t decimation_factor = 2;  // это может быть связано с потерей каждого второго импульса

    ReadingRawData(filename, data, n_boards, n_chennel_groups, decimation_factor, ulaop_params.total_size);
   // ExtractExcitationMap(data);
    //DisplayUtility(data);
    ComplexFunctionMD_F32 data_buffer(data);
    if (data.sizes(0) == 192)        // меняем местами первую и нулевую группу
    {
        SwitchBoards(data, data_buffer, 0, 1, ulaop_params.n_channels_per_board);
        SwitchBoards(data, data_buffer, 1, 0, ulaop_params.n_channels_per_board);
        RearrangeData(data, n_boards, decimation_factor);     // каждый 8-ой импульс кладем рядом
    }
    else
    {
        RealFunctionF32 element_order({ 202, 194, 223, 215, 211, 219, 205, 197, 193, 201, 212, 216, 208, 220, 222, 214, 213, 221, 210, 218, 203, 195, 199, 207, 198, 206, 204, 196, 192, 200, 217, 209, 148, 156, 138, 130, 155, 152, 144, 147, 129, 134, 137, 142, 135, 143, 140, 132, 141, 151, 149, 157, 133, 159, 158, 150, 128, 136, 139, 131, 153, 145, 146, 154, 88, 80, 91, 83, 65, 73, 86, 92, 84, 94, 93, 69, 77, 85, 87, 71, 95, 79, 70, 66, 74, 78, 90, 82, 67, 81, 89, 75, 64, 72, 76, 68, 56, 48, 34, 42, 46, 38, 55, 63, 59, 51, 37, 41, 33, 45, 60, 52, 40, 32, 36, 44, 61, 53, 58, 50, 54, 62, 35, 43, 47, 39, 57, 49, 235, 249, 241, 227, 224, 232, 228, 250, 236, 242, 245, 253, 254, 246, 239, 231, 252, 244, 230, 238, 234, 229, 226, 237, 247, 251, 255, 243, 225, 233, 248, 240, 178, 186, 185, 177, 171, 163, 160, 168, 172, 164, 166, 174, 181, 189, 191, 183, 190, 182, 173, 165, 167, 175, 180, 188, 170, 162, 161, 169, 179, 187, 184, 176, 113, 121, 96, 104, 122, 114, 126, 118, 99, 107, 103, 111, 117, 125, 127, 119, 100, 108, 102, 110, 101, 109, 123, 115, 97, 105, 116, 124, 112, 120, 25, 106, 98, 17, 12, 8, 0, 4, 26, 18, 29, 21, 11, 30, 22, 3, 7, 15, 6, 14, 31, 10, 2, 23, 5, 13, 27, 19, 9, 1, 28, 20, 24, 16 });
        data_buffer.MakeCopy(data);
        for (size_t i = 0; i < element_order.size(); ++i)
        {
            size_t index = element_order[i];
            data.GetSlice({ slice_mask(0), i, slice_mask(1) }).CopyData(data_buffer.GetSlice({ slice_mask(0), index, slice_mask(1) }));
        }
    }
    if(CapsLock()) { ExtractExcitationMap(data); }
    DisplayUtility(data);

    // Обновляем параметры
    ulaop_params.samples_per_line = (int)nGates / 2;
    samples_per_element = ulaop_params.samples_per_line;
    n_tx_elements = (int)nTransmitsToUse;
    n_rx_elements = (int)nElements;
    printf("\n\n Final data size: [%zu x %zu x %zu]\n", data.sizes(0), data.sizes(1), data.sizes(2));
    printf(" Memory: %.2f MB\n", (double)data.sizes(0) * data.sizes(1) * data.sizes(2) * sizeof(complexF32) / 1048576);
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
    for (int n = 0; n <= M; ++n) 
    {
        double n_centered = n - M2;
        if (n_centered == 0) {
            b[n] = 2.0 * (f2 - f1);
        }
        else 
        {
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

double RawSFDataSourceUlaOp::SetCutoffFrequencies(const ComplexFunctionF32& tx_signal, double fs, double threshold_db, double& fc_low, double& fc_high)
{
    double max_val = -1e9;
    if (ulaop_params.center_freq == 8e6 && !CapsLock())
    {
        fc_low = 1e6;
        fc_high = 7e6;
    }
    else if (ulaop_params.center_freq == 15e6 && !CapsLock())
    {
        fc_low = 7e6;
        fc_high = 17e6;
    }
    else
    {
        fc_low = 1e6 * GetFloating("fc_low (MHz)", MakeGUIValue(1, saved_default_value), 0, 100);
        fc_high = 1e6 * GetFloating("fc_high (MHz)", MakeGUIValue(fc_low / 1e6, saved_default_value), fc_low/1e6, 100);
    }
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
    for (size_t n = 0; n < N; ++n) 
    {
        complexF32 sum(0, 0);
        size_t k_max = std::min(n + 1, M);
        for (size_t k = 0; k < k_max; ++k) 
        {
            sum += signal[n - k] * (float)b[k];
        }
        filtered[n] = sum;
    }

    // Компенсируем задержку фильтра (M/2 семплов)
    int delay = (int)M / 2;
    for (size_t n = 0; n < N - delay; ++n) 
    {
        signal[n] = filtered[n + delay];
    }
    // Заполняем конец нулями
    for (size_t n = N - delay; n < N; ++n) 
    {
        signal[n] = complexF32(0, 0);
    }
}

// ============================================================
// ПРИМЕНЕНИЕ ПОЛОСОВОЙ ФИЛЬТРАЦИИ КО ВСЕМ ДАННЫМ
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

    for (size_t t = 0; t < n_tx; ++t) 
    {
        for (size_t e = 0; e < n_rx; ++e) 
        {
            // Извлекаем сигнал
            ComplexFunctionF32 signal(n_samples);
            for (size_t s = 0; s < n_samples; ++s) 
            {
                signal[s] = data.at({ t, e, s });
            }
            ApplyFilterToData(filter_coefficients, signal);
            for (size_t s = 0; s < n_samples; ++s) 
            {
                data.at({ t, e, s }) = signal[s];
            }
            filter_progress.next();
        }
    }
    filter_progress.end();
    fflush(stdout);
}

// ============================================================
// ПРИМЕНЕНИЕ TGC (TIME GAIN COMPENSATION)
// ============================================================

void RawSFDataSourceUlaOp::ApplyTGCToAllData()
{
    if (!tgc_enabled) 
    {
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

    for (size_t s = 0; s < n_samples; ++s) 
    {
        double depth_cm = s * 0.5 * (c / fs);
        if (depth_cm < tgc_start_depth_cm) tgc_coeff[s] = 1.0f;
        else tgc_coeff[s] = (float)exp(alpha * (depth_cm - tgc_start_depth_cm));  // Экспоненциальная компенсация затухания
        if (tgc_coeff[s] > 100.0f) tgc_coeff[s] = 100.0f;     // Ограничиваем максимальное усиление (чтобы не усилить шум)
    }

    printf("  TGC range: %.2f - %.2f\n", tgc_coeff[0], tgc_coeff[n_samples - 1]);
    fflush(stdout);

    GUIProgressBar tgc_progress;
    tgc_progress.start("Applying TGC", n_tx * n_rx);

    for (size_t t = 0; t < n_tx; ++t) 
    {
        for (size_t e = 0; e < n_rx; ++e) 
        {
            for (size_t s = 0; s < n_samples; ++s) 
            {
                data.at({ t, e, s }) *= tgc_coeff[s];
            }
            tgc_progress.next();
        }
    }
    tgc_progress.end();
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

    if (data.sizes(0) > 0 && data.sizes(1) > 0) 
    {
        size_t n_samples = data.sizes(2);
        ComplexFunctionF32 test_signal(n_samples);
        for (size_t s = 0; s < n_samples; ++s) 
        {
            test_signal[s] = data.at({ 0, 0, s });
        }
        double fc_low, fc_high;
        SetCutoffFrequencies(test_signal, fs, filter_threshold_db, fc_low, fc_high);
        filter_fc_low = fc_low;
        filter_fc_high = fc_high;
        DesignBandpassFilter(filter_order, fc_low, fc_high, fs, filter_coefficients);
        if (filter_enabled && !filter_coefficients.empty()) 
        {
            ApplyBandpassFilterToAllData();
        }
    }
}

// ============================================================
// HILBERT TRANSFORM
// ============================================================

void RawSFDataSourceUlaOp::HilbertTransform()
{
    printf("\n=== Applying Hilbert transform ===\n");
    size_t n_tx = data.sizes(0);
    size_t n_rx = data.sizes(1);
    size_t n_samples = data.sizes(2);

    GUIProgressBar hilbert_progress;
    hilbert_progress.start("Hilbert transform", n_tx * n_rx);

    for (size_t t = 0; t < n_tx; ++t) 
    {
        for (size_t e = 0; e < n_rx; ++e) 
        {
            ComplexFunctionF32 signal(n_samples);
            for (size_t s = 0; s < n_samples; ++s) 
            {
                signal[s] = data.at({ t, e, s });
            }

            size_t fft_size = ceil_fft_length(n_samples);
            ComplexFunctionF32 buffer(fft_size, complexF32(0));
            for (size_t s = 0; s < n_samples; ++s) 
            {
                buffer[s] = signal[s];
            }

            FFTf(buffer, fftFwd);

            // Создаем аналитический сигнал
            // Умножаем положительные частоты на 2, обнуляем отрицательные
            for (size_t i = 1; i < fft_size / 2; ++i) 
            {
                buffer[i] *= 2.0f;
            }
            // Сохраняем постоянную составляющую (0 Гц)
            buffer[0] /= 2.0f;
            // Обнуляем отрицательные частоты
            for (size_t i = fft_size / 2 + 1; i < fft_size; ++i) 
            {
                buffer[i] = 0;
            }
            // Сохраняем частоту Найквиста
            if (fft_size % 2 == 0) 
            {
                buffer[fft_size / 2] /= 2.0f;
            }

            FFTf(buffer, fftRev);

            for (size_t s = 0; s < n_samples; ++s) 
            {
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
    fflush(stdout);
    if (rff_filename.empty()) 
    {
        throw runtime_error("RFF filename not set");
    }

    if (ulaop_params.center_freq == 8e6)
    {
        tgc_gain_db_per_cm = 40;
    }
    else if (ulaop_params.center_freq == 15e6)
    {
        tgc_gain_db_per_cm = 20;
    }
    else
    {
        tgc_gain_db_per_cm = GetFloating(" Gain (dB/cm/MHz)", MakeGUIValue(1, saved_default_value), 0, 100);
    }
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
    // 1. Полосовая фильтрация 
    BandPassFiltering();
    // 2. TGC (tgc=4 дБ/см)
    ApplyTGCToAllData();
    // 3. Hilbert transform (RF -> IQ)
    HilbertTransform();
    printf("\n=== Data Loading Complete ===\n");
    printf("  TX transmits: %d\n", n_tx_elements);
    printf("  RX elements: %d\n", n_rx_elements);
    printf("  Samples: %d\n", samples_per_element);
    printf("  Filter: %s, order=%d, fc=[%.2f, %.2f] MHz\n", filter_enabled ? "ON" : "OFF", filter_order, filter_fc_low / 1e6, filter_fc_high / 1e6);
    printf("  TGC: %s, %.1f dB/cm\n", tgc_enabled ? "ON" : "OFF", tgc_gain_db_per_cm);
    printf("  Memory: %.2f MB\n", (double)n_tx_elements * n_rx_elements * samples_per_element * sizeof(complexF32) / 1048576);
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
    for (size_t i = 0; i < row.size(); ++i) 
    {
        result->data[i] = row[i];
    }
    return row_ptr(result);
}

XRAD_END