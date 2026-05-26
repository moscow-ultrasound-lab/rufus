#ifndef __CFDataAmplitudeAnalyzer_h
#define __CFDataAmplitudeAnalyzer_h

//------------------------------------------------------------------
//
//	created:	2021/03/12	10:11
//	filename: 	CFDataAmplitudeAnalyzer.h
//	file path:	q:\Projects\CommonSources\DopplerBasics
//	author:		kns
//	
//	purpose:	
//
//------------------------------------------------------------------

#include <DopplerBasics/CFDataPhaseAnalyzer.h>//"CFDataPhaseAnalyzer.h"

namespace xrad
{

//	Данны алгоритм помещает в карту ЦДК амплитуду ЦДК сигналов.
//	Он не используется в медицинских задачах. Нужен для коррекции датасетов:
//	если окно ЦДК неправильно локализовано относительно данных B, можно внести поправки,
//	используя наложение этих данных на B-картину

class CFDataAmplitudeAnalyzer : public CFDataPhaseAnalyzer
{
	virtual void CalculateMask() override;;
	virtual wstring message() override { return L"Processing CFM frames, Amplitude analyze mode"; }
	//virtual void PrepareBuffers(const index_vector& cfm_frames_sizes, size_t in_beams_in_sweep, size_t in_ray_header_size_in_bytes) override;
	//virtual void ResetFrameAveraging() override;

public://temporary
	PARENT(CFDataPhaseAnalyzer);



public:
//	void AcquirePhaseCFM();
	virtual cfm_mode algorithm() override { return cfm_mode::cfm; };
	virtual	 void AnalyzeFrame(RealFunction2D_F32& result_flow_map, RealFunction2D_F32& result_mask) override;
};


}//namespace xrad

#endif //__CFDataAmplitudeAnalyzer_h
