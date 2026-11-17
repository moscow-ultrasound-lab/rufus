/*!
	\file RASPProcessTomogram.cpp
	\date 2017/04/25 12:09
	\author kulberg
	\brief Вызов функций обработки RASP3CT
*/
#include "pre.h"
#include "RASPProcessSlices.h"

#include "../../RASP3CT/RASP3CTLib/RASP3CTLib.h"

#include <XRADGUI/XRAD.h>

XRAD_BEGIN

using namespace RASP3CTLibrary;

size_t	GetRASPPresetNo()
{
	vector<wstring> preset_description = GetPresetDecisionNames(false);
	preset_description.push_back(L"Не обрабатывать");
	size_t result;
	try
	{
		result = Decide(L"Выберите степень шумоподавления RASP", preset_description);
	}
	catch(canceled_operation)
	{
		result = preset_description.size()-1;
	}
	if(result == preset_description.size()-1)
		result = RASPPresetNo_None;
	return result;
}

RealFunctionMD_F32	RASPProcessSlices(const RealFunctionMD_F32 &slices_p, const point3_F64 &scales_p, size_t preset_no, ProgressProxy pp)
{
	try
	{
		auto result(slices_p);
		RASPProcessSlices(result, result, scales_p, preset_no, pp);
		return result;
	}
	catch(...)
	{
		string	sizes_message = slices_p.n_dimensions() == 3 ?
			ssprintf("sizes=(%zu,%zu,%zu), ",
					EnsureType<size_t>(slices_p.sizes(0)),
					EnsureType<size_t>(slices_p.sizes(1)),
					EnsureType<size_t>(slices_p.sizes(2))) :
			ssprintf("tomogram dimensions = %zu, ",
					EnsureType<size_t>(slices_p.n_dimensions()));

		string	scales_message = ssprintf("scales=(%g,%g,%g)",
				EnsureType<double>(scales_p.z()),
				EnsureType<double>(scales_p.y()),
				EnsureType<double>(scales_p.x()));

		string message = "RASP filtering error:\nData: " +
			ssprintf("preset=%zu ", EnsureType<size_t>(preset_no)) +
			sizes_message +
			scales_message +
			"\nError message:\n" +
			tabify(GetExceptionStringOrRethrow());

		throw runtime_error(message);
	}
}

XRAD_END
