#ifndef RawSFDataSourceINM_h__
#define RawSFDataSourceINM_h__

#include "SignalProcessing/RawSFDataSource.h"
#include <XRADSystem/TextFile.h>

/*!
 * \file RawSFDataSourceINM.h
 * \date 2017/06/22 9:56
 *
 * \author kulberg
 *
 * \brief Реализация источника данных синтетической апертуры. Данные INM (моделирование в ИВМ РАН)
*
 * \note
*/

XRAD_BEGIN


class RawSFDataSourceINM : public RawSFDataSource
{
	PARENT(RawSFDataSource);
public:
	RawSFDataSourceINM(){}
	~RawSFDataSourceINM(){}

	virtual void Display() const { parent::DisplayUtility(data); }
	virtual void LoadData();
//	virtual void SetActiveElements(size_t tx, size_t rx);

	virtual	string	GetFileName() const;
	virtual	void	Init();

	//	access

	class	row_t : public generic_complex_row_interpolator
	{
	public:
		LUT_8bit_interpolator	data;
		virtual complexF64	in(double x){ return data.interpolate(x); };
// 		typename ComplexFunctionI32	data;
// 		virtual complexF64	in(double x){ return data.in(x, &interpolators::complex_icubic); };

//		virtual complexF64	in(double x){ return data.in(x, &interpolators::complex_sinc); };//&interpolators::icubic
//		virtual complexF64	in(double x){ return data.in(x, &interpolators::sinc); };//&interpolators::icubic
//		virtual complexF64	in(double x){ return data.in(x, &interpolators::icubic); };//&interpolators::icubic
//		virtual complexF64	in(double x){ return data.in(x, &interpolators::linear); };//&interpolators::icubic
// 		virtual complexF64	in(double x){ return data[range(int(x), 0, data.size()-1)]; };//&interpolators::icubic
		template<class IT>
		void	copy(IT i1, IT i2){ assert_throw(i2-i1==ptrdiff_t(data.size()), invalid_argument("RawSFDataSource::row_t::copy, invalid data size")); std::copy(i1, i2, data.begin()); };
	};

	virtual row_ptr	GetRow(size_t tx, size_t rx)
	{
		row_t	*result = new row_t;
		data.GetRow(result->data, {tx, rx, slice_mask(0)});
		return row_ptr(result);
	};



// 	virtual size_t NextNonDeadElement(size_t el_no){ return el_no; }
// 	virtual size_t PreviousNonDeadElement(size_t el_no){ return el_no; }
// 	virtual bool IsDeadElement(size_t /*el_no*/){ return false; }

private:

	typedef DataArrayMD<MathFunction2D<LUT_8bit_interpolator>> data_volume_type;
	typedef	typename data_volume_type::slice_type slice_type;
	typedef	typename data_volume_type::row_type row_type;

	vector<text_file_reader>	files;
	vector<wstring>	filenames;
	wstring	foldername;
	data_volume_type	data;
};





XRAD_END

#endif // RawSFDataSourceINM_h__
