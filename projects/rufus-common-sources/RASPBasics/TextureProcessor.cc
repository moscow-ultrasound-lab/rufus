//#included throuth "TextureProcessor.h
//RaspTableFunction.h
// RASPFunctors.h
// RASPFixedPoint.h

XRAD_BEGIN



//-----------------------------------------------------------------------------
//
//	TextureProcessor
//
//-----------------------------------------------------------------------------

template<class TPA>
TextureProcessor<TPA> :: TextureProcessor(sample_t	max_sample) :
		gamma_f(12),
		mask_f(10),
		mask_multiplier(mask_f.table_bits, mask_f.normalizer()),
		max_allowed_sample_value(max_sample)
	{
	//InitBuffers(0,0);
	n_rays = n_samples = 0;
		// эта инициализация оказалась важна, так как иначе эклипса
		// при повторном вызове создавала объект в той же памяти, где
		// он был проинициализирован ранее. в результате первый вызов
		// InitBuffers() проходит без последствий, будто все уже и так 
		// проинициализировано
	}


template<class TPA>
void	TextureProcessor<TPA> :: InitFilters(const TextureProcessorSettings &tp)
	{
	CopyTextureProcessorSettings(*this, tp);
	mask_f.InitPowerFunction(fine_struct_transparency);
	if (apply_gamma_correction)
		{
		gamma_f.InitPowerFunction(gamma);
		}
	}


template<class TPA>
void	TextureProcessor<TPA> :: InitBuffers(int nr, int ns)
	{
	if(n_rays == nr && n_samples == ns) return;
	n_rays = nr;
	n_samples = ns;
	original.realloc(n_rays, n_samples);
	rough.realloc(n_rays, n_samples);
	rough_struct_mask.realloc(n_rays, n_samples);
	fine_struct.realloc(n_rays, n_samples);
	fine_struct_mask.realloc(n_rays, n_samples);
	}


//---------------------------------------------------------------------------------------

template<class TPA>
void	TextureProcessor<TPA> :: ProcessTextures(array2D_t &im)
	{	
	rough.CopyData(im);
	UniversalBlur2D(rough, lateral_scale, axial_scale);

	// данное действие означает fine_struct = im-rough;
	AAA_2D_Op(fine_struct, im, rough, minus_ternary<sample_t,sample_t,sample_t>());

	ComputeFineStructMask();

	if(peak_border_enhance)
		{
		ComputeRoughStructMask();
//		ProcessAntigradient();
		}
	else
		{
        rough_struct_mask.fill(mask_f.normalizer());
		}
	if(apply_gamma_correction)RoughStructGamma();

	//#######
//	fine_struct.fill(255*32);
//	rough.fill(0);

	CombineResult(im);
	RangeCorrection(im);
	}


template<class TPA>
void	TextureProcessor<TPA> :: ComputeFineStructMask()
	{
	//186k exclusive
	fine_struct_mask.CopyData(fine_struct);

	//#######
	UniversalBlur2D(fine_struct_mask, fine_mask_const*axial_scale, fine_mask_const*lateral_scale);

	ApplyFunction(fine_struct_mask, absolute_value_functor());

	sample_t max_fine_struct_value = MaxValue(fine_struct_mask);
	if(!max_fine_struct_value) return;


	if(fine_struct_transparency <= 0) fine_struct_mask.fill(mask_f.normalizer());
	else if(fine_struct_transparency >= 1) return;
	else
		{
		double	factor = double(mask_f.normalizer())/max_fine_struct_value;
		fine_struct_mask *= factor;
		// следующая функция занимает 155к циклов, можно повозиться над ее оптимизацией
		ApplyFunction(fine_struct_mask, mask_f);
		}
	}


template<class TPA>
void	TextureProcessor<TPA> :: RoughStructGamma()
	{
	double max_rough_value = MaxValue(rough); // double, not sample_t
	if(!max_rough_value) return;

	double	pre_normalizer = (gamma_f.normalizer()/max_rough_value);
	double	post_normalizer;

	rough *= pre_normalizer;

//	gamma_f.InitPowerFunction(gamma);
	ApplyFunction(rough, gamma_f);

	if(gamma > 1)
		{
		// величина max_rough_value*(1.-gamma_midpoint) остается постоянной при gamma>1
		post_normalizer = double(max_rough_value*pow(1.-gamma_midpoint, 1.-gamma)/gamma_f.normalizer());
		}
	else
		{
		// величина max_rough_value*gamma_midpoint остается постоянной при gamma<1
		post_normalizer = double(max_rough_value*pow(double(gamma_midpoint), 1.-gamma)/gamma_f.normalizer());
		}
	rough *= post_normalizer;
	}


