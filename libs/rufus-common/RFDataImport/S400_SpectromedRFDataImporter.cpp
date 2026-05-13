#include "pre.h"
#include <XRADBasic/ContainersAlgebra.h>
#include <RFDataImport/S400_SpectromedRFDataImporter.h>

//--------------------------------------------------------
//
//	original header conversion
//
//--------------------------------------------------------
#include <RFDataImport/S400_DryFrame.h>

namespace
{
using std::swap;
using std::fabs;

#if XRAD_ENDIAN==XRAD_BIG_ENDIAN
void	swap_bytes(UINT *bytes)
{
	uint8_t	*b = (uint8_t *)bytes;
	swap(b[0], b[3]);
	swap(b[1], b[2]);
}
void	swap_bytes(int *bytes)
{
	uint8_t	*b = (uint8_t *)bytes;
	swap(b[0], b[3]);
	swap(b[1], b[2]);
}
void	swap_bytes(float *bytes)
{
	uint8_t	*b = (uint8_t *)bytes;
	swap(b[0], b[3]);
	swap(b[1], b[2]);
}

void	swap_bytes(double *bytes)
{
	uint8_t	*b = (uint8_t *)bytes;
	swap(b[0], b[7]);
	swap(b[1], b[6]);
	swap(b[2], b[5]);
	swap(b[3], b[4]);
}

void	swap_bytes(tagDryDataInfo *s)
{

	swap_bytes(&s->uProbeID);
	swap_bytes(&s->uProbeuID);
	swap_bytes(&s->uDepthFrom); //Начальная глубина сканирования (мм).
	swap_bytes(&s->uDepthTo); //Глубина сканирования (мм).

	swap_bytes((int *)&s->Type);
	swap_bytes(&s->uNumOfBeams);
	swap_bytes(&s->uLineSize);

	swap_bytes(&s->Angle);		//Угол раскрыва датчика [radians] Convex only. Угол Стиринга для линейки [radians]
	swap_bytes(&s->Radius);		//Радиус внешней поверхности [mm] Convex only
	swap_bytes(&s->Width);		//Ширина линейки [mm]			  Linear only
	swap_bytes(&s->Frequency);
	swap_bytes(&s->FocusDepth);
}
#elif XRAD_ENDIAN==XRAD_LITTLE_ENDIAN
//does nothing
void	swap_bytes(tagDryDataInfo *){}
#else
#error define swap bytes procedure for this endianness
#endif

} //namespace

XRAD_BEGIN

SpectromedRFDataImporter::SpectromedRFDataImporter()
{
	sp.sfd.sample_rate = MHz(78.848);
	interleave_count = 1;
}
SpectromedRFDataImporter :: ~SpectromedRFDataImporter()
{
}

