#ifndef CFDataPhaseAnalyzerTwinkling_h__
#define CFDataPhaseAnalyzerTwinkling_h__

/********************************************************************
	created:	2016/10/21
	created:	21:10:2016   15:32
	filename: 	q:\programs\ElastoGrafica\ElastoGrafica\CFDataPhaseAnalyzerTwinkling.h
	file path:	q:\programs\ElastoGrafica\ElastoGrafica
	file base:	CFDataPhaseAnalyzerTwinkling
	file ext:	h
	author:		kns
	
	purpose:	
*********************************************************************/

#error Возможности этого класса включены в CFDataPhaseAnalyzerCFM. Использовать его напрямую больше не следует

#include "CFDataPhaseAnalyzerCFM.h"
#include <XRADBasic/MathFunctionTypes2D.h>

XRAD_BEGIN

class CFDataPhaseAnalyzerCascillation : public CFDataPhaseAnalyzerCFM
{
	virtual void AnalyzeFrame(RealFunction2D_F32 &cascillation_map, RealFunction2D_F32 &mask) override;
	virtual wstring message(){ return L"Processing CFM frames, cascillation mode"; }
	virtual ElastoGrafica::cfm_mode algorithm() override { return ElastoGrafica::mode_cascillation; };

	virtual void CalculateMask() override;

	// следующие параметры перенесены из родительсвкого класса, инициализируются при чтении из файла настроек
public:
	double std_threshold = infinity();
	double re_im_cor_threshold = infinity(); // была инициализация в 0.336; Поставить в правильное место
	double cor_threshold = infinity();
	double	amplitude_threshold = infinity(); // была инициализация в 10

};

class CFDataPhaseAnalyzerOscillation : public CFDataPhaseAnalyzerCFM
{


	virtual void AnalyzeFrame(RealFunction2D_F32 &oscillation_map, RealFunction2D_F32 &mask) override;
	virtual wstring message(){ return L"Processing CFM frames, oscillation mode"; }
	virtual ElastoGrafica::cfm_mode algorithm() override { return ElastoGrafica::mode_oscillation; };

	virtual void CalculateMask() override {/*TODO*/};

	virtual void CalculateImaginarySignal();

public:
	double	prf;
	CFDataPhaseAnalyzerOscillation(){ prf = 1000; }//GetFloating("PRF", 1000, 150, 2000); это текст единственной инициализации этого параметра, которая была не на месте. При необходимости восстановить, только, естественно, не здесь
};

class CFDataPhaseAnalyzerCavitation : public CFDataPhaseAnalyzerCFM
{
	virtual void AnalyzeFrame(RealFunction2D_F32 &cavitation_map, RealFunction2D_F32 &mask) override;
	virtual wstring message(){ return L"Processing CFM frames, cavitation mode"; }
	virtual ElastoGrafica::cfm_mode algorithm() override { return ElastoGrafica::mode_cavitation; };

	virtual void CalculateMask() override;
};



XRAD_END

#endif // CFDataPhaseAnalyzerTwinkling_h__
