// RawSFDataSourceVerasonics.h
#ifndef RawSFDataSourceVerasonics_h__
#define RawSFDataSourceVerasonics_h__

#include "SignalProcessing/RawSFDataSource.h"
#include <highfive/H5File.hpp>

#include <vector>
#include <string>

XRAD_BEGIN

class RawSFDataSourceVerasonics : public RawSFDataSource
{
    PARENT(RawSFDataSource);

public:
    RawSFDataSourceVerasonics() : frame_index(0), array_pitch_mm(0) {}
    virtual ~RawSFDataSourceVerasonics() {}

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
    void SetTransmits(const std::vector<int>& tx_list) { transmits = tx_list; }

    // Методы для выбора типа transmits
    enum TransmitType {
        ALL_TRANSMITS,
        SYNTHETIC_APERTURE_ONLY,
        PLANE_WAVE_ONLY
    };

    void SetTransmitType(TransmitType type) { transmit_type = type; }
    void SetCustomTransmits(const std::vector<int>& tx_list) { transmits = tx_list; }

    // Для линейного датчика
    bool IsLinearArray() const { return is_linear_array; }
    void SetLinearArray(bool linear) { is_linear_array = linear; }

private:
    void LoadHDF5Metadata(const std::string& filename);
    void LoadHDF5Data(const std::string& filename);

    typedef ComplexFunctionMD_F32 data_volume_type;
    data_volume_type data;

    string filename;
    int frame_index;
    std::vector<int> transmits;

    float verasonics_sampling_freq;
    float verasonics_center_freq;
    float verasonics_sound_speed;
    float array_pitch_mm;

    TransmitType transmit_type = SYNTHETIC_APERTURE_ONLY;
    bool is_linear_array = true;  // По умолчанию линейный датчик

    bool IsSyntheticApertureTransmit(int tx_index) const;
    bool IsPlaneWaveTransmit(int tx_index) const;
};

XRAD_END

#endif