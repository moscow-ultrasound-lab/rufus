#ifndef __colour_encode_h
#define __colour_encode_h

#include <XRADBasic/Sources/SampleTypes/ColorSample.h>
#include <XRADBasic/Sources/SampleTypes/LABColorSample.h>
#include <XRADBasic/Sources/SampleTypes/HLSColorSample.h>

XRAD_BEGIN

ColorSampleF64	EncodeLAB(double l, double f0, double dw);
ColorSampleF64	EncodeXYZ(double B, double f0, double dw);
ColorSampleF64	EncodeHLS(double l, double f0, double dw);
ColorSampleF64	EncodeF0(double l, double f0, double dw);
ColorSampleF64	EncodeDW(double l, double f0, double dw);

typedef ColorSampleF64 ColorEncodeFunction(double, double, double);

void	BuildFDWHistogram(RealFunction2D_F32 &f, RealFunction2D_F32 &dw, ColorEncodeFunction *cf);
void	ComputeHistogramMaxValues(RealFunction2D_F32 &f, RealFunction2D_F32 &w,  RealFunction2D_F32 &b, double &maxf, double &maxw);
//void	WeightedHistogram(RealFunction2D_F32 &f, RealFunction2D_F32 &w, RealFunction2D_F32 &b);


XRAD_END


#endif //__colour_encode_h
