#include "pre.h"
#include "EtalonSetProcessing.h"
#include <XRADBasic/Sources/Utils/Crayons.h>
#include <MatrixAlgorithms/EigenVectorTools.h>

XRAD_BEGIN

void	EtalonSet::SetEtalonDimensions(size_t	length, size_t	halfwidth)
{
	etalon_length = length;
	etalon_halfwidth = halfwidth;
	etalon_width =2*etalon_halfwidth + 1;

	// данные эталонов обнулЯютсЯ, так как вызов данной функции
	// означает изменение размера эталона
	n_etalons = 0;
	etalons.realloc(0);
}


void	EtalonSet::BuildEtalonDisplayer(SignalProcessor &sp)
{

	etalon_displayer.realloc(sp.n_rays, n_etalon_volumes);
	sp.ExportScanConverterOptions(etalon_displayer);
	etalon_displayer.SetFlip(true);

	size_t	middle_sub = sp.n_frames/2;
//	sp.SetCurrentFrame(middle_sub);
	for(size_t ray = 0; ray < sp.n_rays; ray++)
	{
		ComplexFunctionF32	CurrentRay;
		sp.focused_data.GetRow(CurrentRay, {middle_sub, ray, slice_mask(0)});
//		sp.SetCurrentRay(ray);
		for(size_t range = 0; range < n_etalon_volumes; range++)
		{
			size_t	s0 = range*etalon_volume_step;
			for(size_t sample = 0; sample < etalon_volume_step; sample++)
			{
				etalon_displayer.at(ray, range) += cabs(CurrentRay[s0 + sample]);
			}
		}
	}

	LogCompress(etalon_displayer);
	NormalizeImage(etalon_displayer, 0, 255);
}

void	EtalonSet::EditEtalons(SignalProcessor &/*sp*/)
{
#if 0
	extern	Rect	*imageROI;
	const	size_t	pixel_step_v = etalon_length/5;
	const	size_t	pixel_step_h = 3;

	size_t	n_etalon_requests = GetSigned("N etalon requests", 1, 1, 100);

	BuildEtalonDisplayer(sp);
	size_t	n_rays = sp.n_rays;
	size_t	n_frames = sp.n_frames;

	char	window_title[256];

	size_t	e;
	for(e = 0; e < n_etalons; e++)
	{
		if(in_range(etalons[e].ray, etalon_halfwidth, n_rays-etalon_halfwidth-1) && in_range(etalons[e].range, 0, n_etalon_volumes-1))
			etalon_displayer[etalons[e].ray + etalon_halfwidth][etalons[e].range] = crayons::blue();
		// помечаем существующие эталоны синим цветом
	}

	for(size_t er = 0; er < n_etalon_requests; er++)
	{

		sprintf(window_title, "Etalon request # %d of %d", er+1, n_etalon_requests);

		etalon_displayer.DisplayRaster(window_title);
		// здесь выделЯем область интереса
		if(!imageROI->top && !imageROI->left)
		{
			Show_String("Attention", "Etalon has not been selected");
		}
		else if(CapsLock())
		{
			size_t	i = 0;
			while(i <= n_etalons-1 && n_etalons)
			{
				row_col_coord	rc = etalon_displayer.GetRowColCoords(etalons[i].ray, etalons[i].range);
				size_t	bottom = max(imageROI->top, imageROI->bottom);
				size_t	top = min(imageROI->top, imageROI->bottom);
				size_t	left = min(imageROI->left, imageROI->right);
				size_t	right = max(imageROI->left, imageROI->right);


				if(rc.row_f < bottom && rc.row_f > top && rc.col_f < right && rc.col_f > left)
				{
					etalon_displayer[etalons[i].ray + etalon_halfwidth][etalons[i].range] = crayons::brown();
					RemoveEtalon(i);
					e--;
				}
				else i++;
			}
		}
		else
		{
			size_t	n1 = labs(imageROI->top - imageROI->bottom)/pixel_step_v + 1;
			size_t	n2 = labs(imageROI->left - imageROI->right)/pixel_step_h + 1;

			n_etalons += (n1*n2);

			etalons.resize(n_etalons);
			for(size_t i = 0; i < n1; i++)
			{
				for(size_t j = 0; j < n2; j++)
				{
					size_t	row = min(imageROI->top, imageROI->bottom) + i*pixel_step_v + RandomUniform(-pixel_step_v/2, pixel_step_v/2);
					size_t	col = min(imageROI->left, imageROI->right) + j*pixel_step_h + RandomUniform(-pixel_step_h/2, pixel_step_h/2);

				//	size_t	row = min(imageROI->top, imageROI->bottom) + i*pixel_step_v + (j%4)*(pixel_step_v/4);
				//	size_t	col = min(imageROI->left, imageROI->right) + j*pixel_step_h + (i%2)*(pixel_step_h/2);
					ray_sample_coord	ray_range = etalon_displayer.GetRaySampleCoords(row, col);

//					if(in_range(etalons[e].ray, etalon_halfwidth, n_rays-etalon_halfwidth-1) && in_range(etalons[e].range, 0, n_etalon_volumes-1))
//					{
					if(in_range(ray_range.ray_f, etalon_halfwidth, n_rays-etalon_halfwidth-1) && in_range(ray_range.sample_f, 0, n_etalon_volumes-1))
					{
						PutEtalon(e, ray_range.ray_f, ray_range.sample_f, n_frames/2, sp);
						etalon_displayer[etalons[e].ray + etalon_halfwidth][etalons[e].range] = crayons::green();

						e++;
					}
					else n_etalons--;
				}
			}
			etalons.resize(n_etalons);
		}
		if(!n_etalons && er == n_etalon_requests-1)
		{
			Error("You have deleted all etalons. Another request will be added");
			n_etalon_requests++;
		}
	}


	ShowEtalons("Etalons mapping", sp);

//	if(YesOrNo("Save etalons?", NO))
//		{
//		char	filename[256];
//		if(Put_File_Name(filename, "Etalons file")) SaveEtalons(filename);
//		}
#endif //0
}

