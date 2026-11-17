#ifndef __antispeckle_utils_h
#define __antispeckle_utils_h

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif


#include <stdint.h>


#include "RASPMathFunction2D.h"
#include "RASPFixedPoint.h"
#include <RASPFunctors.h>
//---------------------------------------------------------------
//
//	вспомогательные функции обработки, копирования и т.п.
//
//---------------------------------------------------------------


XRAD_BEGIN


//	копирование из входного буфера в буфер обработки и обратно

template<class PROC_BUF, class IN_BUF>
void	ImportOriginalImage(PROC_BUF &a1, const IN_BUF &a2, IN_BUF::value_type intercept, float slope);

template<class IN_BUF, class PROC_BUF>
void	ExportProcessedImage(IN_BUF &a1, PROC_BUF &a2, IN_BUF::value_type intercept, float slope);
//	во второй функции экспортируемый аргумент не const, он разрушается
//	в процессе импорта




XRAD_END

#include "ImportExportSample.cc"

#endif //__antispeckle_utils_h

