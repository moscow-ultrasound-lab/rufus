#ifndef __etalon_correlator_h
#define __etalon_correlator_h

#include "ColourEncode.h"
#include "SignalProcessing/SignalProcessor.h"
#include "LayersContainer.h"
#include "RangeSpectrumAnalyzer.h"

#include "EtalonSetProcessing.h"

XRAD_BEGIN


//-------------------------------------------------------
//
//	EtalonCorrelator
//
//-------------------------------------------------------

class	EtalonCorrelator : public RangeSpectrumAnalyzer//, public EtalonSet
	{
	PARENT(RangeSpectrumAnalyzer);
	size_t etalon_halfwidth;
	size_t etalon_length;
		
//	EtalonSet	etalon_set_1;
//	EtalonSet	etalon_set_2;
	
	RealFunction2D_F32	correlation_map;
	ColorEncodeFunction	*color_encode;

	
//	void	OptimizeEtalons();

	void	PrepareEtalonSet(EtalonSet &etalon_set, const char *comment);
	void	MeasureEtalons();
	void	PerformDecomposition(const ComplexMatrixF32 &eigen_vectors, DataArray<RealFunction2D_F32> &sb);

	void	ComputeDecompositionPlane(size_t sub, const ComplexVectorF32 &basis_vector, ComplexFunction2D_F32 &decomposition);
	
	void	PrepareResultingImages();
	
	void	PhaseAnalyze();
	
//	void	NormalizeSpectrum();
	
	void	Colourize();
	void	PrepareCorrelationBasis(size_t hw, size_t es, ComplexMatrixF32& eigen_vectors);

public:

	void	InitWork();
	void	Batch();
	void	EndWork();

	EtalonCorrelator();
	virtual ~EtalonCorrelator();
	};

XRAD_END


#endif //__etalon_correlator_h