bool	EtalonSet::FindEtalon(size_t	ray_no, size_t	range_no)
{
	for(size_t e = 0; e < n_etalons; e++)
	{
		if(etalons[e].ray == ray_no && etalons[e].range == range_no) return true;
	}
	return false;
}


void	EtalonSet::PutEtalon(size_t	etalon_no, size_t	ray_no, size_t	range_no, size_t	sub_no, SignalProcessor &sp)
{
	size_t	n_rays = sp.n_rays;
	size_t	n_samples = sp.n_samples;
//	size_t	n_frames = sp.n_frames;
	etalons[etalon_no].ray = range(ray_no - etalon_halfwidth, 0, n_rays-1-etalon_halfwidth);
	etalons[etalon_no].range = range(range_no, 0, n_etalon_volumes-1);;
	etalons[etalon_no].sample = range(etalons[etalon_no].range*etalon_volume_step - etalon_length/2, 0, n_samples-etalon_length);

	etalons[etalon_no].sub = sub_no;
	etalons[etalon_no].data.realloc(etalon_width, etalon_length);


	complexF32	e0(0); //константа

//	sp.SetCurrentFrame(etalons[etalon_no].sub);

	for(size_t k = 0; k < etalon_width; k++)
	{
		ComplexFunctionF32	CurrentRay;
		sp.focused_data.GetRow(CurrentRay, {etalons[etalon_no].sub, range(etalons[etalon_no].ray + k, 0, n_rays-1), slice_mask(0)});
//		sp.SetCurrentRay(range(etalons[etalon_no].ray + k, 0, n_rays-1));
		for(size_t l = 0; l < etalon_length; l++)
		{
			size_t	s = range(etalons[etalon_no].sample + l, 0, n_samples-1);
			e0 += (etalons[etalon_no].data.at(k,l) = CurrentRay[s]);
		}
	}
	etalons[etalon_no].data -= e0/(etalon_width*etalon_length);// центрируем

	float	e_dispersion(0); // дисперсиЯ
	for(size_t k = 0; k < etalon_width; k++)
	{
		for(size_t l = 0; l < etalon_length; l++)
		{
			e_dispersion += cabs2(etalons[etalon_no].data.at(k,l));
		}
	}

//	etalons[etalon_no].data /= cabs(etalons[etalon_no].data.MaxValue());
	etalons[etalon_no].data /= sqrt(e_dispersion); // нормализуем по дисперсии
}


void	EtalonSet::RemoveEtalon(size_t	etalon_no)
{
	for(size_t e = etalon_no; e < n_etalons-1; e++)
	{
		etalons[e] = etalons[e+1];
	}
	n_etalons--;
	etalons.resize(n_etalons);
}



