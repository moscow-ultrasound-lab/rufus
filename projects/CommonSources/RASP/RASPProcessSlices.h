/*!
	\file RASPProcessTomogram.h
	\date 2017/04/25 12:08
	\author kulberg
	\brief Вызов функций обработки RASP3CT
*/
#ifndef RASPProcessTomogram_h__
#define RASPProcessTomogram_h__

#include "RASPProcessSlicesBatch.h"

XRAD_BEGIN

//! \brief Выдать диалог запроса номера пресета
//!
//! \return Возвращает номер одного из "рабочих" пресетов или RASPPresetNo_None.
size_t	GetRASPPresetNo();
//TODO реорганизовать номера пресетов.

RealFunctionMD_F32	RASPProcessSlices(const RealFunctionMD_F32 &slices_p, const point3_F64 &scales_p, size_t preset_no, ProgressProxy pp);

XRAD_END

#endif // RASPProcessTomogram_h__
