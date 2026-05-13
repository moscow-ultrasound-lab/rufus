#include "pre.h"
#include <XRADBasic/Sources/Utils/StatisticUtils.h>
#include <XRADBasic/DataArrayIO.h>

#include <RFDataImport/S500_CFMFrameSet.h>
#include <RFDataImport/S500_CFMRawDataDisplay.h>

//#include <DopplerBasics/DuplexDisplayer.h>

#include "../ElastoGrafica\rufus-elasto.h"
#include "AnalyzeCFMFrameSet.h"

/*

содержимое файла заголовка для примера

NumOfFrames=66
RawFrameSize=1522824
HeaderSize=20
NumOfBBeams=192
SizeofBBeamAtSamples=519
NumOfCFShots=9
NumOfSweeps=10
BeamsInSweep=5
SizeofCFMBeamAtSamples=198
FirstScanCFMBeam=67
CFMDensity=2
NumOfCFMBeams=50
NumOfFirstCFMSample=29
CFMFilterOrder=29

*/




XRAD_USING;

S500_CFMParamFileData	GetElastoFileParams()
{
	wstring	filename = GetFileNameRead(L"Choose data files", saved_default_value, L"*.par"); //GetFileNameRead(L"Choose data files", L"*.par");

	S500_CFMParamFileData	params;
	params.Init(filename);

	return params;
}


S500_CFMFrameSet GetFrames()
{
	S500_CFMParamFileData	params = GetElastoFileParams();
	size_t	start_frame = 0;//GetUnsigned("First frame to analyze", 0, 0, params.NumOfFrames);//0
	size_t	end_frame = params.NumOfFrames;//GetUnsigned("Last frame to analyze", params.NumOfFrames, start_frame, params.NumOfFrames);//params.NumOfFrames;
	S500_CFMFrameSet	frames(params, start_frame, end_frame);
	frames.ReadAllFrames(false);
	return frames;
}



int xrad::xrad_main(int, char** const)
{
	XRAD_USING

	try
	{
		while (true)
		{
			try
			{
				S500_CFMFrameSet frames = GetFrames();
				AnalyzeCFMFrameSet(frames);
			}
			catch(canceled_operation&) {}
			catch(quit_application& ex) { throw ex; }
			catch(...) { Error(GetExceptionString()); }

			XRAD_ASSERT_THROW_EX(YesOrNo(L"Обработать еще один файл?", saved_default_value), canceled_operation);
		}
	}
	catch (canceled_operation &) {}
	catch (quit_application &) {}
	catch (...) { Error(GetExceptionString()); }
	return 0;
}


XRAD_BEGIN

XRAD_END