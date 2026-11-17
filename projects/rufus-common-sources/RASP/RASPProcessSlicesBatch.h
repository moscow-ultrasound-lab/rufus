// file RASPProcessTomogramBatch.h
//--------------------------------------------------------------
#ifndef __RASPProcessTomogramBatch_h
#define __RASPProcessTomogramBatch_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>
#include <XRADBasic/MathFunctionTypesMD.h>

XRAD_BEGIN

//--------------------------------------------------------------

//! \brief Особый номер пресета: "Не обрабатывать". Используется в GetRASPPresetNo()
constexpr size_t RASPPresetNo_None = (size_t)-1;

//! \brief Получить названия пресетов human readable, локализованные
vector<wstring> GetRASPPresetNames();

//! \brief Получить идентификаторы пресетов (постоянные)
vector<string> GetRASPPresetIds();

//! \note Если номер пресета равен RASPPresetNo_None, output_slices не заполняется.
void	RASPProcessSlices(RealFunctionMD_F32 &output_slices,
		const RealFunctionMD_F32 &src_slices, const point3_F64 &scales,
		size_t preset_no,
		ProgressProxy pp);

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif // __RASPProcessTomogramBatch_h
