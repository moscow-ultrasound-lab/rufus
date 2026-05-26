//#include "Q:\projects\StringsPhantom\StringsPhantom\StringsPhantom\pre.h"
#include "Q:\projects\StringsPhantom\StringsPhantom\pre.h"
//#include "S500_CFMFrameSet.h"
//#include "S500_CFMRawDataDisplay.h"
#include "GenerateFigures.h" 
#include "CreateVolumes.h" 
//#include "pre.h"
//#include "EMD.h"
//#include "WallFilters.h"
#include <XRADBasic/Sources/Utils/LeastSquares.h> //введена для dummy

XRAD_USING;



int xrad::xrad_main(int, char** const)
{
	XRAD_USING

		try
	{

		//120x68x170 мм размеры печатной области
		size_t	n_slices = 200; //z
		//n_slices = GetUnsigned("n_slices", n_slices, 1, 15 * n_slices);
		n_slices = GetUnsigned("n_slices", saved_default_value, 1, 15 * n_slices);
		size_t	w = 3840;// 2160; //y
		size_t	h = 2160;// 3840; //x
		//if(CapsLock())
		{
			//w = GetUnsigned("y width of the printing area (px)", w, 1, 15 * w);
			w = GetUnsigned("y width of the printing area (px)", saved_default_value, 1, 15 * w);
			
			//h = GetUnsigned("x width of the printing area (px)", h, 1, 15 * h);
			h = GetUnsigned("x width of the printing area (px)", saved_default_value, 1, 15 * h);
		}
		
		printf("Print area\nz height (slices) = %zu\n", n_slices);
		printf("x width (px) = %zu\n", h);
		printf("y width (px) = %zu\n\n", w);

		size_t answer(0);
		while (true)
		{
			answer = Decide("Create Phantom", { "Create Sponge (Volume 1)", "Create Sphere (Volume 2)","Create Empty (Volume 3)", "Combine 3 Volumes", "Exit" });
			switch (answer)
			{
			case 0:
			{
				try
				{
					TestSpongeSimulation(n_slices, w, h);
				}
				catch (canceled_operation) {}
				catch (...) { Error(GetExceptionStringOrRethrow()); }
			}
			break;

			case 1:
			{
				try
				{
					CreateSphere(n_slices, w, h);
				}
				catch (canceled_operation) {}
				catch (...) { Error(GetExceptionStringOrRethrow()); }
			}
			break;
			
			case 2:
			{
				try
				{
					CreateEmptyVolume(n_slices, w, h);
				}
				catch (canceled_operation) {}
				catch (...) { Error(GetExceptionStringOrRethrow()); }
			}
			break;
			
			case 3:
			{
				try
				{
					CombineVolumes(n_slices, w, h);
				}
				catch (canceled_operation) {}
				catch (...) { Error(GetExceptionStringOrRethrow()); }
			}
			break;

			default:
				throw;

			}
		}
		return 0;
	}
	catch (canceled_operation&) {}
	catch (quit_application&) {}
	catch (exception& ex) { ShowString("An exception occurred:", ex.what()); }
	catch (...) { Error("Unknown exception"); }
	return 0;

}



XRAD_BEGIN
XRAD_END