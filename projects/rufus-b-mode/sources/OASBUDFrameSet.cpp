#include "pre.h"
#include "OASBUDFrameSet.h"
#include <matio.h>
#include <cmath>
#include <map>

#ifdef _WIN32
#include <windows.h>
#include <fileapi.h>
#endif

XRAD_BEGIN


// Функция для получения короткого пути (8.3) на Windows
string GetShortPath(const string& longPath)
{
#ifdef _WIN32
    // Конвертируем UTF-8 в UTF-16 для Windows API
    wstring wLongPath = string_to_wstring(longPath, e_decode_literals);

    // Проверяем существование файла
    DWORD attrs = GetFileAttributesW(wLongPath.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        printf("  File does not exist or cannot be accessed\n");
        return longPath;
    }
    printf("  File exists, attributes: 0x%08X\n", attrs);

    // Получаем длину короткого имени
    DWORD shortLen = GetShortPathNameW(wLongPath.c_str(), NULL, 0);
    if (shortLen == 0) {
        DWORD err = GetLastError();
        printf("  GetShortPathName failed with error: %lu\n", err);
        return longPath;
    }

    // Получаем короткое имя
    wchar_t* shortPath = new wchar_t[shortLen];
    if (GetShortPathNameW(wLongPath.c_str(), shortPath, shortLen) == 0) {
        DWORD err = GetLastError();
        printf("  GetShortPathName failed with error: %lu\n", err);
        delete[] shortPath;
        return longPath;
    }

    string result = wstring_to_string(wstring(shortPath), e_encode_literals);
    delete[] shortPath;
    printf("  Short path: %s\n", result.c_str());
    return result;
#else
    return longPath;
#endif
}

mat_t* OpenMatFile(const string& filename)
{
   // printf("\n=== Trying to open file ===\n");
    printf("Original path: %s\n", filename.c_str());

    mat_t* mat = Mat_Open(filename.c_str(), MAT_ACC_RDONLY);

    //printf("Trying to open just filename...\n");
    // Получаем только имя файла
    size_t last_slash = filename.find_last_of("/\\");
    string basename = (last_slash != string::npos) ? filename.substr(last_slash + 1) : filename;
    printf("Basename: %s\n", basename.c_str());

    // Сохраняем текущую директорию
    char current_dir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, current_dir);
    //printf("Current directory: %s\n", current_dir);

    // Переходим в директорию файла
    if (last_slash != string::npos) {
        string dir = filename.substr(0, last_slash);
        wstring wdir = string_to_wstring(dir, e_decode_literals);
        SetCurrentDirectoryW(wdir.c_str());
        //printf("Changed to directory: %s\n", dir.c_str());
    }

    // Пробуем открыть только имя файла
    mat = Mat_Open(basename.c_str(), MAT_ACC_RDONLY);

    // Возвращаемся в исходную директорию
    SetCurrentDirectoryA(current_dir);

    if (mat) {
        //printf("SUCCESS: Opened via basename with directory change\n");
        return mat;
    }

    printf("ALL methods failed to open file\n");
    return NULL;
}

OASBUDFrameSet::OASBUDFrameSet()
{
    m_sco = ScanFrameRectangle(mm(38.0), mm(50.0));
    m_sco.pixels_per_cm = 60;
}

OASBUDFrameSet::~OASBUDFrameSet()
{
}

