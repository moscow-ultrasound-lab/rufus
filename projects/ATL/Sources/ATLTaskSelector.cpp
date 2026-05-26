/*****************************************************************************
Modifications history :
Ver	Date		Author	Modification
1.0	10 Nov 93	Nicolas	Created
2.0	15 Feb 94	Nicolas	Modified
	27 Oct 97	Alexey Yelizarov // @ ACS
*****************************************************************************/

#include "pre.h"

#include "SignalProcessing/_rare_/AdaptiveFocusing/ExactPathfinder.h"
#include "SignalProcessing/_rare_/AdaptiveFocusing/CurvePathfinder.h"
#include "SignalProcessing/_rare_/AdaptiveFocusing/Pathfinder.h"
#include "SignalProcessing/_rare_/AdaptiveFocusing/NLPathfinder.h"
//#include "Holland.h"

#include "SignalProcessing/SyntheticApertureFocuser.h"
#include "SignalProcessing/SyntheticApertureFocuserAina.h"
#include "SignalProcessing/AberrationsCorrector.h"

#include "SignalProcessing/_rare_/Attenuator.h"
#include "SignalProcessing/ColourAnalyzer/ColourAnalyzer.h"
#include "SignalProcessing/RFDataToSimIOImport.h"

#if 0
//#include "Interpolation.h"
#include "Correlator.h"
//#include "MediaSimulation.h"
#include "ZPathfinder.h"
#include "NoiseTwister.h"
#include "EtalonCorrelator.h"
#include "WhiteAnalyzer.h"
#include "SubbandAnalyzer.h"
#include "FrequencyCompound.h"
#include "EntropyAnalyzer.h"
#endif//0
XRAD_USING

static	SectorData	*GetSFocusingTask()
{
	SectorData	*theResult;
	size_t	answer = GetButtonDecision("Choose focus algorithm", //3,
	{"Conventional", "Aberrations", "Exit"});

	switch(answer)
	{
		case	0:
			theResult = (SectorData *)new SyntheticApertureFocuser;
			break;
		case	1:
			theResult = (SectorData *)new AberrationsCorrector;
			break;
		default:
			theResult = NULL; // @ ACS e
	};
	return theResult;
}

static	SectorData	*GetPathfinderTask()
{
	SectorData	*theResult;
	size_t	answer = GetButtonDecision("Choose method:", //4,
	{"Fourier", "Exact", "Convolution", "Exit"});
	switch(answer)
	{
		case	0: theResult = (SectorData *)new Pathfinder;
			break;
		case	1: theResult = (SectorData *)new Exact_Pathfinder;
			break;
		case	2: theResult = (SectorData *)new Curve_Pathfinder;
			break;
		default:
		case	3: theResult = NULL;
			break;
	};
	return theResult;
}


SectorData	*GetSProcessingTask()
{
	SectorData	*result = NULL;
	size_t	answer = GetButtonDecision("Choose processing task:", //9,
										 {"No processing", "Em. paths",
										 "SResolution", "Speckle suppress", "Attenuation coef.",
										 "Media simulation",
										 "Colour analyze",
										 "Speed analyze",
										 "Exit"});

	switch(answer)
	{
		case	0:
			result = (SectorData *)new SignalProcessor;
			break;
		case	1:
			result = GetPathfinderTask();
			break;
		case	2:

			FatalError("Superresolution now unavailable");
			//if(Decide2("Choose SResolution type:",
			//	"Axial SResolution",
			//	"Lateral SResolution", 1))
			//	result = (SectorData *)new NL_Pathfinder;
			//else	result = (SectorData *)new AxialSR;

			break;
		case	3:
			result = (SectorData *) new ColourAnalyzer;
			break;
		case	4:
			result = (SectorData *) new Attenuator;
			break;
		case	5:
//			result = (SectorData *) new MediaSimulator;
			break;
		case	6:
			result = (SectorData *) new ColourAnalyzer;
			break;
		case	7:
			FatalError("Speed analyzer is obsolete!!!");
			break;
		case	8:
			result = NULL;
			break;
	};
	return result;
}

