#ifndef RawSFDataSourceSimulation_h__
#define RawSFDataSourceSimulation_h__

#include "SignalProcessing/RawSFDataSource.h"

XRAD_BEGIN


class RawSFDataSourceSimulation : public RawSFDataSourceTransEcho
{
	PARENT(RawSFDataSourceTransEcho);
public:
	RawSFDataSourceSimulation() {}
	~RawSFDataSourceSimulation() {}
	virtual	void	Init();
	virtual void LoadData();

	class	row_t : public generic_complex_row_interpolator
	{
	public:
		typename ComplexFunctionF32	data;
		virtual complexF64	in(double x) { return data.in(x, &interpolators::complex_icubic); };
		template<class IT>
		void	copy(IT i1, IT i2) { XRAD_ASSERT_THROW_M(i2 - i1 == ptrdiff_t(data.size()), invalid_argument, "RawSFDataSource::row_t::copy, invalid data size"); std::copy(i1, i2, data.begin()); };
	};

	virtual row_ptr	GetRow(size_t tx, size_t rx)
	{
		row_t	*result = new row_t;
		data.GetRow(result->data, { tx, rx, slice_mask(0) });
		return row_ptr(result);
	};

private:
	typedef ComplexFunctionMD_F32 data_volume_type;
	typedef	typename data_volume_type::slice_type slice_type;
	typedef	typename data_volume_type::row_type row_type;

	shared_cfile	file;

	string	filename;
	data_volume_type	data;
//	slice_type	calibration_pulses;
};


XRAD_END

#endif // RawSFDataSourceSimulation_h__
