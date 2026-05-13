#include "pre.h"
#include <var_a.h>
using namespace Photomod;


#include "ColourAnalyzer.h"

/*static	SectorData *GetGeneralTask()
	{
	enum { bExit, bSimIO, bSpectromed};
	int fileType = GetButtonDecision("What to process?",
			QButton("Sim/IO files - Color Analyze", bSimIO)+
			QButton("Spectromed files", bSpectromed)+
			QButton("Skip", bExit));
	switch(fileType)
		{
		case bSimIO:
			_ftype = 'rays';
			return new ColourAnalyzer;
		case bSpectromed:
			return new SpectromedImport;
		}
	return 0;
	}*/

static	var_a<SectorData*> GetGeneralTask32()
	{
	enum { bExit, bSimIO};
	int fileType = GetButtonDecision("Select processing task",
			QButton("Color Analyze", bSimIO)+
			QButton("Exit", bExit));
	switch(fileType)
		{
		case bSimIO:
			_ftype = 'rays';
			return new_a<ColourAnalyzer>();
		}
	return 0;
	}

void	main()
	{
	Init_XRAD();

	Init_FFTs(1024,16);

	Init2DInterpolators();

	for(;;)
		{
		try{
			var_a<SectorData*> Signal = GetGeneralTask32();
			if( !Signal)
				return;
			Signal->Work();
			}
		catch(...)
			{
			Error("Something happened");
			}
		}

	Finish_XRAD();
	}