void OASBUDFrameSet::DisplayLesionWithMenu(size_t index)
{
    const auto& lesion = m_lesions[index];

    while (true) {
        printf("\n========================================");
        printf("\nLesion ID: %s", lesion.id.c_str());
        printf("\nBI-RADS: %s", lesion.birads.c_str());
        printf("\nType: %s", lesion.is_malignant ? "MALIGNANT" : "BENIGN");
        printf("\n========================================\n");

        size_t choice = GetButtonDecision("Lesion Viewer",
            {
                MakeButton(L"B-mode image (Scan 1)", 0),
                MakeButton(L"Mask (Scan 1)", 1),
                MakeButton(L"ROI (Scan 1)", 2),
                MakeButton(L"B-mode image (Scan 2)", 3),
                MakeButton(L"Mask (Scan 2)", 4),
                MakeButton(L"ROI (Scan 2)", 5),
                MakeButton(L"Return to lesions list", 6)
            });

        try {
            if (choice == 0)
                DisplayMathFunction2D(lesion.b_frame1, ssprintf("Lesion ID=%s, BI-RADS=%s, %s (Scan 1 - Full B-mode)", lesion.id.c_str(), lesion.birads.c_str(), lesion.is_malignant ? "MALIGNANT" : "BENIGN"), m_sco);
            else if (choice == 1)
                DisplayMathFunction2D(lesion.mask1, ssprintf("Lesion ID=%s, BI-RADS=%s, %s (Scan 1 - Mask)", lesion.id.c_str(), lesion.birads.c_str(), lesion.is_malignant ? "MALIGNANT" : "BENIGN"), m_sco);
            else if (choice == 2)
                DisplayMathFunction2D(lesion.roi1, ssprintf("Lesion ID=%s, BI-RADS=%s, %s (Scan 1 - ROI Only)", lesion.id.c_str(), lesion.birads.c_str(), lesion.is_malignant ? "MALIGNANT" : "BENIGN"), m_sco);
            else if (choice == 3)
                DisplayMathFunction2D(lesion.b_frame2, ssprintf("Lesion ID=%s, BI-RADS=%s, %s (Scan 2 - Full B-mode)", lesion.id.c_str(), lesion.birads.c_str(), lesion.is_malignant ? "MALIGNANT" : "BENIGN"), m_sco);
            else if (choice == 4)
                DisplayMathFunction2D(lesion.mask2, ssprintf("Lesion ID=%s, BI-RADS=%s, %s (Scan 2 - Mask)", lesion.id.c_str(), lesion.birads.c_str(), lesion.is_malignant ? "MALIGNANT" : "BENIGN"), m_sco);
            else if (choice == 5)
                DisplayMathFunction2D(lesion.roi2, ssprintf("Lesion ID=%s, BI-RADS=%s, %s (Scan 2 - ROI Only)", lesion.id.c_str(), lesion.birads.c_str(), lesion.is_malignant ? "MALIGNANT" : "BENIGN"), m_sco);
            else if (choice == 6)
            {
                printf("\nReturning to lesions list...\n");
                break;
            }
        }
        catch (canceled_operation&)
        {
            printf("\nDisplay window closed. Returning to lesion menu...\n");
            continue;
        }
    }
}

