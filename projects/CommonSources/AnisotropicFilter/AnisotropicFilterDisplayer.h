/*
	created:	6:2:2017   12:16
*/
#ifndef AnisotropicFilterDisplayer_h__
#define AnisotropicFilterDisplayer_h__

#include "AnisotropicFilter1D.h"
#include <GraphSet.h>

XRAD_BEGIN

class AnisotropicFilterDisplayer
{
	GraphSet masks_gs;
	GraphSet blur_result_gs;
	GraphSet differences_gs;
	GraphSet result_gs;
	GraphSet parametric_gs;

public:
	AnisotropicFilterDisplayer() :
		blur_result_gs("Blur result", "x", "y"),
		differences_gs("Differences", "x", "y"),
		result_gs("Result", "x", "y"),
		masks_gs("Masks", "x", "y"),
		parametric_gs("Parametric", "f'", "f''")
	{

	}

	void	ShowBlurResult(const AnisotropicFilter1D &filter, bool stop);
	void	ShowDifferences(const AnisotropicFilter1D &filter, bool stop);
	void	ShowMasks(const AnisotropicFilter1D &filter, bool stop);
	void	ShowClearedSignal(const AnisotropicFilter1D &filter, bool stop);
};

XRAD_END

#endif // AnisotropicFilterDisplayer_h__
