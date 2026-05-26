#ifndef __scan_params
#define __scan_params

#include <XRADBasic/Sources/ScanConverter/ScanAreaGeometry.h>
#include "ProbeOptions/UltrasonicProbeOptions.h"
#include "ProbeOptions/ApertureFocusingOptions.h"

XRAD_BEGIN
//--------------------------------------------------------------------------------------------------
//
//	минимальный набор параметров длЯ описаниЯ одного кадра уз изображениЯ
//
//	параметры для скан-конвертера, общие для линейного, конвексного и фазированного датчиков
//	скан-конвертер может работать при любых сочетаниях без дополнительного указания
//	типа датчика. прочие параметры легко по ним вычисляются
//
//		   	array_length	end_angle-start_angle
//	convex		!=0		!=0
//	linear		!=0		0
//	sector		0		!=0
//
//	радиус конвекса R = array_length / (end_angle-start_angle);
//
//	имеетсЯ неслучайное пересечение с классом ScanConverterOptions. возможно, его надо оформить общим базовым классом
//	отличие принципиальное: в скан-конвертере глубина задана в см и нет привЯзки к отсчетам. здесь основной
//	способ заданиЯ глубины через отсчеты и скорость звука
//	NB 2013_03_22: так, похоже, было. если где-то и осталось, нужно вернуть к см.
//

// #pragma message ("следующий класс собран вчерне, отладить тщательно")

struct ScanParams
	{
	physical_speed	sound_speed;

//	physical_frequency carrier_frequency; // не здесь этому место. bandwidth еще добавить (шутка)

	SampledFrameDimensions sfd;
	UltrasonicProbeOptions upo;
	MultifocusOptions	mfo;
	PhysicalFrameDimensions pfd;	
	SampledFrameROI	sfroi;
	
	
	ScanParams &operator=(const ScanParams &sp)
		{
		sound_speed = sp.sound_speed;
//		carrier_frequency = sp.carrier_frequency;

		sfd = sp.sfd;
		upo = sp.upo;
		mfo.MakeCopy(sp.mfo);
		pfd = sp.pfd;	
		sfroi = sp.sfroi;
		
		return *this;
		}
	
	//-------------------------------------------------------------------------------
	
	size_t n_samples_total() const
		{
		return sfd.n_samples + sfroi.n_samples_to_skip_at_start + sfroi.n_samples_to_skip_at_end;
		}

	ScanParams():sound_speed(cm_sec(1.54e5))/*, carrier_frequency(MHz(0))*/{};
	
	physical_length	calculate_min_depth() const
		{
		return cm(sfroi.n_samples_skipped + sfroi.n_samples_to_skip_at_start)*sound_speed.cm_sec()/(2.*sfd.sample_rate.Hz());
		}
	physical_length	calculate_max_depth() const
		{
		return cm(sfroi.n_samples_skipped + sfroi.n_samples_to_skip_at_start + sfd.n_samples)*sound_speed.cm_sec()/(2.*sfd.sample_rate.Hz());
		}
	};


//
//--------------------------------------------------------------------------------------------------

XRAD_END

#endif //__scan_params