void	EtalonSet::ShowEtalons(const char* title, SignalProcessor &sp)
{
	BuildEtalonDisplayer(sp);
	for(size_t e = 0; e < n_etalons; e++)
	{
		if(in_range(etalons[e].ray, etalon_halfwidth, sp.n_rays-etalon_halfwidth-1) && in_range(etalons[e].range, 0, n_etalon_volumes-1))
			etalon_displayer.at(etalons[e].ray + etalon_halfwidth,etalons[e].range) = crayons::red();
	}

	DisplayMathFunction2D(etalon_displayer, title, etalon_displayer);
}

void	EtalonSet::ComputeBasis(ComplexMatrixF32 &eigen_vectors)
{
	size_t	es = etalon_length;
	ComplexMatrixF32	ac_matrix(es, es);
	RealVectorF32	lambda(es);

	eigen_vectors.realloc(es, es);


	for(size_t i = 0; i < es; i++)
	{
		for(size_t j = 0; j < es; j++)
		{
			float	d1(0), d2(0);
			for(size_t e = 0; e < n_etalons; e++)
			{
				complexF32	c1(etalons[e].data.at(0,i));
				complexF32	c2(etalons[e].data.at(0,j));
				ac_matrix.at(i,j) += c1 % c2;
				d1 += cabs2(c1);
				d2 += cabs2(c2);
			}
			if(d1&&d2) ac_matrix.at(i,j) /= sqrt(d1*d2);
		}
	}

//	ac_matrix.Display("Autocorrelation matrix");

	bool	result = eigenvectors_hermit(ac_matrix, eigen_vectors, lambda);
	if(!result)
	{
		Error("Eigen vectors not found");
	}

	for(size_t i = 0; i < es; ++i)
	{
//		if(0) eigen_vectors.col(i).roll_half(true);
//		if(0) ApplyWindowFunction(eigen_vectors.col(i), blackman_nuttall_win());
	}
}

void	EtalonSet::AnalyzeEtalons(SignalProcessor &sp)
{
	ComplexFunctionF32	fft_buffer(etalon_length);
	RealFunctionF32	analyzer(etalon_length);
	RealFunctionF32	analyzer1(etalon_length);

	static	GraphSet	gsa("Average spectra", "MHz", "Magnitude average");
	static	GraphSet	gs1("Single spectra", "MHz", "Magnitude ");

	string	graph_title = GetString("Enter graph title", "Etalons # ");

	size_t	et = 0;
//	sp.SetFrequencyUnits(M_HERZ);
	if(1)do
	{
		et = GetSigned("Etalon NO", et, 0, n_etalons-1);
		if(etalon_width > 1) DisplayMathFunction2D(etalons[et].data, "Current etalon");
		else
		{
			fft_buffer.CopyData(etalons[et].data.row(0));
			ApplyWindowFunction(fft_buffer, cos2_window());
			FFT(fft_buffer, ftForward);
			DisplayMathFunction(fft_buffer, 0, sp.sample_rate.MHz()/etalon_length, "Current etalon");
		}
	} while(YesOrNo("Display another etalon?", true));

	for(size_t e = 0; e < n_etalons; e++)
	{
		fft_buffer.CopyData(etalons[e].data.row(0));
		ApplyWindowFunction(fft_buffer, cos2_window());
		FFT(fft_buffer, ftForward);
		for(size_t s = 0; s < etalon_length; s++)
		{
			analyzer[s] += cabs(fft_buffer[s])/n_etalons;
			if(e == n_etalons/2)
				analyzer1[s] = cabs(fft_buffer[s]);
		}
	}
	gsa.AddGraphUniform(analyzer, 0, sp.sample_rate.MHz()/etalon_length, graph_title);
	gs1.AddGraphUniform(analyzer1, 0, sp.sample_rate.MHz()/etalon_length, graph_title);
	DisplayMathFunction(analyzer, 0, sp.sample_rate.MHz()/etalon_length, "Average etalons spectra", "MHz");
}
bool	EtalonSet::SaveEtalons(const char *filename)
{
	FILE	*ef = fopen(filename, "wb");
	if(!ef) return false;

	fwrite(&n_etalons, sizeof(int), 1, ef);
	fwrite(&etalon_length, sizeof(int), 1, ef);
	fwrite(&etalon_width, sizeof(int), 1, ef);

	for(size_t i = 0; i < n_etalons; i++)
	{
		for(size_t j = 0; j < etalon_width; j++)
		{
			fwrite_numbers(etalons[i].data.row(j), ef, ioComplexF32_LE);
		}
	}
	fclose(ef);
	return true;
}

