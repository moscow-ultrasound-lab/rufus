#ifndef __etalon_set_processsing_h
#define __etalon_set_processsing_h

#include "SignalProcessing/SignalProcessor.h"

XRAD_BEGIN

//-------------------------------------------------------
//
//	класс, содержащий в себе двумерный эталон длЯ
//	коррелЯционной обработки.
//
//	data -- собственно данные длЯ коррелЯтора;
//
//	sub, ray, sample, range -- местонахождение эталона
//	в текущем наборе ультразвуковых сигналов.
//	если эталон получен не из текущего набора, эти
//	параметры устанавливаютсЯ в -1;
//
//	наследованное от complexF32 значение содержит коэффициент
//	коррелЯции данного эталона с измерЯемыми данными, либо
//	с прочими эталонами из набора. используетсЯ длЯ оценки
//	качества данного эталона; может применЯтьсЯ в алгоритмах
//	сортировки
//	
//-------------------------------------------------------


struct	EtalonContainer : public complexF32
{
	size_t	ray, sample, range, sub;
	ComplexFunction2D_F32 data;

	EtalonContainer()
	{
		ray=sample=range=sub=0;
	}

	EtalonContainer(int)
	{
		ray=sample=range=sub=0;
	}

	EtalonContainer(const EtalonContainer &e) : data(e.data), complexF32(e)
	{
		ray = e.ray;
		sample = e.sample;
		range = e.range;
		sub = e.sub;
	}

	EtalonContainer &operator= (const EtalonContainer &e)
	{
		re = e.re;
		im = e.im;
		ray = e.ray;
		sample = e.sample;
		range = e.range;
		sub = e.sub;
		data.MakeCopy(e.data);
		return *this;
	}

};

inline EtalonContainer zero_value(const EtalonContainer&){ return EtalonContainer(); }

//-------------------------------------------------------
//
//	EtalonSet
//	набор эталонов, взЯтых с какого-то характерного участка изображениЯ.
//	класс включает средства редактированиЯ, чтениЯ/записи
//	и коррелЯционного анализа эталонов (построение базиса
//	карунена-лоэва
//
//-------------------------------------------------------

class	EtalonSet
{

private:
	ColorScanConverter	etalon_displayer;
//	GrayScanConverter	etalon_displayer;
	void	BuildEtalonDisplayer(SignalProcessor &sp);

	void	PutEtalon(size_t	etalon_no, size_t	ray_no, size_t	range_no, size_t	sub_no, SignalProcessor &sp);
	void	RemoveEtalon(size_t	etalon_no);

public:
	void	SetEtalonDimensions(size_t	length, size_t	halfwidth = 0);

	size_t	etalon_width;
	size_t	etalon_length;
	size_t	etalon_halfwidth;	// = etalon_width/2 - 1;
					// часто используетсЯ, все времЯ приходилось
					// вычислЯть. пусть хранитсЯ постоЯнно


	size_t	n_etalon_volumes;
		// количество интервалов спектрального анализа в одном луче
	float	etalon_volume_step;
		// шаг между ними, задаетсЯ с субпиксельной точностью

	size_t	n_etalons;
		// общее количество эталонов в наборе
	DataArray<EtalonContainer> etalons;

	// methods
	void	EditEtalons(SignalProcessor &sp);

	bool	SaveEtalons(const char *filename);
	bool	LoadEtalons(const char *filename);
	void	ShowEtalons(const char *title, SignalProcessor &sp);
	bool	FindEtalon(size_t	ray_no, size_t	range_no);
	void	AnalyzeEtalons(SignalProcessor &sp);

	void	ComputeBasis(ComplexMatrixF32 &eigen_vectors);
	void	CalculateCorrelationMap(SignalProcessor &sp, RealFunction2D_F32	&correlation_map);
	void	OptimizeEtalons(SignalProcessor &sp, RealFunction2D_F32	&correlation_map);
};

XRAD_END


#endif //__etalon_set_processsing_h
