#ifndef RawSFDataSource_h__
#define RawSFDataSource_h__

#include "LUT_8bit_interpolator.h"
#include "ProbeOptions/ApertureFocusingOptions.h"
//#include "Q:\projects\ATL\Sources\ProbeOptions/ApertureFocusingOptions.h"

/*!
 * \file RawSFDataSource.h
 * \date 2017/06/22 9:56
 *
 * \author kulberg
 *
 * \brief Источники данных синтетической апертуры (абстрактный класс)
 * 
 * В настоящее время имеются две реализации
 *
 * 1. Старые ATL RawSFData, включающие чтение формата SimIO
 * 2. Данные проекта TransEcho (Сономед).
 * 3. Результаты моделирования от Васюкова
 * Объединяются через базовый класс с пустыми виртуальными функциями.
 * Должны содержать информацию о фазированной решетке и параметры сигнала
 * Единственный доступ непосредственно к данным -- выбор текущих элементов приема/передачи,
 * получение интерполированного комплексного отсчета с заданной глубины.
 *
 * \note
*/

XRAD_BEGIN



class RawSFDataSource
{
public:
	RawSFDataSource()
	{
		calibrator_angle = radians(0);
		aberrator_thickness = cm(0);
	}
	~RawSFDataSource(){}

	virtual void Display() const = 0;
	virtual void LoadData() = 0;
//	virtual void LoadCalibrationPulses() = 0;

	virtual	string	GetFileName() const = 0;
	virtual	void	Init() = 0;
	//virtual	void	InitCalibrationPulses() = 0;
	

	virtual size_t NextNonDeadElement(size_t el_no){ return el_no; }
	virtual size_t PreviousNonDeadElement(size_t el_no){ return el_no; }
	virtual bool IsDeadElement(size_t /*el_no*/){ return false; }

//	access
	class	generic_complex_row_interpolator
	{
	public:
		virtual complexF64	in(double x) = 0;
	};

	typedef shared_ptr<generic_complex_row_interpolator> row_ptr;

	virtual row_ptr	GetRow(size_t tx, size_t rx) = 0;


//	others
	size_t n_tx_elements, n_rx_elements, samples_per_element, n_frames;
	size_t	first_raw_sample;
	size_t	last_raw_sample;

	physical_frequency	raw_signal_sample_rate;
	
	size_t	hilbert_samples;
	size_t	recommended_n_rays;
	physical_frequency	hilbert_signal_sample_rate;
	physical_angle	recommended_angle;// зависит от шага решетки
	physical_frequency	probe_carrier, probe_bandwidth;

	bool plane_wave_flag = false;

	physical_speed	sound_speed;
	physical_length	array_pitch;

	physical_angle	calibrator_angle;
	physical_length aberrator_thickness;
	TimeVector	aberration_profile;// профиль аберрации, измеренный с помощью калибровочного источника
	RealFunctionF32 amplitude_correction;

protected:
	template<class ARR3D>
	void	DisplayUtility(const ARR3D &data) const;



};



XRAD_END

#include "RawSFDataSource.cc"

#endif // RawSFDataSource_h__
