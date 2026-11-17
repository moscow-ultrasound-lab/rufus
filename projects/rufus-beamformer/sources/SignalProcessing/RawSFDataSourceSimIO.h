#ifndef RawSFDataSourceSimIO_h__
#define RawSFDataSourceSimIO_h__

#include "SignalProcessing/RawSFDataSource.h"
#include "LUT_8bit_interpolator.h"

/*!
 * \file RawSFDataSourceSimIO.h
 * \date 2017/06/22 9:56
 *
 * \author kulberg
 *
 * \brief Реализация источника данных синтетической апертуры. Старые ATL RawSFData, включающие чтение формата SimIO
 *
 * \note
*/

XRAD_BEGIN




class RawSFDataSourceSimIO : public RawSFDataSource
{
	PARENT(RawSFDataSource);
public:
	RawSFDataSourceSimIO(){ file = NULL; }
	~RawSFDataSourceSimIO();

	class	row_t : public generic_complex_row_interpolator
	{
	public:
		LUT_8bit_interpolator	data;
		virtual complexF64	in(double x){ return data.interpolate(x); };

		template<class IT>
		void	copy(IT i1, IT i2){ XRAD_ASSERT_THROW_M(i2-i1==ptrdiff_t(data.size()), invalid_argument, "RawSFDataSource::row_t::copy, invalid data size"); std::copy(i1, i2, data.begin()); };
	};

	virtual row_ptr	GetRow(size_t tx, size_t rx)
	{
		row_t	*result = new row_t;
		data.GetRow(result->data, {tx, rx, slice_mask(0)});
		return row_ptr(result);
	};


private:
	FILE*	file;
	string	filename;
	DataArray<size_t>	dead_elements_list;

	virtual void	Display() const { parent::DisplayUtility(data); }
	virtual void	LoadData();
	virtual void	DeadElements();
//	virtual void	LoadOtherSource(RawSFDataSource	&original);

//	virtual void	SetActiveElements(size_t tx_el, size_t rx_el);

	virtual size_t NextNonDeadElement(size_t el_no);
	virtual size_t PreviousNonDeadElement(size_t el_no);
	virtual bool IsDeadElement(size_t el_no);


	typedef DataArrayMD<MathFunction2D<LUT_8bit_interpolator>> data_volume_type;
	typedef	typename data_volume_type::slice_type slice_type;
	typedef	typename data_volume_type::row_type row_type;

	
	//	complexF64	in(double x){ return current.interpolate(x); }

//	DataArrayMD<MathFunction2D<LUT_8bit_interpolator>> data;
	data_volume_type data;
//	LUT_8bit_interpolator	current;//temp

	virtual	string	GetFileName() const;
	virtual	void	Init();

};


XRAD_END

#endif // RawSFDataSourceSimIO_h__
