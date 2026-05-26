//---------------------------------------------------------------------------

#ifndef LinearSignalProcessingH
#define LinearSignalProcessingH
//---------------------------------------------------------------------------

#include <NonDiffraction.h>
#include "LinearSignal.h"

class	LinearSignalProcessor : public LinearSignal
	{
	public:

		void	AddNoise(float noiseLevelDB);
		void	Focus();
		void	OriginalFocusAlgorithm();
		void	SynthAperture();
		
		void	ExportNonDiffractionSetup(non_diffraction_setup &s, int n_focusing_intervals);
	};


#endif