bool OASBUDFrameSet::ReadMatFile(const string& filename)
{
    printf("\n========================================");
    printf("\nLoading OASBUD Database");
    printf("\n========================================\n");

    mat_t* mat = OpenMatFile(filename);

    if (!mat) {
        printf("ERROR: Cannot open file\n");
        return false;
    }
    printf("File opened successfully\n\n");

    matvar_t* data_var = Mat_VarRead(mat, "data");
    if (!data_var || data_var->class_type != MAT_C_STRUCT)
    {
        printf("ERROR: Invalid OASBUD file format\n");
        if (data_var) Mat_VarFree(data_var);
        Mat_Close(mat);
        return false;
    }

    printf("Variable 'data' found\n");
    printf("  Class type: %d\n", data_var->class_type);
    printf("  Rank: %d\n", data_var->rank);
    printf("  Dimensions: ");

    for (int i = 0; i < data_var->rank; i++)
    {
        printf("%zu ", data_var->dims[i]);
    }
    printf("\n");

    size_t num_lesions = (data_var->rank == 2) ? data_var->dims[0] * data_var->dims[1] : data_var->dims[0];
    printf("Total lesions: %zu\n", num_lesions);

    m_lesions.reserve(num_lesions);

    int successful_lesions = 0;

    GUIProgressBar	progress;
    progress.start("Reading OASBUD file", num_lesions);
    {
        for (size_t i = 0; i < num_lesions; i++)
        {
            OASBUDLesionData lesion;
            bool has_data = false;

            printf("\n--- Lesion %zu ---\n", i);

            // --- Чтение ID ---
            matvar_t* id_var = Mat_VarGetStructFieldByName(data_var, "Id", i);
            if (!id_var)
            {
                id_var = Mat_VarGetStructFieldByName(data_var, "id", i);
            }
            if (id_var)
            {
                if (id_var->data_type == MAT_T_CHAR || id_var->data_type == MAT_T_UINT8)
                {
                    char* str_data = (char*)id_var->data;
                    if (str_data && id_var->nbytes > 0) {
                        lesion.id = string(str_data, id_var->nbytes);
                        // Удаляем пробелы в конце
                        while (!lesion.id.empty() && lesion.id.back() == ' ')
                        {
                            lesion.id.pop_back();
                        }
                        // Удаляем нулевые символы в конце
                        while (!lesion.id.empty() && lesion.id.back() == '\0')
                        {
                            lesion.id.pop_back();
                        }
                        printf("  ID = '%s'\n", lesion.id.c_str());
                    }
                }
                else if (id_var->data_type == MAT_T_DOUBLE)
                {
                    // Если вдруг ID как число, конвертируем в строку
                    lesion.id = std::to_string((int)((double*)id_var->data)[0]);
                    printf("  ID = '%s'\n", lesion.id.c_str());
                }
                else
                {
                    printf("  ID has wrong type: %d\n", id_var->data_type);
                }
                Mat_VarFree(id_var);
            }
            else
            {
                // Если ID не найден, генерируем
                lesion.id = "Lesion_" + std::to_string(i);
                printf("  ID NOT found, using '%s'\n", lesion.id.c_str());
            }

            // --- Чтение BI-RADS (как строка) ---
            matvar_t* birads_var = Mat_VarGetStructFieldByName(data_var, "birads", i);
            if (birads_var) {
                if (birads_var->data_type == MAT_T_CHAR || birads_var->data_type == MAT_T_UINT8)
                {
                    char* str_data = (char*)birads_var->data;
                    if (str_data && birads_var->nbytes > 0) {
                        lesion.birads = string(str_data, birads_var->nbytes);
                        // Удаляем пробелы в конце
                        while (!lesion.birads.empty() && lesion.birads.back() == ' ')
                        {
                            lesion.birads.pop_back();
                        }
                        while (!lesion.birads.empty() && lesion.birads.back() == '\0')
                        {
                            lesion.birads.pop_back();
                        }
                        printf("  BI-RADS = '%s'\n", lesion.birads.c_str());
                    }
                }
                else if (birads_var->data_type == MAT_T_DOUBLE)
                {
                    // Если BI-RADS как число, конвертируем в строку
                    int val = (int)((double*)birads_var->data)[0];
                    lesion.birads = std::to_string(val);
                    printf("  BI-RADS = '%s'\n", lesion.birads.c_str());
                }
                else
                {
                    printf("  BI-RADS has wrong type: %d\n", birads_var->data_type);
                }
                Mat_VarFree(birads_var);
            }
            else
            {
                lesion.birads = "unknown";
                printf("  BI-RADS NOT found, using 'unknown'\n");
            }

            // --- Чтение класса ---
            matvar_t* class_var = Mat_VarGetStructFieldByName(data_var, "class", i);
            if (!class_var)
            {
                class_var = Mat_VarGetStructFieldByName(data_var, "Class", i);
            }
            if (class_var)
            {
                if (class_var->data_type == MAT_T_DOUBLE)
                {
                    lesion.is_malignant = (int)((double*)class_var->data)[0] == 1;
                    printf("  Class = %s\n", lesion.is_malignant ? "MALIGNANT" : "BENIGN");
                    has_data = true;
                }
                else
                {
                    printf("  Class has wrong type: %d\n", class_var->data_type);
                }
            }
            else
            {
                printf("  Class NOT found\n");
            }
            Mat_VarFree(class_var);

            // --- Чтение ROI масок ---
            matvar_t* roi1_var = Mat_VarGetStructFieldByName(data_var, "roi1", i);
            if (roi1_var)
            {
                lesion.mask1 = ExtractROI(roi1_var);
                printf("  Scan1 dimensions: %zux%zu\n", lesion.mask1.vsize(), lesion.mask1.hsize());
                if (lesion.mask1.vsize() > 0 && lesion.mask1.hsize() > 0) has_data = true;
            }
            else
            {
                printf("  ROI1 NOT found\n");
            }
            Mat_VarFree(roi1_var);

            matvar_t* roi2_var = Mat_VarGetStructFieldByName(data_var, "roi2", i);
            if (roi2_var)
            {
                lesion.mask2 = ExtractROI(roi2_var);
                printf("  Scan2 dimensions: %zux%zu\n", lesion.mask2.vsize(), lesion.mask2.hsize());
            }
            else
            {
                printf("  ROI2 NOT found\n");
            }
            Mat_VarFree(roi2_var);

            // --- Чтение РФ сигналов ---
            matvar_t* rf1_var = Mat_VarGetStructFieldByName(data_var, "rf1", i);
            if (rf1_var)
            {
                ComplexFunction2D_F32 rf_data = ExtractRFData(rf1_var);
                lesion.b_frame1 = BuildBModeImage(rf_data, (lesion.mask1.vsize() > 0) ? &lesion.mask1 : nullptr, DEFAULT_DYNAMIC_RANGE);
                lesion.roi1 = ApplyMask(lesion.b_frame1, &lesion.mask1);
                lesion.n_lines = lesion.b_frame1.vsize();
                lesion.n_samples = lesion.b_frame1.hsize();
               // printf("  B-mode1 dimensions: %zux%zu\n", lesion.n_lines, lesion.n_samples);
                has_data = true;
                Mat_VarFree(rf1_var);
            }

            matvar_t* rf2_var = Mat_VarGetStructFieldByName(data_var, "rf2", i);
            if (rf2_var)
            {
                ComplexFunction2D_F32 rf_data = ExtractRFData(rf2_var);
                if (rf_data.vsize() > 0 && rf_data.hsize() > 0)
                {
                    lesion.b_frame2 = BuildBModeImage(rf_data, (lesion.mask2.vsize() > 0) ? &lesion.mask2 : nullptr, DEFAULT_DYNAMIC_RANGE);
                    lesion.roi2 = ApplyMask(lesion.b_frame2, &lesion.mask2);
                   // printf("  B-mode2 dimensions: %zux%zu\n", lesion.b_frame2.vsize(), lesion.b_frame2.hsize());
                }
                else
                {
                    printf("  RF2 is empty\n");
                }
                Mat_VarFree(rf2_var);
            }
            else
            {
                printf("  RF2 NOT found\n");
            }

            if (has_data) {
                m_lesions.push_back(lesion);
                successful_lesions++;
            }
            else
            {
                printf("  WARNING: No data found for this lesion\n");
            }
            ++progress;
        }
    }
    progress.end();

    Mat_Close(mat);
    printf("\n========================================");
    printf("\nLoad complete");
    printf("\n  Total lesions in file: %zu", num_lesions);
    printf("\n  Successfully loaded: %d", successful_lesions);
    printf("\n========================================\n");

    // Обновляем параметры сканирования
    if (!m_lesions.empty())
    {
        size_t n_lines = m_lesions[0].b_frame1.vsize();
        size_t n_samples = m_lesions[0].b_frame1.hsize();

        double depth_mm = n_samples * (SOUND_SPEED / 2.0) / SAMPLING_FREQ * 1000;
        double width_mm = n_lines * PITCH_MM;

        printf("\nImage dimensions:");
        printf("\n  Lines (beams): %zu", n_lines);
        printf("\n  Samples (depth): %zu", n_samples);
        printf("\n  Width: %.1f mm", width_mm);
        printf("\n  Depth: %.1f mm\n", depth_mm);

        m_sco = ScanFrameRectangle(mm(width_mm), mm(depth_mm));
        m_sco.pixels_per_cm = 60;
    }
    DisplayMetadata();

    return !m_lesions.empty();
}