bool	SpectromedRFDataImporter::OpenRFData()
{
	tagDryDataInfo	settings;

	file_name = GetFileNameRead("Spectromed RF data", "*.dat");
	theFile.open(file_name, "rb");

	theFile.read(&settings, sizeof(tagDryDataInfo), 1);
	swap_bytes(&settings);

	sp.sfd.n_rays = settings.uNumOfBeams;
	sp.sfd.n_samples = settings.uLineSize;
	sp.sfroi.n_samples_to_skip_at_end = sp.sfroi.n_samples_to_skip_at_start = 0;

	size_t	settings_size = sizeof(tagDryDataInfo);
	size_t	file_size = theFile.size();
	size_t	data_size = file_size -  settings_size;

	frame_size_bytes = sp.sfd.n_rays*sp.n_samples_total()*sizeof(short);
	n_frames = data_size/frame_size_bytes;

	if(data_size%frame_size_bytes)
	{
//		ErrorFmt("Data file might be incorrect (%d bytes over)", data_size%frame_size_bytes);
	}

	sp.upo.SetCarrier(MHz(settings.Frequency));
//	static_filter.filter_f0 = carrier;
//	static_filter.filter_bandwidth = carrier;
//	filter_bandwidth = filter_f0;
//	#error upo есть, а где pfd?
	switch(settings.Type)
	{
		case tagDryDataInfo::eConvex:
			sp.upo.InitProbeConvex(mm(settings.Radius), radians(settings.Angle), sp.sfd.n_rays, 32);
//			upo.probe_type = convex_2D_probe;
//			upo.scanning_radius = mm(settings.Radius);///10;
			break;
		case tagDryDataInfo::eLinear:
			sp.upo.InitProbeLinear(mm(settings.Width), sp.sfd.n_rays, 32);
//			upo.probe_type = linear_2D_probe;
//			upo.scanning_radius = mm(0);///10;
			break;
		default:
			throw logic_error(ssprintf("SpectromedRFDataImporter::OpenRFData -- invalid settings.Type parameter = %d", int(settings.Type)));
			break;
	};



	if(settings.Width && settings.Angle)
	{
	// линейка со стирингом. обдумать этот случай более тщательно
		sp.pfd.SetFrameSector(mm(settings.Width), cm(sp.sfd.n_samples)/1024, -radians(settings.Angle)/2, radians(settings.Angle)/2);
//		sp.pfd.scanning_trajectory_length = mm(settings.Width);///10;
	}
	else if(!settings.Width && settings.Angle)
	{
	// конвексный датчик, длину траектории высчитываем из радиуса и угла
		sp.pfd.SetFrameSector(mm(settings.Radius)*settings.Angle,
							  cm(sp.sfd.n_samples)/1024,
							  -radians(settings.Angle)/2,
							  radians(settings.Angle)/2);

				  //		sp.pfd.scanning_trajectory_length = mm(settings.Radius)*sp.pfd.angle_range().radians();
	#pragma message ("see comment")
	// вообще неверно, так как не учтены пропущенные отсчеты (см. ниже)
	}
	else if(!settings.Angle && settings.Width)
	{
	// линейка
		sp.pfd.SetFrameRectangle(mm(settings.Width), cm(sp.sfd.n_samples)/1024);
	}
	else
	{
		ForceDebugBreak();
		throw logic_error("Invalid params in file header");
	}

// 	sp.pfd.start_angle = -radians(settings.Angle)/2;
// 	sp.pfd.end_angle = radians(settings.Angle)/2;
// 	sp.pfd.depth_range = cm(sp.sfd.n_samples)/1024;


//TODO это тоже учесть надо
	//z0 = mm(settings.uDepthFrom);///10;
	sp.sfroi.n_samples_skipped = mm(settings.uDepthFrom).cm() * 1024;

//	depth_range = float(settings.uDepthTo - settings.uDepthFrom)/10;
	dz = sp.pfd.depth_range()/sp.sfd.n_samples;
	z_window_center = sp.pfd.depth_range()/2;
	z_window_width = sp.pfd.depth_range();

	sp.mfo.realloc(1);
	sp.mfo[0] = mm(settings.FocusDepth);///10;

	frame_buffer.realloc(sp.sfd.n_rays, sp.sfd.n_samples);

	return true;
}



size_t SpectromedRFDataImporter::GetFrameOffset(size_t frame_no, size_t line_no) const
{
	size_t line_size = frame_size_bytes/sp.sfd.n_rays;
	return	sizeof(tagDryDataInfo) +
		(frame_no/interleave_count) * (frame_size_bytes*interleave_count) +
		(frame_no%interleave_count) * line_size +
		line_no*line_size*interleave_count;
}



void	SpectromedRFDataImporter::GetFrame(size_t frame_no)
{
	ComplexFunctionF32	read_buffer(sp.sfd.n_samples);
	for(size_t i = 0; i < sp.sfd.n_rays; i++)
	{
	// чтение по одной строке, так как допускаетсЯ interleave
		theFile.seek(GetFrameOffset(frame_no, i), SEEK_SET);
		//theFile.fread_numbers(frame_buffer[i], ioPCInt16);
		theFile.read_numbers(read_buffer, ioI16_LE);
		std::copy(read_buffer.begin() + sp.sfroi.n_samples_to_skip_at_start, read_buffer.end() - sp.sfroi.n_samples_to_skip_at_end, frame_buffer.row(i).begin());
	}
}

XRAD_END