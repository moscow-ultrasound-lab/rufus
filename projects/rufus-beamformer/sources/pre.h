#ifndef __pre_h_precompiled
#define __pre_h_precompiled


#ifdef XRAD__pre_h 
#error File "pre.h" must be included once!!!
#endif
#define XRAD__pre_h 

#define __pre_h_first

#ifndef _CRT_SECURE_NO_WARNINGS
	#define _CRT_SECURE_NO_WARNINGS// это определение уже задано в XRAD/Core.h, но там оно появляется поздновато. Особенность включения SimIO.h
#endif


#ifndef	STANDARD_INCLUDE_H
	#define	STANDARD_INCLUDE_H
	#define	INCLUDE_SF_DATA_SPECIES
	#include "SimIOHeaders/SimIO.h"
#endif //STANDARD_INCLUDE_H

#undef	__pre_h_first

#include <XRADBasic/Core.h>
#if XRAD_CFG_ARRAYS_INTERACTIONS_VER >= 2
// Режим совместимости с новой версией библиотеки
#include <XRADGUI/XRAD.h>

#include <XRADBasic/Math.h>
#include <XRADBasic/ContainersAlgebra.h>
#include <XRADBasic/DataArrayIO.h>
#include <XRADBasic/Sources/Utils/PhysicalUnits.h>

#include <XRADGUI/Sources/GUI/GraphSet.h>
#include <XRADGUI/Sources/GUI/MathFunctionGUIMD.h>

#include <XRADSystem/CFile.h>
#include "XRADBasic/Sources/ScanConverter/ScanConverter.h"


XRAD_BEGIN

// Старые функторы должны быть заменены новыми
template <class, class>
using cabs_functor = Functors::absolute_value;
template <class, class>
using fabs_functor = Functors::absolute_value;
template <class, class>
using absolute_value_functor = Functors::absolute_value;

// Функтор cabs2 должен быть заменен на лямбду:
// [](const complexF32 &v) { return cabs2(v); }
// С лямбдой не работает старая версия библиотеки.
template <class Result, class Value>
struct cabs2_functor
{
	Result operator() (const Value &v) const { return cabs2(v); }
};

// Замена имен алгоритмов
#define AA_1D_OpEq Apply_AA_1D_F2

XRAD_END
#else
XRAD_BEGIN
// Режим совместимости с изменениями, необходимыми для использования новой версии библиотеки
namespace Functors
{
template <class T>
const T &assign_f1(const T &v) { return v; }
}
XRAD_END
#endif

#endif //XRAD__pre_h _precompiled