ComplexFunction2D_F32 OASBUDFrameSet::ExtractRFData(matvar_t* rf_var)
{
    ComplexFunction2D_F32 result;
    if (!rf_var || rf_var->rank != 2)
    {
        return result;
    }
    size_t rows = rf_var->dims[0];
    size_t cols = rf_var->dims[1];
    result.realloc(cols, rows, complexF32(0));

    if (rf_var->data_type == MAT_T_DOUBLE) {
        double* data = (double*)rf_var->data;
        for (size_t i = 0; i < rows; i++)
        {
            for (size_t j = 0; j < cols; j++)
            {
                result.at(j, i) = complexF32((float)data[i + j * rows], 0);
            }
        }
    }
    else if (rf_var->data_type == MAT_T_SINGLE) {
        float* data = (float*)rf_var->data;
        for (size_t i = 0; i < rows; i++)
        {
            for (size_t j = 0; j < cols; j++)
            {
                result.at(j, i) = complexF32(data[i + j * rows], 0);
            }
        }
    }
    return result;
}

RealFunction2D_F32 OASBUDFrameSet::ExtractROI(matvar_t* roi_var)
{
    RealFunction2D_F32 result;
    if (!roi_var || roi_var->rank != 2)
    {
        return result;
    }
    size_t rows = roi_var->dims[0];
    size_t cols = roi_var->dims[1];
    result.realloc(cols, rows, 0.0f);

    if (roi_var->data_type == MAT_T_DOUBLE) {
        double* data = (double*)roi_var->data;
        for (size_t i = 0; i < rows; i++)
        {
            for (size_t j = 0; j < cols; j++)
            {
                result.at(j, i) = (float)data[i + j * rows];
            }
        }
    }
    else if (roi_var->data_type == MAT_T_UINT8) {
        uint8_t* data = (uint8_t*)roi_var->data;
        for (size_t i = 0; i < rows; i++)
        {
            for (size_t j = 0; j < cols; j++)
            {
                result.at(j, i) = data[i + j * rows] > 0 ? 1.0f : 0.0f;
            }
        }
    }
    return result;
}

