#ifndef RawSFDataSourcePICMUS_h__
#define RawSFDataSourcePICMUS_h__
#include "SignalProcessing/RawSFDataSource.h"
#include <vector>
#include <string>

XRAD_BEGIN

//bool use_plane_wave_mode(false);

class RawSFDataSourcePICMUS : public RawSFDataSource
{
	PARENT(RawSFDataSource);
public:
	RawSFDataSourcePICMUS() : angle_index(0), array_pitch_mm(0) {}
	virtual ~RawSFDataSourcePICMUS() {}

	// ќб€зательные методы базового класса RawSFDataSource
	virtual void   Display() const override;
	virtual void   LoadData() override;
	virtual void   Init() override;
	virtual string GetFileName() const override;

	// —трока (A-скан) одного канала: комплексный интерпол€тор, как у других источников
	class row_t : public generic_complex_row_interpolator
	{
	public:
		ComplexFunctionF32 data;
		virtual complexF64 in(double x) override
		{
			return data.in(x, &interpolators::complex_icubic);
		}
	};
	// tx здесь Ч номер угла (плоской волны), rx Ч номер приЄмного элемента.
	virtual row_ptr GetRow(size_t tx, size_t rx) override;

	// ƒоступ к углам плоских волн (нужно дл€ PlaneWaveFocuserPICMUS).
	//size_t NumAngles() const { return angles_rad.size(); }
	//double GetAngle(size_t idx) const { return angles_rad.at(idx); }   // радианы

private:
	void LoadHDF5Metadata(const std::string& filename);
	void LoadHDF5Data(const std::string& filename);

	// ¬нутреннее хранилище: комплексный многомерный контейнер, как в других источниках RUFUS
	// ќси: [angle_index, rx_element, sample]
	typedef ComplexFunctionMD_F32 data_volume_type;
	data_volume_type data;
	string filename;

	int    angle_index;   // не используетс€ дл€ загрузки (грузим все углы), оставлено дл€ отладки/Display()

	// параметры, прочитанные из PICMUS
	int    n_angles = 0;          // число углов (первое измерение /channel_data/data)
	//std::vector<double> angles_rad;   // угол (в радианах) дл€ каждой плоской волны; индекс == tx в GetRow()

	float  picmus_sampling_freq = 0;
	float  picmus_sound_speed = 0;
	float  array_pitch_mm = 0;
};

XRAD_END

#endif // RawSFDataSourcePICMUS_h__