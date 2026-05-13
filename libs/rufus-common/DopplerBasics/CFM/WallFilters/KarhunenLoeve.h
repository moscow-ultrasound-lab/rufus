#ifndef KarhunenLoeve_h__
#define KarhunenLoeve_h__

/********************************************************************
	created:	2016/06/20
	created:	20:6:2016   15:43
	filename: 	Q:\programs\DopplerTest\sources\KarhunenLoeve.h
	file path:	Q:\programs\DopplerTest\sources
	file base:	KarhunenLoeve
	file ext:	h
	author:		kns
	
	purpose:	
*********************************************************************/

#include <XRADBasic/MathMatrixTypes.h>
#include <XRADBasic/MathFunctionTypesMD.h>

XRAD_BEGIN

void ComputeCorrelationMatrix(ComplexMatrixF32 &R, ComplexFunctionMD_F32 &frame_by_shots, size_t sweep_start, size_t sweep_end);

XRAD_END

#endif // KarhunenLoeve_h__
