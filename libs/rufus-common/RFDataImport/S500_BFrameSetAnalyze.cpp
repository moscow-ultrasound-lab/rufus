#include "pre.h"

#include "S500_BFrameSetAnalyze.h"

#include <XRADBasic/Sources/DataArrayIO/DataArrayIOFunctions.h>

//TODO исправить путь
#include <q:/Projects/CommonSources/Utils/DetectRawFrameSize.h>//in CommonSources
#include <XRADSystem/System.h>
#include <XRADSystem/CFile.h>

//	открывает raw файл, содержащий серию изображений неизвестного размера.
//	находит стыки между кадрами и определяет по ним размер кадра



XRAD_BEGIN


void	LoadS500_BFrameSet(RealFunctionMD_UI8 &data, FILE *file)
{
	size_t	data_size = filesize(file);
	RealFunctionUI8 raw_frames(data_size);	// все кадры объединены в одномерном масиве
	fread_numbers(raw_frames, file, ioUI8);

	size_t	n_samples = DetectRawFrameSize(raw_frames, 1, 1);
	size_t	n_rays = DetectRawFrameSize(raw_frames, n_samples, 1);

	size_t	n_frames = data_size/(n_rays*n_samples);

	data.realloc({n_frames, n_rays, n_samples});

	fseek(file, 0, SEEK_SET);
	for(size_t i = 0; i < n_frames; ++i)
	{
		RealFunction2D_UI8	slice;
		data.GetSlice(slice, {i, slice_mask(0), slice_mask(1)});
		for(size_t j = 0; j < n_rays; ++j)
		{
			fread_numbers(slice.row(j), file, ioUI8);
		}
	}
}





XRAD_END