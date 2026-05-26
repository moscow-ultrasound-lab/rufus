// file RASPProcessTomogramBatch.cpp
//--------------------------------------------------------------
#include "pre.h"

#include "RASPProcessSlicesBatch.h"

#include "../../RASP3CT/RASP3CTLib/RASP3CTLib.h"

XRAD_BEGIN

using namespace RASP3CTLibrary;

//--------------------------------------------------------------

vector<wstring> GetRASPPresetNames()
{
	return GetPresetDecisionNames(false);
}

//--------------------------------------------------------------

vector<string> GetRASPPresetIds()
{
	return GetPresetDecisionIds(false);
}

//--------------------------------------------------------------

void	RASPProcessSlices(RealFunctionMD_F32 &output_slices,
		const RealFunctionMD_F32 &src_slices, const point3_F64 &scales,
		size_t preset_no,
		ProgressProxy pp)
{
	if(preset_no == RASPPresetNo_None)
		return;

	RealFunctionMD_I16 filtered;
	MakeCopy(filtered, src_slices);

	size_t z_size = filtered.sizes(0);
	size_t y_size = filtered.sizes(1);
	size_t x_size = filtered.sizes(2);
	int16_t	*data_processed = &filtered.at({0,0,0});

	InitLibrary();
	try
	{
		LoadData(data_processed, z_size, y_size, x_size);
		ApplyLibrarySettings(preset_no, scales[0], scales[1], scales[2], false);

		RandomProgressBar	progress(pp);
		progress.start("Filtering data");
		ProcessData(progress.subprogress(0, 1));
		progress.set_position(1);
		progress.end();
	}
	catch (...)
	{
		FinishLibrary();
		throw;
	}
	FinishLibrary();
	MakeCopy(output_slices, filtered);
}

//--------------------------------------------------------------

XRAD_END