static	SectorData *GetGeneralTask()
{
	size_t	fileType = GetButtonDecision("What to process?", //3,
									   {"Focused SimIO data",
									   "Raw SF data",
									   "RF data import"});
	switch(fileType)
	{
		default:
		case 0:
			return GetSProcessingTask();
			break;

		case 1:
			return GetSFocusingTask();
			break;
		case 2:
			return (SectorData *)new RFDataToSimIOImport;
			break;
	}
}

void	check_clock_interval1()
{
//	int	n = 1e6;
	clock_t	c0 = clock(), c00 = c0;
	clock_t c1, dc;
	double	ave_c = 0;
	int	count = 0;


	for(int i = 0; i < 1e6; i++)
	{
		c1 = clock();
		dc = c1 - c0;
		if(dc)
		{
			c0 = clock();
			ave_c += dc;
			count++;
		}
	}
	ave_c /= count;

	ShowFloating("Time", (clock()-c00)/CLOCKS_PER_SEC);
	ShowSigned("Count", count);
	ShowFloating("average delay", ave_c);
	ShowSigned("Sizeof(clock_t)", sizeof(clock_t));
	ForceQuit(0);
}


void	check_clock_interval()
{
	int	n = 1000;
	clock_t	c0 = clock();
//	double	ave_c = 0;
	RealFunctionF32	f(n);

	for(int i = 0; i < n; i++) f[i] = clock() - c0;

	ShowString("Report:", ssprintf("Time = %g, average delay = %g", (clock()-c0)/CLOCKS_PER_SEC, (clock()-c0)/n));
	for(int i = 1; i < n; i++) f[i-1] -= f[i];
	f[n-1] = f[n-2];
	f *= -1.;
	DisplayMathFunction(f, 0, 1, "f");

	ForceQuit(0);
}

XRAD_BEGIN
void	TestMotionAnalyzer();

XRAD_END


int xrad::xrad_main(int, char** const)
{

	Init2DInterpolators(GUIProgressProxy());


	SectorData	*Signal;

	try
	{
		do
		{
			try
			{
				if(ControlPressed()) Signal = GetGeneralTask();
			//	if (CapsLock()) Signal = GetGeneralTask();
			//	else Signal = (SectorData *) new NoiseTwister;
			//	else Signal = (SectorData *) new ZPathfinder;
			//	else Signal = (SectorData *) new MediaSimulator;
			//	else Signal = (SectorData *) new ColourAnalyzer;
	//			else Signal = (SectorData *) new EtalonCorrelator;
	//			else Signal = (SectorData *) new SubbandAnalyzer;
	//			else Signal = (SectorData *) new WhiteAnalyzer;

		//		else Signal = (SectorData *) new AberrationsCorrector;
				else Signal = (SectorData *) new SyntheticApertureFocuserAina;
	//			else Signal = (SectorData *) new WhiteAnalyzer;
		//		else Signal = (SectorData *) new SpectromedImport;
	//			else Signal = (SectorData *) new RFDataToSimIOImport;

	//			else Signal = new FrequencyCompound;
	//			else Signal = (SectorData *) new EntropyAnalyzer;
	//			else Signal = (SectorData *)new SignalProcessor;
	//			else Signal = (SectorData *)new RFDataToSimIOImport;

	//			else Signal = (SectorData *) new SignalProcessor;
			//	else Signal = (SectorData *) new Pathfinder;
			//	else Signal = (SectorData *) new SyntheticApertureFocuser;
			//	else Signal = (SectorData *) new SpeedAnalyzer;
			//	else Signal = (SectorData *) new Attenuator;

				if(Signal)
				{
					Signal->Work();
					delete	Signal;
				}
			}
			catch (...)
			{
				Error(GetExceptionStringOrRethrow());
			}
		} while(YesOrNo("Process another file?", false));
	}
	catch(quit_application &ex)
	{
		return ex.exit_code;
	}
	catch (canceled_operation &)
	{
	}

	return 0;
}