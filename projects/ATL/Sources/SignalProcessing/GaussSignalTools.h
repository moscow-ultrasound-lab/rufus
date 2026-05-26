#ifndef __gauss_signal_tools_h
#define __gauss_signal_tools_h


XRAD_BEGIN

typedef enum	{
	spatial, temporal
	}complexSignalCoord;

void	NormalizeSpectrum(ComplexFunction2D_F32 &procBuffer,
			complexSignalCoord coord,
			double shift,
			double broadFactor = 1);
//	после обработки усредненный спектр сигнала по всем строкам (rows)
//	становитсЯ точно гауссовским. мат. ожидание (несущаЯ частота) смещаетсЯ
//	пропорционально параметру shift, помноженному на среднюю ширину спектра.
//	дисперсиЯ (ширина спектра) умножаетсЯ на broadFactor

XRAD_END

#endif //__gauss_signal_tools_h
