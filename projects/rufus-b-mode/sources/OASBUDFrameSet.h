#ifndef __oasbud_frame_set_h
#define __oasbud_frame_set_h

#include <XRADBasic/MathFunctionTypes2D.h>
#include <XRADBasic/MathFunctionTypesMD.h>
#include <XRADBasic/Sources/ScanConverter/ScanConverterOptions.h>
#include <RFDataImport/S500_CFMDataTypes.h>  // <-- ДОБАВИТЬ ЭТОТ ЗАГОЛОВОК
#include <matio.h>
#include <string>

#define MAT_T_CHAR 16

XRAD_BEGIN


// Структура для хранения данных одного поражения
struct OASBUDLesionData {
    string id;              // ID как строка (например "19wp")
    string birads;          // BI-RADS как строка ("3", "4a", "4b", "4c", "5")
    bool is_malignant;      // 0 - benign, 1 - malignant

    // B-mode изображения
    b_slice_t b_frame1;
    b_slice_t b_frame2;

    b_slice_t roi1;
    b_slice_t roi2;

    // ROI маски
    RealFunction2D_F32 mask1;
    RealFunction2D_F32 mask2;

    size_t n_lines;
    size_t n_samples;

    void PrintInfo() const {
        printf("\n========================================");
        printf("\nLesion ID: %s", id.c_str());
        printf("\nBI-RADS: %s", birads.c_str());
        printf("\nType: %s", is_malignant ? "MALIGNANT" : "BENIGN");
        printf("\nB-mode1: %zux%zu", b_frame1.vsize(), b_frame1.hsize());
        if (b_frame2.vsize() > 0) {
            printf("\nB-mode2: %zux%zu", b_frame2.vsize(), b_frame2.hsize());
        }
        printf("\n========================================\n");
    }
};

class OASBUDFrameSet
{
public:
    // Публичные константы
    static constexpr double SOUND_SPEED = 1540.0;    // m/s
    static constexpr double SAMPLING_FREQ = 40.0e6;  // 40 MHz
    static constexpr double PITCH_MM = 0.3;          // Шаг элементов L14-5
    static constexpr int NUM_RF_LINES = 510;

    OASBUDFrameSet();
    ~OASBUDFrameSet();

    // Загрузка данных из .mat файла
    
    bool ReadMatFile(const string& filename);
    // Доступ к данным
    size_t GetNumLesions() const { return m_lesions.size(); }

    // Отображение
    void DisplayMetadata() const;
    void DisplayLesionWithMenu(size_t index);

    // Параметры сканирования для отображения
    const ScanConverterOptions& GetScanConverterOptions() const { return m_sco; }

private:
    
    ComplexFunction2D_F32 ExtractRFData(matvar_t* rf_var);
    RealFunction2D_F32 ExtractROI(matvar_t* roi_var);
    // Построение B-mode из РФ сигнала (через преобразование Гильберта)
    b_slice_t BuildBModeImage(const ComplexFunction2D_F32& rf_data, const RealFunction2D_F32* roi = nullptr, double dynamic_range = 60.0);
    b_slice_t ApplyMask(b_slice_t& result, RealFunction2D_F32* roi);

    vector<OASBUDLesionData> m_lesions;
    ScanConverterOptions m_sco;

    // Вспомогательные константы для отображения
    static constexpr double DEFAULT_DYNAMIC_RANGE = 60.0;
};

XRAD_END

#endif // __oasbud_frame_set_h