bool	EtalonSet::LoadEtalons(const char *filename)
{
	FILE	*ef = fopen(filename, "rb");
	if(!ef) return false;

	size_t	n_etalons_to_add, etalon_width_new, etalon_length_new;

	fread(&n_etalons_to_add, sizeof(int), 1, ef);
	fread(&etalon_length_new, sizeof(int), 1, ef);
	fread(&etalon_width_new, sizeof(int), 1, ef);

	if(n_etalons_to_add <= 0 || etalon_length_new != etalon_length || etalon_width_new != etalon_width)
	{
		if(n_etalons)
		{
			fclose(ef);
			return false;
		}
		etalon_length = etalon_length_new;
		etalon_width = etalon_width_new;
	}
	etalons.resize(n_etalons + n_etalons_to_add);
	for(size_t i = 0; i < n_etalons_to_add; i++)
	{
		etalons[n_etalons + i].ray = size_t(-1);
		etalons[n_etalons + i].sample = size_t(-1);
		etalons[n_etalons + i].sub = size_t(-1);
		etalons[n_etalons + i].range = size_t(-1);
		etalons[n_etalons + i].data.realloc(etalon_width, etalon_length);
		for(size_t j = 0; j < etalon_width; j++)
		{
			fread_numbers(etalons[n_etalons + i].data.row(j), ef, ioComplexF32_LE);
		}
	}
	n_etalons += n_etalons_to_add;
	fclose(ef);
	return true;
}

void	EtalonSet::CalculateCorrelationMap(SignalProcessor &sp, RealFunction2D_F32	&correlation_map)
{
	size_t	n_rays = sp.n_rays;
	size_t	n_samples = sp.n_samples;
	size_t	n_frames = sp.n_frames;

	correlation_map.realloc(n_rays, n_etalon_volumes);

	GUIProgressBar	progress;
	progress.start("Analyzing etalons", n_etalon_volumes);

	for(size_t i = 0; i < n_etalon_volumes; i++)
	{
		size_t	s0 = range(float(i)*etalon_volume_step - etalon_length/2, 0, n_samples-etalon_length);
		for(size_t j = 0; j < n_rays; j++)
		{
			size_t	r0 = range(j, 0, n_rays-1-etalon_width);
			correlation_map.at(j,i) = 0;

			for(size_t e = 0; e < n_etalons; e++)
			{
				for(size_t sub = 0; sub < n_frames; sub++)
				{
//					sp.SetCurrentFrame(sub);
					complexF32	correlator(0);
					float	sigma_1(0), sigma_2(0);
					ComplexFunction2D_F32	&current_correlator(etalons[e].data);

					for(size_t k = 0; k < etalon_width; k++)
					{
						size_t	r = r0 + k;
//						sp.SetCurrentRay(r);

						ComplexFunctionF32	CurrentRay;
						sp.focused_data.GetRow(CurrentRay, {sub, r, slice_mask(0)});

						ComplexFunctionF32::iterator etalon_v = current_correlator.row(k).begin();
						ComplexFunctionF32::iterator local_v = CurrentRay.begin() + s0;

						for(size_t l = 0; l < etalon_length; l++)
						{
							correlator += (*local_v) % (*etalon_v);
							sigma_1 += cabs2(*local_v);
							sigma_2 += cabs2(*etalon_v);
							++local_v;
							++etalon_v;
						}
					}

					if(sigma_1 && sigma_2) correlator /= sqrt(sigma_1*sigma_2);
					else correlator = 0;
					correlation_map.at(j,i) += cabs(correlator)/n_etalons;
//					range_brightness.at(j,i) += sqrt(sigma_1);
				}
			}
//			range_brightness.at(j,i) /= n_frames;
			correlation_map.at(j,i) /= n_frames;
		}
		++progress;
	}
//	EndProgress();
}




