#include "pre.h"
#include "ColourEncode.h"

XRAD_BEGIN


ColorSampleF64	EncodeXYZ(double B, double f0, double dw){
static	const	double	amp = 50;
	XYZColorSample	ls;
	ls.X = range(B, 0, 255)/2.55;
	ls.Y = range((f0+1)*amp, 0, 100);
	ls.Z = range((dw+1)*amp, 0, 100);
	
	return	ColorSampleF64(ls);
	}


ColorSampleF64	EncodeLAB(double B, double f0, double dw){
static	const	double	amp = 100;
static	LABColorSample	ls;
	ls.L = range(B, 0, 255)/2.55;
	ls.a = range(amp*f0, -100, 100);
	ls.b = range(amp*dw, -100, 100);
	
	return	ColorSampleF64(ls);
	}

ColorSampleF64	EncodeHLS(double B, double f0, double dw){
static	HLSColorSample	hs;
	hs.L = B;
	hs.H = std::atan2(f0,dw) - pi()/2;
	hs.S = std::pow((f0*f0 + dw*dw)/2, 0.5);
	
	return ColorSampleF64(hs);
	}


ColorSampleF64	EncodeHLS_polar(double B, double f0, double dw){
static	HLSColorSample	hs;
	hs.L = B;
	hs.H = f0*pi() - pi()/6;
	hs.S = square((dw + 1)/2);
	
	return ColorSampleF64(hs);
	}




ColorSampleF64	EncodeF0(double B, double f0, double){
static	const	double	angle_0 = 0;//-pi()/3 + pi()/2;
static	HLSColorSample	hs;
	hs.L = B;					
	hs.S = fabs(f0);
	hs.H = f0 > 0 ? angle_0 : angle_0 + pi();
	
	return ColorSampleF64(hs);
	}

ColorSampleF64	EncodeDW(double B, double , double dw)
	{
	double	angle_0 = -pi()/3;	
//	double	p = 3*dw;
	
static	HLSColorSample	hs;
	hs.L = B;
	hs.S = fabs(dw);
	hs.H = dw>0 ? angle_0 : angle_0 + pi();
	
	return ColorSampleF64(hs);
	}

void	BuildFDWHistogram(RealFunction2D_F32 &f, RealFunction2D_F32 &dw, ColorEncodeFunction *cf)
	{
	size_t	vs = f.vsize();
	size_t	hs = f.hsize();
	size_t	s = 256, s2 = s/2;
	RealFunction2D_F32	hist(s,s);
	ColorImageF64	combine(s,2*s);
	
	if(dw.vsize() != vs || dw.hsize() != hs)
		{
		Error("2D histogram : Different matrices!");
		return;
		}
	for(size_t i = 0; i < vs; i++)
		{
		for(size_t j = 0; j < hs; j++)
			{
			int	v = range(s2 - f.at(i,j)*s2, 0, s-1);
			int	h = range(s2 + dw.at(i,j)*s2, 0, s-1);
			hist.at(v,h) ++;
			}
		}
	hist.FilterGaussSeparate(1,1);
	NormalizeImage(hist, 0,1);
	for(size_t i = 0; i < s; i+=4) hist.at(i,s2) = hist.at(s2,i) = 1;

	for(size_t i = 0; i < s; i++)
		{
		double	v = -double(i-s2)/s2;
		for(size_t j = 0; j < s; j++)
			{
			double	h = double(j-s2)/s2;
			combine.at(i,j) = cf(255, v, h);
			combine.at(i,j+s) = cf(255.*pow(hist.at(i,j), 0.5f), v, h);
			}
		}
	DisplayMathFunction2D(combine, "combine_histogram.pct");
	}

void	TraditionalHistogram(RealFunction2D_F32 &f, RealFunction2D_F32 &w, RealFunction2D_F32 &, RealFunction2D_F32 &hist)
	{
	size_t vs = f.vsize();
	size_t hs = f.hsize();
	size_t s = hist.vsize();
	double	maxf = MaxValue(f);
	double	minf = MinValue(f);
	double	maxw = MaxValue(w);
	double	minw = MinValue(w);
	
	if(maxf == minf) minf++, maxf --;
	if(maxw == minw) minw++, maxw --;
	
	for(size_t i = 0; i < vs; i++)
		{
		for(size_t j = 0; j < hs; j++)
			{
			double	df = (f.at(i,j) - minf)/(maxf-minf);
			double	dw = (w.at(i,j) - minw)/(maxw-minw);
			
			int	v = range(df*s, 0, s-1);
			int	h = range(dw*s, 0, s-1);
			hist.at(v,h) ++;
			}
		}
	hist.FilterGaussSeparate(1,1);
	NormalizeImage(hist, 0,1);
	}


void	WeightedHistogram(RealFunction2D_F32 &f, RealFunction2D_F32 &w, RealFunction2D_F32 &b, RealFunction2D_F32 &hist)
	{
	size_t vs = f.vsize();
	size_t hs = f.hsize();
	size_t s = hist.vsize();
	RealFunction2D_F32	hist0(s,s);
			
	double	maxf = MaxValue(f);
	double	minf = MinValue(f);
	double	maxw = MaxValue(w);
	double	minw = MinValue(w);
	
	if(maxf == minf) minf++, maxf --;
	if(maxw == minw) minw++, maxw --;
	
	b -= MinValue(b);
	
	for(size_t i = 0; i < vs; i++)
		{
		for(size_t j = 0; j < hs; j++)
			{
			double	df = (f.at(i,j) - minf)/(maxf-minf);
			double	dw = (w.at(i,j) - minw)/(maxw-minw);
			
			int	v = range(df*s, 0, s-1);
			int	h = range(dw*s, 0, s-1);
			hist.at(v,h) +=b.at(i,j);
			hist0.at(v,h) ++;
			}
		}
	
	for(size_t i = 0; i < s; i++)
		{
		for(size_t j = 0; j < s; j++)
			{
			if(hist0.at(i,j)) hist.at(i,j) /= hist0.at(i,j);
			}
		}
	
	hist.FilterGaussSeparate(4,4);
	NormalizeImage(hist, 0,1);
	}



void	ComputeHistogramMaxValues(RealFunction2D_F32 &f, RealFunction2D_F32 &w, RealFunction2D_F32 &b, double &f0, double &w0)
	{
	size_t vs = f.vsize();
	size_t hs = f.hsize();
	size_t s = 256;
	RealFunction2D_F32	hist(s,s);

	double	maxf = MaxValue(f);
	double	minf = MinValue(f);
	double	maxw = MaxValue(w);
	double	minw = MinValue(w);
	
	if(w.vsize() != vs || w.hsize() != hs)
		{
		Error("2D histogram : Different matrices!");
		return;
		}
	
	TraditionalHistogram(f,w,b,hist);
//	WeightedHistogram(f,w,b,hist);
	
	size_t	f0_pos, w0_pos;
	MaxValue(hist, &f0_pos, &w0_pos);
	
	f0 = minf + f0_pos*(maxf-minf)/s;
	w0 = minw + w0_pos*(maxw-minw)/s;
	
	DisplayMathFunction2D(hist, "combine_histogram.pct");
	}





XRAD_END