template<class TPA>
void	TextureProcessor<TPA> :: CombineResult(array2D_t &im)
	{
//	можно умножить крупномасштабную часть на маску,
//	но в этом случае детали мелкой структуры на фоне подавленных участков
//	будут проступать слишком заметно.
//	поэтому умножаем на маску уже сложенные части
//	(последняя строчка этой функции)

	AA_2D_OpEq(fine_struct, fine_struct_mask, mask_multiplier);
		// fine_struct *= fine_struct_mask с поправкой на нормировку табличной функции
	AAA_2D_Op(im, fine_struct, rough, plus_ternary<sample_t,sample_t,sample_t>());
		// im = fine_struct + rough;
	AA_2D_OpEq(im, rough_struct_mask, mask_multiplier);
		// im *= rough_struct_mask
	}


template<class sample_t>
struct	truncate_functor
	{
    typedef	RASPFunctors::shift_multiplier<sample_t> multiplier_t;

	const	sample_t	tr0, tr1, tr2;
	const	sample_t	factor;
	const	multiplier_t	&multiplier;

	truncate_functor(sample_t t0, sample_t t1, sample_t t2, sample_t normalizer, const multiplier_t& m):
		tr0(t0),
		tr1(t1),
		tr2(t2),
		factor(sample_t(normalizer*(double(t1-t0)/double(t2-t0)))),
		multiplier(m)
		{
		}

	sample_t	&operator()(sample_t &result, const sample_t& v) const
		{
		if(v<tr0) return result = v;
		else
			{
			sample_t buffer = v-tr0;
			return result = tr0 + multiplier(buffer,factor);
			}
		}
	};

template<class TPA>
void	TextureProcessor<TPA> :: RangeCorrection(array2D_t &im)
	{
//	static	int	n;
//	int	N = 40;
//	if((++n/N)%2) return;

	sample_t max_value = MaxValue(im);
	if(!max_value) return;
	if(max_value <= max_allowed_sample_value) return;


	sample_t	unchanged_treshold = max_allowed_sample_value*0.5;
	ApplyFunction(im, truncate_functor<sample_t>(unchanged_treshold, max_allowed_sample_value, max_value, mask_f.normalizer(), mask_multiplier));
	}


//

template<class sample_t>
inline	void	fast_gradient(sample_t &g, const sample_t &v00, const sample_t &v01, const sample_t &v10)
	{
	g = fast_tp_norma(v00 - v01, v00 - v10);
	}


//-----------------------------------------------------------------------------
//template<class sample_t>
//inline bool	has_peak(	const sample_t& g00,
//						const sample_t& g01,	const sample_t& g0_1,
//						const sample_t& g10,	const sample_t& g_10,
//						const sample_t& g11,	const sample_t& g_1_1,
//						const sample_t& g1_1,	const sample_t& g_11)
//		{
//		return	((g00 > g01  && g00 > g0_1) ||
//				(g00 > g10   && g00 > g_10) ||
//				(g00 > g11 && g00 > g_1_1) ||
//				(g00 > g1_1 && g00 > g_11));
//		}

//-----------------------------------------------------------------------------

template<class sample_t>
inline int	has_bright_edge(	const sample_t& g00,
						const sample_t& g01,	const sample_t& g0_1,
						const sample_t& g10,	const sample_t& g_10,
						const sample_t& g11,	const sample_t& g_1_1,
						const sample_t& g1_1,	const sample_t& g_11)
		{
		return	(g00>g01) + (g00>g0_1) +
				(g00>g10) + (g00>g_10) +
				(g00>g11) + (g00>g_1_1) +
				(g00>g1_1) + (g00>g_11);
		}

//-----------------------------------------------------------------------------

template<class sample_t>
inline int	has_dark_edge(	const sample_t& g00,
						const sample_t& g01,	const sample_t& g0_1,
						const sample_t& g10,	const sample_t& g_10,
						const sample_t& g11,	const sample_t& g_1_1,
						const sample_t& g1_1,	const sample_t& g_11)
		{
		return	(g00<g01) + (g00<g0_1) +
				(g00<g10) + (g00<g_10) +
				(g00<g11) + (g00<g_1_1) +
				(g00<g1_1) + (g00<g_11);
		}


#if defined __TI_COMPILER_VERSION__


inline int	__has_bright_edge(	const int16_t&  restrict g00,
						const int16_t&  restrict g01,	const int16_t&  restrict g0_1,
						const int16_t&  restrict g10,	const int16_t&  restrict g_10,
						const int16_t&  restrict g11,	const int16_t&  restrict g_1_1,
						const int16_t&  restrict g1_1,	const int16_t&  restrict g_11)
		{
		int	gg = _pack2(g00, g00);
		buffer32	b;
		b.i = _bitc4(_spacku4(
			_pack2(
				_cmpgt2(gg, _pack2(g01,g0_1)),
				_cmpgt2(gg, _pack2(g10,g_10))
				),
			_pack2(
				_cmpgt2(gg, _pack2(g11,g_1_1)),
				_cmpgt2(gg, _pack2(g1_1,g_11))
				)));
		
//		return b.c[0]+b.c[1]+b.c[2]+b.c[3];
		return b.i;
		}



