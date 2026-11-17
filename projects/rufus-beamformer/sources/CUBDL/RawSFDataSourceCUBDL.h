// RawSFDataSourceCUBDL.h
#ifndef RawSFDataSourceCUBDL_h__
#define RawSFDataSourceCUBDL_h__

#include "SignalProcessing/RawSFDataSource.h"
#include <vector>
#include <string>
#include <cstdint>

// Предварительное объявление hid_t для использования в заголовочном файле
// Фактическое определение будет из hdf5.h в .cpp файле
#ifndef H5Ipublic_H
typedef int64_t hid_t;
#endif

XRAD_BEGIN

class RawSFDataSourceCUBDL : public RawSFDataSource
{
    PARENT(RawSFDataSource);

public:
    RawSFDataSourceCUBDL() : frame_index(0), array_pitch_mm(0) {}
    virtual ~RawSFDataSourceCUBDL() {}

    virtual void Display() const override;
    virtual void LoadData() override;
    virtual void Init() override;
    virtual string GetFileName() const override;

    class row_t : public generic_complex_row_interpolator
    {
    public:
        ComplexFunctionF32 data;
        virtual complexF64 in(double x) override
        {
            return data.in(x, &interpolators::complex_icubic);
        }
    };

    virtual row_ptr GetRow(size_t tx, size_t rx) override;

    void SetFrameIndex(int idx) { frame_index = idx; }
    void SetSequenceNumber(const std::string& seq) { sequence_number = seq; }

    // Структура для хранения метаданных CUBDL
    struct CUBDLMetadata {
        std::string sequence_id;
        std::string institution;
        std::string scanner;
        std::string transducer;
        double center_freq_mhz = 0;
        double sampling_freq_mhz = 0;
        double sound_speed = 1540.0;
        int n_plane_waves = 0;
        double image_depth_mm = 0;
        double pitch_mm = 0;
        bool is_phantom = true;
        std::string phantom_model;
        std::string tissue_type;
    };

    const CUBDLMetadata& GetMetadata() const { return metadata; }
    //size_t NumAngles() const { return angles_rad.size(); }

private:
    // Вспомогательные методы загрузки
    void LoadHDF5Metadata(const std::string& filename);
    void LoadHDF5Data(const std::string& filename);
    void ParseSequenceID(const std::string& seq_id);
    void hilberttransform();

    // Методы для работы с HDF5 - используем предварительно объявленный hid_t
    void AnalyzeDataset(hid_t dataset_id, const std::string& path);
    void ReadDatasetGeneric(hid_t dataset_id, std::vector<int16_t>& buffer);

    // Для обратной совместимости с другими форматами
    void LoadHDF5DataLegacy(const std::string& filename);

    typedef ComplexFunctionMD_F32 data_volume_type;
    data_volume_type data;

    string filename;
    int frame_index;
    std::vector<int> transmits;

    // Параметры из HDF5 файла
    float verasonics_sampling_freq = 40.0e6f;
    float verasonics_center_freq = 7.8e6f;
    float verasonics_sound_speed = 1540.0f;
    float array_pitch_mm = 0.3f;
    int n_samples = 0;
    double angle_min_deg = -16.0;
    double angle_max_deg = 16.0;
    int n_angles = 75;
    double inc;  // эмпирический сдвиг сэмплов
    CUBDLMetadata metadata;
    std::string sequence_number;

    bool is_linear_array = true;  // По умолчанию линейный датчик

    bool IsSyntheticApertureTransmit(int tx_index) const;
    bool IsPlaneWaveTransmit(int tx_index) const;

    // Структура для описания layout данных
    struct DatasetLayout {
        size_t n_frames = 1;
        size_t n_transmits = 128;
        size_t n_samples = 0;
        size_t n_elements = 128;
        int data_type = 0; // 0=unknown, 1=int16, 2=float32, 3=complex
        size_t total_size = 0;
        std::vector<size_t> dims;
    };

    DatasetLayout layout;
    //std::vector<double> angles_rad;   // угол (в радианах) для каждой плоской волны; индекс == tx в GetRow()
};

XRAD_END

#endif