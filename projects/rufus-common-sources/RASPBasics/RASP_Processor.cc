XRAD_BEGIN


//-----------------------------------------------------------------------------
//
//	эта функция преобразует значения порогов в настройках таким же образом,
//	каким преобразованы сами отсчеты. использовать функцию ImportOriginalSample
//	для этой цели напрямую неудобно, поскольку она сделана под преобразование
//	величин конкретных типов. пороговые же значения у нас идут в плавающей
//	запятой

template<class T1, class T2>
void	RASP_Processor<T1,T2>::ImportTresholdValue(float &imported_value, const float &original_value)
	{
	processing_sample	normalizer_factor;
	ImportOriginalSample(normalizer_factor, input_sample(1));

	processing_sample	ps;
	proper_range(ps, original_value * normalizer_factor);

	imported_value = ps;
	}

template<class T1, class T2>
void	RASP_Processor<T1,T2>::SetActivePreset(const TextureProcessorSettings *preset)
	{
	texture_processor.InitFilters(*preset);
	//	три последующие величины настроек идут в масштабе входных данных.
	//	они импортируются таким же образом
	ImportTresholdValue(texture_processor.amplification_treshold, preset->amplification_treshold);
	ImportTresholdValue(texture_processor.peak_enhance_treshold, preset->peak_enhance_treshold);
	ImportTresholdValue(texture_processor.border_enhance_treshold, preset->border_enhance_treshold);
	}



template<class T1, class T2>
void	RASP_Processor<T1,T2>::ProcessRaster(input_sample *out_data, const input_sample *in_data,
			int n_rows, int n_columns,
			int out_pitch, int in_pitch,
			int intercept, float slope)
	{
	//	оболочки, используемые для импорта и экспорта внешних данных,
	//	следует перестравать только в двух случаях
	//	1. если оболочки еще не проинициализированы;
	//	2. если указатели на входные данные действительно изменились.
	static	int	sample_alignment = row_alignment_bytes/sizeof(processing_sample);
	if(row_alignment_bytes%sizeof(processing_sample))
		{
        throw logic_error("Invalid sample size/data alignment combination");
		}

	bool	remap_in_data_shell = !in_data_shell.ready();
	bool	remap_out_data_shell = !out_data_shell.ready();

	if(!remap_in_data_shell)
		{
		remap_in_data_shell =
			(in_data != &in_data_shell.at(0,0)	||
			n_rows != in_data_shell.vsize()	||
			n_columns != in_data_shell.hsize()||
			in_pitch != in_data_shell.vstep());
		}
	if(!remap_out_data_shell)
		{
		remap_out_data_shell =
			(out_data != &out_data_shell.at(0,0)	||
			n_rows != out_data_shell.vsize()	||
			n_columns != out_data_shell.hsize()||
			out_pitch != out_data_shell.vstep());
		}

	if(remap_in_data_shell)
			{
			//	!!! здесь сейчас будет явное преобразование типов конст-неконст.
			//	нехорошо
			in_data_shell.UseData(const_cast<input_sample*>(in_data),
				n_rows,n_columns, in_pitch, 1);
			}
	if(remap_out_data_shell)
			{
			out_data_shell.UseData(out_data,n_rows,n_columns,
				out_pitch, 1);
			}


	int	n_rows_aligned = n_rows;
	int	n_columns_aligned = n_columns;
	
//	выравнивание по строкам восстанавливаем, т.к., возможно,
//	из-за него была ошибка: в некоторых ситуациях функция RASP_Finish()
//	повисала

//	как оказалось, видимо, не из-за него, но из-за импорта!!!

//	int	row_alignment = 8;
	
//	if(n_rows % row_alignment)
//		{
//		n_rows_aligned += (sample_alignment - (n_rows % row_alignment));
//		}
//	выравнивание по отсчету существенно необходимо, т.к.
//	алгоритмы фильтрации начинаются с выровненного отсчета на каждой строке.
//	для ак. строк это необязательно (специально написаны невыровненные версии)
//	p.s. возможно, это так только кажется. см. комментарий выше (row_alignment)


	if(n_columns % sample_alignment)
		{
		n_columns_aligned += (sample_alignment - (n_columns % sample_alignment));
		}

	process_buffer.realloc(n_rows_aligned, n_columns_aligned);
		// не следует ли здесь ввести также pitch?

	//	в целочисленных вариантах алгоритма
	//  входные данные импортируются со сдвигом влево

 	ImportOriginalImage(process_buffer,in_data_shell, intercept, slope);

	texture_processor.InitBuffers(n_rows_aligned, n_columns_aligned);
	texture_processor.ProcessTextures(process_buffer);

	//  экспортируем со сдвигом вправо
	ExportProcessedImage(out_data_shell,process_buffer, intercept, slope);
	}




XRAD_END

