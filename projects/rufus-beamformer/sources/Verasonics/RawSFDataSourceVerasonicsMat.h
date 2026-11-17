// RawSFDataSourceVerasonicsMat.h
#ifndef RawSFDataSourceVerasonicsMat_h__
#define RawSFDataSourceVerasonicsMat_h__

#include "SignalProcessing/RawSFDataSource.h"
#include <vector>
#include <string>

XRAD_BEGIN

class RawSFDataSourceVerasonicsMat : public RawSFDataSource
{
    PARENT(RawSFDataSource);

public:
    RawSFDataSourceVerasonicsMat() : frame_index(0), array_pitch_mm(0), n_samples_per_frame(0) {}
    virtual ~RawSFDataSourceVerasonicsMat() {}

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

    bool IsLinearArray() const { return is_linear_array; }
    void SetLinearArray(bool linear) { is_linear_array = linear; }

private:
    void LoadMatData(const std::string& filename);
    void HilbertTransform(); 

    typedef ComplexFunctionMD_F32 data_volume_type;
    data_volume_type data;

    string filename;
    int frame_index;

    float verasonics_sampling_freq;
    float verasonics_center_freq;
    float verasonics_sound_speed;
    float array_pitch_mm;
    int n_samples_per_frame; 
    std::string transducer_name;

    bool is_linear_array = true;
};

XRAD_END

#endif