#endif //__TI_COMPILER_VERSION__






template<class TPA>
void	TextureProcessor<TPA> :: ComputeRoughStructMask()
	{
	sample_t	*rough_p = &rough[0][0];
	sample_t	*mask_p = &rough_struct_mask[0][0];

	//	дальность поиска границ преобразуем к int, так как
	//	поиск производится с дискретностью в отсчет. сам
	//	параметр в настройках, однако, должен быть непрерывным, так как
	//	он используется еще и для нормировки порогов (см. следующие 3 формулы)

	int	ax = border_search_distance;
	int	lat = border_search_distance*search_aspect_ratio;


	//	последующие величины идут в масштабе входных данных. поэтому они
	//	не должны здесь подвергаться никаким изменениям. все, что нужно было,
	//	сделано в одно время с импортом входных данных при входе
	//	в процедуру обработки RASP_Processor<T1,T2>::ProcessRaster
	// 	только тип преобразуем к sample_t, чтобы не сравнивать разнородные
	//	величины

//	sample_t	amp_tr = amplification_treshold;
//	sample_t	peak_treshold = int(double(peak_enhance_treshold*border_search_distance)/sqrt(axial_scale+lateral_scale));
	sample_t	border_treshold = int(double(border_enhance_treshold*border_search_distance)/sqrt(axial_scale+lateral_scale));

	//	оба множителя ниже приведены к масштабу внутренних данных.
	//	иначе пришлось бы выполнять в цикле преобразования double-->int,
	//	что катастрофически снижает производительность.
	//  умножать эти множители на отсчеты данных нужно не через *, а через
	//	multiply().


	sample_t	bef = (mask_f.normalizer()*(1. + border_enhance_factor*peak_border_factor/2));
	sample_t	def = (mask_f.normalizer()*(1. - border_shadow_factor*peak_border_factor/3));
	//	в качестве единицы используется величина mask_f.normalizer.
	//	это потому, что в качестве оператора умножения на эту маску используется
	//	функтор mask_multiplier

//	rough_struct_mask.fill(sample_t(0));
	rough_struct_mask.fill(mask_f.normalizer());

	for(int i = 2*lat; i < n_rays-3*lat; ++i)
		{
		int	i0 = i*n_samples;
		int	di = lat*n_samples;
//		int	i1 = (i+lat)*n_samples;
//		int	i_1 = (i-lat)*n_samples;

		int	j_first = 2*ax;
		int	j_last = n_samples-3*ax;

		sample_t	*mp =		mask_p + i0 + j_first;
		sample_t	*rp = 		rough_p + i0 + j_first;

		sample_t	*rp01 =		rp + ax;
		sample_t	*rp0_1 =	rp - ax;

//		sample_t	*rp10 = 	rough_p + i1 + j_first;
//		sample_t	*rp_10 =	rough_p + i_1 + j_first;
		sample_t	*rp10 = 	rp + di;
		sample_t	*rp_10 =	rp - di;

		sample_t	*rp11 =		rp10 + ax;
		sample_t	*rp1_1 =	rp10 - ax;

		sample_t	*rp_11 =	rp_10 + ax;
		sample_t	*rp_1_1 =	rp_10 - ax;

		for(int j = j_first; j < j_last; ++j, ++rp, ++rp01, ++rp10, ++rp0_1, ++rp_10, ++rp11, ++rp1_1, ++rp_11, ++rp_1_1)
			{
//			if(*rp > amp_tr)
				{
				int	bright_edge = has_bright_edge(sample_t(rough_p[i0+j] - border_treshold),
						*rp01,  *rp0_1, *rp10,  *rp_10,
						*rp11, *rp_1_1, *rp1_1,  *rp_11);

				int	dark_edge = has_dark_edge(sample_t(rough_p[i0+j] + border_treshold),
						*rp01,  *rp0_1, *rp10,  *rp_10,
						*rp11, *rp_1_1, *rp1_1,  *rp_11);

				if(dark_edge>bright_edge) *mp = def;//rough_p[i0+j]* bef;
				else if(bright_edge) *mp = bef;//rough_p[i0+j]* bef;
				}
			++mp;
			}
		}
	UniversalBlur2D(rough_struct_mask, border_shadow_radius*lateral_scale, border_shadow_radius*axial_scale/search_aspect_ratio);
	}




XRAD_END