b_slice_t OASBUDFrameSet::ApplyMask(b_slice_t& data, RealFunction2D_F32* roi)
{
    b_slice_t result(data);
    for (size_t i = 0; i < result.vsize(); i++)
    {
        for (size_t j = 0; j < result.hsize(); j++)
        {
            if (roi && roi->at(i, j) < 0.5)
            {
                result.at(i, j) = complexF32(0, 0);
            }
        }
    }
    return result;
}

b_slice_t OASBUDFrameSet::BuildBModeImage(const ComplexFunction2D_F32& rf_data, const RealFunction2D_F32* roi, double dynamic_range)
{
    size_t n_lines = rf_data.vsize();
    size_t n_samples = rf_data.hsize();

    b_slice_t result(n_lines, n_samples, complexF32(0));

    const int hilbert_order = 31;
    RealFunctionF32 hilbert_kernel(hilbert_order);

    static const double h_coeff[31] = {
        -0.005261, -0.004399, -0.002665, -0.000505,  0.001690,
         0.003500,  0.004371,  0.003793,  0.001614, -0.001718,
        -0.005681, -0.009026, -0.010567, -0.009248, -0.004525,
         0.004350,  0.018528,  0.038801,  0.065575,  0.098646,
         0.137538,  0.181210,  0.227991,  0.275862,  0.322653,
         0.366389,  0.405525,  0.438972,  0.466170,  0.487025,
         0.501568
    };

    for (int i = 0; i < hilbert_order; i++)
    {
        hilbert_kernel[i] = (float)h_coeff[i];
    }

    int half_kernel = hilbert_order / 2;
    RealFunction2D_F32 envelope(n_lines, n_samples, 0.0f);

    for (size_t i = 0; i < n_lines; i++)
    {
        for (size_t j = 0; j < n_samples; j++)
        {
            double I = rf_data.at(i, j).re;
            double Q = 0.0;
            for (int k = 0; k < hilbert_order; k++)
            {
                int idx = (int)i - half_kernel + k;
                if (idx >= 0 && idx < (int)n_lines)
                {
                    Q += rf_data.at(idx, j).re * hilbert_kernel[k];
                }
            }
            envelope.at(i, j) = (float)sqrt(I * I + Q * Q);
        }
    }

    double max_val = MaxValue(envelope);
    if (max_val <= 0) max_val = 1.0;

    double scale = dynamic_range / 10.0;
    double log_scale = log(1.0 + scale);

    for (size_t i = 0; i < n_lines; i++)
    {
        for (size_t j = 0; j < n_samples; j++)
        {
            double normalized = envelope.at(i, j) / max_val;
            double compressed = log(1.0 + scale * normalized) / log_scale;
            compressed = compressed * 1.3;
            if (compressed > 1.0) compressed = 1.0;
            compressed = pow(compressed, 0.7);
            result.at(i, j) = complexF32((float)compressed, 0);
        }
    }
    return result;
}

void OASBUDFrameSet::DisplayMetadata() const
{
    printf("\n========================================");
    printf("\nOASBUD Database Metadata");
    printf("\n========================================\n");
    int malignant = 0, benign = 0;
    std::map<string, int> birads_dist;
    for (const auto& lesion : m_lesions)
    {
        if (lesion.is_malignant) malignant++;
        else benign++;
        birads_dist[lesion.birads]++;
    }
    printf("Total lesions: %zu\n", m_lesions.size());
    printf("  Malignant: %d (%.1f%%)\n", malignant, 100.0 * malignant / m_lesions.size());
    printf("  Benign: %d (%.1f%%)\n", benign, 100.0 * benign / m_lesions.size());
    printf("\nBI-RADS distribution:\n");
    for (const auto& pair : birads_dist)
    {
        printf("  Category %s: %d lesions (%.1f%%)\n", pair.first.c_str(), pair.second, 100.0 * pair.second / m_lesions.size());
    }
    if (!m_lesions.empty())
    {
        double depth_mm = m_lesions[0].b_frame1.hsize() * (SOUND_SPEED / 2.0) / SAMPLING_FREQ * 1000;
        double width_mm = m_lesions[0].b_frame1.vsize() * PITCH_MM;
        printf("\nImage dimensions:");
        printf("\n  Width: %.1f mm", width_mm);
        printf("\n  Depth: %.1f mm\n", depth_mm);
    }
}

XRAD_END