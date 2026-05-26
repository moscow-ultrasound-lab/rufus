#ifndef RawSFDataSourceTransEcho_h__
#define RawSFDataSourceTransEcho_h__

#include "SignalProcessing/RawSFDataSource.h"
//#include "Q:\projects\ATL\Sources\SignalProcessing/RawSFDataSource.h"

/*!
 * \file RawSFDataSourceTransEcho.h
 * \date 2017/06/22 9:56
 *
 * \author kulberg
 *
 * \brief Реализация источника данных синтетической апертуры. Данные TransEcho (Sonomed)
 *
 * \note
*/

XRAD_BEGIN


class RawSFDataSourceTransEcho : public RawSFDataSource
{
	PARENT(RawSFDataSource);

	//	typedef ComplexFunctionMD_I32F data_volume_type;
	typedef ComplexFunctionMD_F32 data_volume_type;
	typedef	typename data_volume_type::slice_type slice_type;
	typedef	typename data_volume_type::row_type row_type;

public:
	RawSFDataSourceTransEcho(){}
	~RawSFDataSourceTransEcho(){}

	virtual void Display() const { parent::DisplayUtility(data); }
	virtual void LoadData();
	virtual void	LoadCalibrationPulses();
	RealFunctionF32	aberration_profile_function;
//	virtual void SetActiveElements(size_t tx, size_t rx);

	virtual	string	GetFileName() const;
	virtual	void	Init();
	virtual	void	InitCalibrationPulses();
	virtual void	Normalize();
	virtual void	HighPassFilter();
	virtual void	LoadOtherSource(RawSFDataSource	&original);
	virtual physical_frequency EstimateCentralFrequency();

	//	access

	class	row_t : public generic_complex_row_interpolator
	{
	public:
		typename ComplexFunctionF32	data;
		virtual complexF64	in(double x){ return data.in(x, &interpolators::complex_icubic); };
		template<class IT>
		void	copy(IT i1, IT i2){ assert_throw(i2-i1==ptrdiff_t(data.size()), invalid_argument("RawSFDataSource::row_t::copy, invalid data size")); std::copy(i1, i2, data.begin()); };
	};

	virtual row_ptr	GetRow(size_t tx, size_t rx)
	{
		row_t	*result = new row_t;
		data.GetRow(result->data, {tx, rx, slice_mask(0)});
		return row_ptr(result);
	};

	const slice_type &GetCalibrationPulses() const { return calibration_pulses; }

// 	virtual size_t NextNonDeadElement(size_t el_no){ return el_no; }
// 	virtual size_t PreviousNonDeadElement(size_t el_no){ return el_no; }
// 	virtual bool IsDeadElement(size_t /*el_no*/){ return false; }
	

private:
	
	void	LoadInfFile(string inf_filename);
//	void	LoadCalibrationPulses();
	//void	LoadOtherSource(RawSFDataSource	&original_source);
	

	shared_cfile	file;
	size_t number_of_frames = 1;
	string	filename;
	data_volume_type	data;
	slice_type	calibration_pulses;
	void	ComputeOffsetsCrossCorrelation(/*RealFunctionF64 &offsets, RealFunctionF64 &amplitude_corrections*/);
};


XRAD_END

#endif // RawSFDataSourceTransEcho_h__