void	EtalonSet::OptimizeEtalons(SignalProcessor &sp, RealFunction2D_F32	&correlation_map)
{
	size_t	n_rays = sp.n_rays;
//	size_t	n_samples = sp.n_samples;
	size_t	n_frames = sp.n_frames;

//	сбор информации о коэффициентах коррелЯции по эталонам

	RealFunctionF32	displayer(n_etalons);
	for(size_t e = 0; e < n_etalons; e++)
	{
	// анализ эталонов, набранных по исходному изображению
		if(in_range(etalons[e].ray, 0, n_rays-1) && in_range(etalons[e].range, 0, n_etalon_volumes-1))
			etalons[e].re = displayer[e] = correlation_map.at(etalons[e].ray,etalons[e].range);
		// анализ эталонов, загруженных из внешнего источника
		else
		{
			etalons[e].re = displayer[e] = 0;
			for(size_t e1 = 0; e1 < n_etalons; e1++)
			{
				float	s1(0), s2(0);
				complexF32	correlator(0);
				for(size_t i = 0; i < etalon_width; i++)
				{
					for(size_t j = 0; j < etalon_length; j++)
					{
						s1 += cabs2(etalons[e].data.at(i,j));
						s2 += cabs2(etalons[e1].data.at(i,j));
						correlator += etalons[e1].data.at(i,j) % etalons[e].data.at(i,j);
					}
				}
				if(s1 && s2) correlator /= sqrt(s1*s2);
				etalons[e].re = displayer[e] += cabs(correlator)/n_etalons;
			}
		}
	}

//	сортировка по степени коррелированности (с убыванием, чтобы потом легче было удалить слабо коррелированные эталоны)
	reorder_descent(etalons);

//	сортировка по степени коррелированности (с возрастанием, чтобы распределение более естественно выглЯдело на экране)
	reorder_ascent(displayer);

//	просмотр и удаление слабокоррелированных элементов
	DisplayMathFunction(displayer, 0, 1, "etalons correlation");
	size_t	n = GetUnsigned("N etalons to remove", 0, 0, n_etalons-1);
	n_etalons -= n;
	etalons.resize(n_etalons);


//	показ "урезанных" эталонов
	ShowEtalons("Etalons mapping (reduced)", sp);
	float	correlation_treshold = etalons[0].re;
	size_t	count_added;


//	поиск точек, сильно коррелированных с уже имеющимисЯ эталонами
//	и добавление их в список длЯ анализа
	do
	{
		correlation_treshold = GetFloating("Correlation treshold", correlation_treshold, etalons[n_etalons-1].re, etalons[0].re);
		count_added = 0;
		for(size_t i = 1; i < n_rays-1; i++)
		{
			for(size_t j = 1; j < n_etalon_volumes-1; j++)
			{
				float	c = correlation_map.at(i,j);
				if(c >= correlation_treshold)
				{
					if(c > correlation_map.at(i-1,j) && c > correlation_map.at(i+1,j) &&
					   c > correlation_map.at(i,j-1) && c > correlation_map.at(i,j+1) &&
					   c > correlation_map.at(i-1,j-1) && c > correlation_map.at(i+1,j+1) &&
					   c > correlation_map.at(i+1,j-1) && c > correlation_map.at(i-1,j+1))
					{

						if(!FindEtalon(i, j)) count_added++;
					}
				}
			}
		}
		ShowSigned("Etalons to be added", count_added);
	} while(YesOrNo("Change treshold", true));

	bool	use_all_subapertures = YesOrNo("Take etalons from all frames?", false);
	if(use_all_subapertures)
		etalons.resize(n_etalons + count_added * n_frames);
	else etalons.resize(n_etalons + count_added);

	for(size_t i = 1; i < n_rays-1; i++)
	{
		for(size_t j = 1; j < n_etalon_volumes-1; j++)
		{
			float	c = correlation_map.at(i,j);
			if(c >= correlation_treshold)
			{
				if(c > correlation_map.at(i-1,j) && c > correlation_map.at(i+1,j) &&
				   c > correlation_map.at(i,j-1) && c > correlation_map.at(i,j+1) &&
				   c > correlation_map.at(i-1,j-1) && c > correlation_map.at(i+1,j+1) &&
				   c > correlation_map.at(i+1,j-1) && c > correlation_map.at(i-1,j+1))
				{

					if(!FindEtalon(i, j))
					{
						if(use_all_subapertures)for(size_t sub = 0; sub < n_frames; sub++)
							PutEtalon(n_etalons++, i, j, sub, sp);
						else PutEtalon(n_etalons++, i, j, 0, sp);
					}
				}
			}
		}
	}
	ShowEtalons("Etalons mapping (added)", sp);
}



XRAD_END
