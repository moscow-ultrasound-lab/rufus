#include "pre.h"
#include <ComplexFunction.h>
#include "Correlator.h" 
#include <deCorrelationMatrix.h>
#include <ImagePoint.h>



XRAD_BEGIN

void	Correlator :: InitWork()
	{
	Display("Initially");
	}

void	Correlator :: EndWork()
	{
	Display("Finally");
	}

Correlator :: Correlator():SignalProcessor(){}
Correlator :: ~Correlator(){}




/*
void	Correlator :: Batch()
	{
	long	i, j, k;
	ComplexFunctionF32	corrFunction(n_rays);
		
	ComplexFunction2D_F32	procBuffer(n_rays, n_samples);
	ComplexFunction2D_F32	resultBuffer(n_rays, n_samples);
	ComplexFunction2D_F32	extremBuffer(n_rays, n_samples);
	
	long	nExtrems0 = 4;


	for(Start_Ray(); End_Ray(); Next_Ray())
		{
		procBuffer[ray].realloc(n_samples);
		resultBuffer[ray].realloc(n_samples);
		for(j = 0; j < n_samples; j++)
			{
			procBuffer[ray][j] = CurrentRay[0][j];
			}
		resultBuffer[ray].CopyData(procBuffer[ray]);
		extremBuffer[ray].CopyData(procBuffer[ray]);
		}
	
	ISignal_Write();
	
	resultBuffer.Display("Before", 45);
		
	long	mL = 10, sL = 30;

	SetAngleUnits(DEGREES);
	float	dAngle = (end_Angle-start_Angle)/n_rays;
	float	mainLobe = GetFloating("Main lobe width, degrees", 3, 3*dAngle, (end_Angle-start_Angle)/2);
	float	sideLobe = GetFloating("Main lobe width, degrees", 9, mainLobe, (end_Angle-start_Angle)/2);
	float	resultMainLobeWidth = mainLobe/(8*dAngle);
	
	Show_Double("Result main lobe", resultMainLobeWidth);
	
	
	//Show_Double("resultMainLobe", resultMainLobeWidth);
	
	mL = mainLobe/dAngle;
	sL = sideLobe/dAngle;
	printf("\nmain lobe = %lu, side lobe = %lu\n", mL, sL);
	fflush(stdout);

	float	slLevel = 10; //approx side lobe level, dB
	
	pointsArray extBoth(nExtrems0), extLeft(nExtrems0), extRight(nExtrems0);
	long	nLeftExts = 0, nRightExts = 0, nBothExts = 0;
	
	deCorrelationMatrix maxFinder(2*sL+1, 8);
	StartProgress("Searching extremes", n_rays);
	for(i = 0; i < n_rays; i++)
		{
		for(j = 0; j < n_samples; j++)
			{
			cImagePoint	centrePoint(0, i,j);
			procBuffer.GetSegmentFromCentre(centrePoint, maxFinder);
			extremType extKind = maxFinder.hasExtreme(mL, sL, 10);
			switch(extKind)
				{
				case bothExtreme:
					extBoth[nBothExts] = cImagePoint(fabs(procBuffer.at(i,j)), i, j);
					nBothExts++;
					extremBuffer.at(i,j) /= 10;
					if(nBothExts >= extBoth.getSize()) extBoth.resize(nBothExts*2);
					break;
				case leftExtreme:
					extLeft[nLeftExts] = cImagePoint(fabs(procBuffer.at(i,j)), i, j);
					nLeftExts++;
					if(nLeftExts >= extLeft.getSize()) extLeft.resize(nLeftExts*2);
					extremBuffer.at(i,j) *= 10;
					break;
				case rightExtreme:
					extRight[nRightExts] = cImagePoint(fabs(procBuffer.at(i,j)), i, j);
					nRightExts++;
					if(nRightExts >= extRight.getSize()) extRight.resize(nRightExts*2);
					extremBuffer.at(i,j) *= 10;
					break;
				default:
					break;
				}
			}
		NextProgress();
		}
	EndProgress();
	
	extLeft.resize(nLeftExts);
	extRight.resize(nRightExts);
	extBoth.resize(nBothExts);
	
	extremBuffer.Display("Extremes", 50);
	
	if(nBothExts) extBoth.reorder_descent();
	if(nLeftExts) extLeft.reorder_descent();
	if(nRightExts) extRight.reorder_descent();
	
	
	
	long	ex;
	//long	sW = 2*sL + 1;
	long	sW = 2*sL + 1;
	long	sH = 16; //обЯзательно степень двойки, так как возможно бпф!!!
				
	StartProgress("Decorrelating", nBothExts + nRightExts + nLeftExts);
	deCorrelationMatrix	segment(sW, sH);
	for(ex = 0; ex < nBothExts; ex++)
		{
		resultBuffer.GetSegmentFromCentre(extBoth[ex], segment);
		segment.deCorrelateCenter(resultMainLobeWidth, bothExtreme);
		resultBuffer.PutSegmentToCentre(extBoth[ex], segment);
		NextProgress();
		}
	
	for(ex = 0; ex < nLeftExts; ex++)
		{
		resultBuffer.GetSegmentFromCentre(extLeft[ex], segment);
		segment.deCorrelateCenter(resultMainLobeWidth, leftExtreme);
		resultBuffer.PutSegmentToCentre(extLeft[ex], segment);
		NextProgress();
		}

	for(ex = 0; ex < nRightExts; ex++)
		{
		resultBuffer.GetSegmentFromCentre(extRight[ex], segment);
		segment.deCorrelateCenter(resultMainLobeWidth, rightExtreme);
		resultBuffer.PutSegmentToCentre(extRight[ex], segment);
		NextProgress();
		}
	EndProgress();
	
	
	
	for(Start_Ray(); End_Ray(); Next_Ray())
		{
		for(j = 0; j < n_samples; j++)
			{
			CurrentRay[0][j] = resultBuffer.row(ray)[j];
			}
		}
	
	resultBuffer.Display("After",45);	
	}
*/




void	deCorrelateMatrix(ComplexFunction2D_F32 &M);

float	displayV, displayH;
float	pixelsPerCm = 30;
float	pixelsPerDegree = 5;


void	Correlator :: Batch()
	{
	long	i, j, k;
	ComplexFunctionF32	corrFunction(n_rays);
		
	ComplexFunction2D_F32	procBuffer(n_rays, n_samples);
	
	
	long	nExtrems = 0;

	//SetAngleUnits(DEGREES);
	//SetDepthUnits(CENTIMETRES);

	displayH = 2*(end_Angle - start_Angle)*pixelsPerDegree/72;
	displayV = 2*(r_max - r_min);

	//displayV = displayH = 0;

	printf("\ndisplayV = %g, displayH = %g\n", displayV, displayH);

	for(Start_Ray(); End_Ray(); Next_Ray())
		{
		for(j = 0; j < n_samples; j++)
			{
			procBuffer[ray][j] = CurrentRay[0][j];
			}
		}
	
	
	ISignal_Write();
	procBuffer.Display("Before", displayV, displayH);		
	deCorrelateMatrix(procBuffer);
	procBuffer.Display("After", displayV, displayH);

	for(Start_Ray(); End_Ray(); Next_Ray())
		{
		for(j = 0; j < n_samples; j++)
			{
			CurrentRay[0][j] = procBuffer[ray][j];
			}
		}
	}
XRAD_END
