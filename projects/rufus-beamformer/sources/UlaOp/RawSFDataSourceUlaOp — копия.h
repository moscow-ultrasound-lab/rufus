// ============================================================
// RawSFDataSourceUlaOp.h - ИСПРАВЛЕННАЯ ВЕРСИЯ
// ============================================================

#ifndef RawSFDataSourceUlaOp_h__
#define RawSFDataSourceUlaOp_h__

#include "SignalProcessing/RawSFDataSource.h"
#include <vector>
#include <string>
#include <fstream>
#include <cstdint>
#include <cstring>

XRAD_BEGIN

class RawSFDataSourceUlaOp : public RawSFDataSource
{
    PARENT(RawSFDataSource);

public:
    RawSFDataSourceUlaOp() : frame_index(0)
    {
        memset(&ulaop_params, 0, sizeof(ulaop_params));
        memset(&rff_info, 0, sizeof(rff_info));
    }
    virtual ~RawSFDataSourceUlaOp() {}

    virtual void Display() const override;
    virtual void LoadData() override;
    virtual void Init() override;
    virtual string GetFileName() const override;

    // Настройка фильтрации
    void SetFilterOrder(int order) { filter_order = order; }
    void SetFilterCutoffThreshold(float threshold_db) { filter_threshold_db = threshold_db; }
    void EnableFiltering(bool enable) { filter_enabled = enable; }
    bool IsFilterEnabled() const { return filter_enabled; }
    double GetFilterFcLow() const { return filter_fc_low; }
    double GetFilterFcHigh() const { return filter_fc_high; }
    int GetFilterOrder() const { return filter_order; }

    // Настройка TGC
    void EnableTGC(bool enable) { tgc_enabled = enable; }
    void SetTGCGain(double gain_db_per_cm) { tgc_gain_db_per_cm = gain_db_per_cm; }
    void SetTGCStartDepth(double start_depth_cm) { tgc_start_depth_cm = start_depth_cm; }
    bool IsTGCEnabled() const { return tgc_enabled; }

    struct UlaOpParameters 
    {
        int elements_number = 192;
        double elements_pitch = 245e-6;
        int start_element = 0;
        double probe_delay = 0;
        double sound_speed = 1540.0;
        double sampling_freq = 78.125e6;
        double center_freq = 8e6;
        int samples_per_line = 8192;
        int n_lines = 288;
        int active_elements = 192;
        int max_voltage = 45;
        int n_blocks = 384;
        int block_length = 8192;
        long long total_size = 0;
    };

    struct Rff256Info {
        int Type = 0;
        int Slice = 0;
        long long TotalSize = 0;
        int NBlocks = 0;
        int BlockLength = 0;
        int BlockSize = 0;
        int FirstBlock = 0;
        int NBoards = 0;
    };

    class row_t : public generic_complex_row_interpolator
    {
    public:
        ComplexFunctionF32 data;
        virtual complexF64 in(double x) override
        {
            return data.in(x, &interpolators::complex_icubic);
        }
    };

    enum TransmitType {
        ALL_TRANSMITS,
        SYNTHETIC_APERTURE_ONLY,
        PLANE_WAVE_ONLY
    };

    bool IsLinearArray() const { return is_linear_array; }
    void SetLinearArray(bool linear) { is_linear_array = linear; }

    virtual row_ptr GetRow(size_t tx, size_t rx) override;
    void SetTransmitType(TransmitType type) { transmit_type = type; }
    void SetFrameIndex(int idx) { frame_index = idx; }
    void SetActiveElements(const std::vector<int>& elements) { active_elements = elements; }

private:
    void ParseUopFile(const std::string& filename);
    void ParseUosFile(const std::string& filename);
    void ReadRffData(const std::string& filename);

    void Unpack12BitData(const std::vector<uint8_t>& packed_data,
        std::vector<int16_t>& unpacked_data,
        size_t n_gates,
        size_t n_elements_32,
        size_t n_transmits);
    void Unpack12BitDataSimple(const std::vector<uint8_t>& packed_data,
        std::vector<int16_t>& unpacked_data,
        size_t n_gates,
        size_t n_elements,
        size_t n_transmits);
    void DiagnoseFile(const std::string& filename);

    // Методы фильтрации
    void DesignBandpassFilter(int order, double fc_low, double fc_high, double fs,
        std::vector<double>& b);
    void ApplyFilterToData(const std::vector<double>& b,
        ComplexFunctionF32& signal);
    void ApplyFIRFilter(ComplexFunctionF32& signal);
    double EstimateCutoffFrequencies(const ComplexFunctionF32& tx_signal,
        double fs, double threshold_db,
        double& fc_low, double& fc_high);
    void ApplyBandpassFilterToAllData();
    void BandPassFiltering();

    // Методы TGC
    void ApplyTGCToAllData();

    void HilbertTransform();

    typedef ComplexFunctionMD_F32 data_volume_type;
    data_volume_type data;

    string rff_filename;
    string uop_filename;
    string uos_filename;
    int frame_index;

    UlaOpParameters ulaop_params;
    Rff256Info rff_info;

    std::vector<int> active_elements;
    std::vector<int> active_transmits;

    TransmitType transmit_type = SYNTHETIC_APERTURE_ONLY;
    bool is_linear_array = true;

    // Параметры фильтра
    bool filter_enabled = true;
    int filter_order = 81;
    float filter_threshold_db = -20.0f;
    double filter_fc_low = 0;
    double filter_fc_high = 0;
    std::vector<double> filter_coefficients;

    // Параметры TGC
    bool tgc_enabled = true;
    //double tgc_gain_db_per_cm = 4.0;  // дБ/см (как в MATLAB)
    double tgc_gain_db_per_cm = 8.0;  // дБ/см (как в MATLAB)
    //double tgc_start_depth_cm = 0.5;   // Начинаем компенсацию с 0.5 см
    double tgc_start_depth_cm = 1;   // Начинаем компенсацию с 0.5 см

    bool IsSyntheticApertureTransmit(int tx_index) const;
    bool IsPlaneWaveTransmit(int tx_index) const;
};

XRAD_END

#endif