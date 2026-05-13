#ifndef __lc_simulation_h
#define __lc_simulation_h

#include "LinearSignal.h"
#include "MovableMedia.h"

class LC_Simulator : public LinearSignal
	{
	protected:
		static	physical_frequency	omega0;
		static	physical_frequency	halfWidth;

		void	Init(physical_frequency carrierFrequency, physical_length apertSize);
		void	PrepareData();

		physical_length	ComputeDistance(short element, physical_length deltaX, physical_length pointZ);

	//	константы
		void	GenerateInitPulse();
		void	ComputeDistances();
		void	ComputeLens();
		void	ComputeApodization(const abstract_window_function &wf = constant_win());
		
		ComplexFunctionF32 initPulse; //спектр начального импульса
		RealFunction2D_F32	distances; //дальности от элементов решетки до точек среды
		RealFunctionF32	lens;	// фиксированнаЯ линза
		RealFunctionF32	apod;	// функциЯ аподизации

	public:
		void	Compute();
		void	ComputeNearZone();
		void	SimulateSpeckle();

		void	SimulateSpeckleExact();
		ComplexFunctionF32	FocusRayExact(int ray, scatterMedia &);
	};


#endif //__lc_simulation_h
