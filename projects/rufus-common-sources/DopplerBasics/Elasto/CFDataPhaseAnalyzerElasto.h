#ifndef FrameElastoAnalyzer_h__
#define FrameElastoAnalyzer_h__

/********************************************************************
	created:	2016/10/20
	created:	20:10:2016   14:10
	filename: 	q:\programs\ElastoGrafica\ElastoGrafica\FrameElastoAnalyzer.h
	file path:	q:\programs\ElastoGrafica\ElastoGrafica
	file base:	FrameElastoAnalyzer
	file ext:	h
	author:		kns
	
	purpose:	
*********************************************************************/

#include <DopplerBasics/CFDataPhaseAnalyzer.h>

XRAD_BEGIN

class CFDataPhaseAnalyzerElasto : public CFDataPhaseAnalyzer
{
	void CalculateMask(RealFunction2D_F32 &elastogram);

	void AcquirePhaseElastoOriginal();

	void PostfilterElastogram();
	void NormalizeDynamics(RealFunction2D_F32 &elastogram);

	double m_average_frame_offset;

	virtual wstring message() override { return L"Processing CFM frames, elastography mode"; }
	virtual void CalculateMask() override{ m_mask.fill(1); }

public:
//	CFDataPhaseAnalyzerElasto() { algorithm = e_elasto; }
	virtual cfm_mode algorithm() override { return cfm_mode::elastography; }
	virtual void	AnalyzeFrame(RealFunction2D_F32 &result_elastogram, RealFunction2D_F32 &result_mask) override;
	virtual double average_frame_offset() const override { return m_average_frame_offset; }
};

XRAD_END

#endif // FrameElastoAnalyzer_h__
