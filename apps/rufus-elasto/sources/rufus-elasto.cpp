#include "pre.h"

#include <XRADSystem/Sources/IniFile/XRADIniFile.h>
#include <XRADSystem/system.h>


 #include "rufus-elasto.h"

#include <DopplerBasics/Elasto/CFDataPhaseAnalyzerElasto.h>
#include <DopplerBasics/CFM/CFDataPhaseAnalyzerCFM.h>
//#include <DopplerBasics/CFM/CFDataPhaseAnalyzerTwinkling.h>
#include <DopplerBasics/CFMModes.h>

// warning C4702: unreachable code
// для порядка использую блоки try/catch в таких местах,
// где исключений при нынешнем устройстве быть не может.
// убирать их оттуда не следует, т.к. ситуация может измениться.
// здесь (только!) это предупреждение отключаю.
#pragma warning(disable:4702)


namespace xrad
{
bool ELASTO_DLL_EI	unsweep_on_read = false;
unique_ptr<TimeProfiler>	tp_full_frame_proc;
unique_ptr<TimeProfiler>	tp_arrays_alloc;
}



using namespace xrad;

namespace ElastoGrafica
{
CFDataPhaseAnalyzer *phase_elasto_analyzer = NULL;
RealFunction2D_F32	*full_elastogram_buffer = NULL;
RealFunction2D_F32	*full_mask_buffer = NULL;

RealFunctionF32	*blur_values(NULL), *frame_averaging_values(NULL);
// буферные переменные для взаимодействия с интерфейсом пользователя
// при каждом цикле обработки они заново передаются объекту-обработчику
double	current_elastogram_blur, current_elastogram_agility;
double	current_mask_threshold(0.5), current_velocity_threshold(0.5);


xrad::cfm_mode	mode = cfm_mode::elastography;


size_t	roi_sample_start;
size_t	roi_sample_end;




void    LoadSettingsIni(const char *settings_filename)
{
	blur_values = new RealFunctionF32(n_elasto_blur_values);
	frame_averaging_values = new RealFunctionF32(n_elasto_frame_averaging_values);
//    shared_cfile    f;
	text_file_reader    f;
	size_t filter_order(2);
	try
	{
		f.open(settings_filename);
	}
	catch(file_container_error &) {}
	bool    file_read_failed = false;
	int    current_mode;
	if(f.is_open())
	{
		int    n = 0;

		f.scanf_("Blur values = %d\n", &n);
		if(n!=n_elasto_blur_values) file_read_failed = true;

		for(size_t i = 0; i < n_elasto_blur_values; ++i)
		{
			if(!f.scanf_("%g, ", &(blur_values->at(i))))
			{
				file_read_failed = false;
			}
		}

		n=0;
		f.scanf_("Frame averaging values = %d\n", &n);
		if(n!=n_elasto_frame_averaging_values) file_read_failed = true;

		for(size_t i = 0; i < n_elasto_frame_averaging_values; ++i)
		{
			if(!f.scanf_("%g, ", &(frame_averaging_values->at(i))))
			{
				file_read_failed = false;
			}
		}


		f.scanf_("Mode = %d\n", &current_mode);
		if ((current_mode < 0)||(current_mode > 4)) file_read_failed = true;
		mode = cfm_mode(current_mode);

		f.scanf_("filter_order = %d\n", &filter_order);
		if ((filter_order < 1) ||(filter_order > 3)) file_read_failed = true;
	}
	else
	{
		file_read_failed = true;
	}

	if(file_read_failed)
	{
		double    default_blur[] ={1, 2, 3, 4, 5};
		double    default_frame_averaging[] ={1, 0.5, 0.25, 0.1, 0.05};

		for(size_t i = 0; i < n_elasto_blur_values; ++i) blur_values->at(i) = default_blur[i];
		for(size_t i = 0; i < n_elasto_frame_averaging_values; ++i) frame_averaging_values->at(i) = default_frame_averaging[i];
		mode = cfm_mode(0);
	}
}



void	PrintErrorInfo(exception &ex)
{
	/*
	FILE	*file = xrad::fopen("c:\\temp\\elasto_error.txt", "ab+");
	fprintf(file, "%s", ex.what());
	fclose(file);
	*/
}

elasto_status	ELASTO_DLL_EI ChangeMode(xrad::cfm_mode in_mode)

{
	try
	{
		mode = in_mode;
		DestroyObject(phase_elasto_analyzer);

		switch(in_mode)
		{
			default:
				throw invalid_argument("ChangeMode, Unknown mode");

			case xrad::cfm_mode::elastography:
				phase_elasto_analyzer = new CFDataPhaseAnalyzerElasto;
				break;
			case xrad::cfm_mode::cfm:
				phase_elasto_analyzer = new CFDataPhaseAnalyzerCFM;
				break;
		}
		return ResetElastogram();
	}
	catch(exception &ex)
	{
		PrintErrorInfo(ex);
		return elasto_status::initialization_error;
	}
	catch(...)
	{
		DestroyObject(phase_elasto_analyzer);
		return elasto_status::initialization_error;
	}
}

elasto_status	ELASTO_DLL_EI InitElastographyLib(const char *settings_filename)
{
	try
	{
		tp_full_frame_proc = make_unique<TimeProfiler>();
		tp_arrays_alloc = make_unique<TimeProfiler>();


		roi_sample_start = roi_sample_end = 0;
		DestroyObject(phase_elasto_analyzer);
		DestroyObject(full_elastogram_buffer);
		DestroyObject(full_mask_buffer);
		DestroyObject(blur_values);
		DestroyObject(frame_averaging_values);

		LoadSettingsIni(settings_filename);
/*
		IniFileReader	f;
		f.open(settings_filename);
		f.set_section("Elasto_settings");
		current_elastogram_blur = f.read_double("blur");;
		current_elastogram_agility = f.read_double("agility");
		f.close();
*/
		//ChangeMode(mode);

		full_elastogram_buffer = new RealFunction2D_F32;
		full_mask_buffer = new RealFunction2D_F32;

		// устанавливаем дефолтные значения переменных такими, как их делает конструктор обработчика
		current_elastogram_blur = phase_elasto_analyzer->result_axial_blur;
		current_elastogram_agility = phase_elasto_analyzer->frame_agility_factor;
		return elasto_status::ok;
	}
	catch(exception &ex)
	{
		PrintErrorInfo(ex);
		return elasto_status::initialization_error;
	}
	catch(...)
	{
		DestroyObject(phase_elasto_analyzer);
		DestroyObject(full_elastogram_buffer);
		DestroyObject(full_mask_buffer);
		DestroyObject(blur_values);
		DestroyObject(frame_averaging_values);
		return elasto_status::initialization_error;
	}
}

elasto_status ELASTO_DLL_EI	FinishElastographyLib()
{
	if(!phase_elasto_analyzer) return elasto_status::not_initialized;
	try
	{
		tp_full_frame_proc.release();
		tp_arrays_alloc.release();


		DestroyObject(phase_elasto_analyzer);
		DestroyObject(full_elastogram_buffer);
		DestroyObject(full_mask_buffer);
		DestroyObject(blur_values);
		DestroyObject(frame_averaging_values);
		return elasto_status::ok;
	}
	catch(exception &ex)
	{
		PrintErrorInfo(ex);
		return elasto_status::finalization_error;
	}
	catch(...)
	{
		return elasto_status::finalization_error;
	}
}

elasto_status ELASTO_DLL_EI	ResetElastogram()
{
	if(!phase_elasto_analyzer) return elasto_status::not_initialized;
	try
	{
		phase_elasto_analyzer->ResetFrameAveraging();
		return elasto_status::ok;
	}
	catch(exception &ex)
	{
		PrintErrorInfo(ex);
		return elasto_status::operation_error;
	}
	catch(...)
	{
		return elasto_status::operation_error;
	}
}

elasto_status ELASTO_DLL_EI	SetElastogramAgilityDirect(double factor)
{
	if(!phase_elasto_analyzer) return elasto_status::not_initialized;
	try
	{
		current_elastogram_agility = factor;
		return elasto_status::ok;
	}
	catch(exception &ex)
	{
		PrintErrorInfo(ex);
	}
	catch(...)
	{
		return elasto_status::operation_error;
	}
}

elasto_status ELASTO_DLL_EI	SetElastogramAgility(frame_averaging_t n)
{
	if(!frame_averaging_values) return elasto_status::not_initialized;
	else return SetElastogramAgilityDirect(frame_averaging_values->at(n));
}

elasto_status ELASTO_DLL_EI	SetElastogramBlur(axial_blur_t n)
{
	if(!blur_values) return elasto_status::not_initialized;
	else return SetElastogramBlurDirect(blur_values->at(n));
}

elasto_status ELASTO_DLL_EI	SetElastogramBlurDirect(double radius)
{
	if(!phase_elasto_analyzer) return elasto_status::not_initialized;
	try
	{
		current_elastogram_blur = radius;
		return elasto_status::ok;
	}
	catch(exception &ex)
	{
		PrintErrorInfo(ex);
	}
	catch(...)
	{
		return elasto_status::operation_error;
	}
}

elasto_status ELASTO_DLL_EI	SetFrameSizes(size_t n_shots, size_t n_beams, size_t n_samples, size_t beams_in_sweep, size_t ray_header_size_in_bytes, size_t in_roi_sample_start, size_t in_roi_sample_end)
{
	if(!phase_elasto_analyzer) return elasto_status::not_initialized;
	try
	{
		phase_elasto_analyzer->PrepareBuffers({n_beams, n_shots, n_samples}, beams_in_sweep, ray_header_size_in_bytes);

		roi_sample_start = in_roi_sample_start;
		roi_sample_end = in_roi_sample_end;
		if(roi_sample_end <= roi_sample_start || roi_sample_start < 0 || roi_sample_end > n_samples)
		{
			throw invalid_argument("invalid ROI");
		}

		full_elastogram_buffer->realloc(n_beams, n_samples, 0);
		full_mask_buffer->realloc(n_beams, n_samples, 0);


		return elasto_status::ok;
	}
	catch(exception &ex)
	{
		PrintErrorInfo(ex);
		return elasto_status::operation_error;
	}
	catch(...)
	{
		return elasto_status::operation_error;
	}
}


void	UpdateElastoSettings()
{
	static	double last_time = 0;
	double	current_time = clock() / CLOCKS_PER_SEC;
	const double update_time = 1;//sec
	if (current_time - last_time > update_time)
	{
		IniFileReader f;
		wstring	ini_file_name = L"C:/temp/elasto_settings.ini";
		f.open(ini_file_name);
		f.set_section("Elasto_settings");
		current_elastogram_blur = f.read_double("blur");;
		current_elastogram_agility = f.read_double("agility");
		f.close();
		last_time = clock() / CLOCKS_PER_SEC;
	}
}

void	UpdateSettings()
{
	static	double last_time = 0;
	double	current_time = clock() / CLOCKS_PER_SEC;
	const double update_time = 1;//sec
	if (current_time - last_time > update_time)
	{
		//double std_threshold(0), correlation_threshold(0), re_im_correlation_threshold(0), amplitude_threshold(0);
		
		
		IniFileReader f;
		//wstring	ini_file_name = WGetApplicationDirectory() + L"//artifact_settings.ini";
		wstring	ini_file_name = L"Q:/projects/ElastoGrafica/settings/artifact_settings.ini";
		

		f.open(ini_file_name);

		f.set_section("Probe&Examination");

		string	probe_and_examination = f.read_string("type");
		probe_and_examination += ssprintf("_%zu_pulses", phase_elasto_analyzer->n_cfm_shots()); 

		f.set_section(probe_and_examination);



// 		auto twinkling_analyzer = dynamic_cast<CFDataPhaseAnalyzerCascillation*>(phase_elasto_analyzer);
// 		if(twinkling_analyzer)
// 		{
// 			twinkling_analyzer->re_im_cor_threshold = f.read_double("re_im_correlation");
// 			twinkling_analyzer->std_threshold = f.read_double("std");
// 			twinkling_analyzer->cor_threshold = f.read_double("correlation");
// 			twinkling_analyzer->amplitude_threshold = f.read_double("amplitude");
// 		}
// 		
		f.close();

		last_time = clock() / CLOCKS_PER_SEC;
	}
}

elasto_status ELASTO_DLL_EI	BuildElastogram(const __int32 *in_cfm_data, float *out_elastogram, float *offset_value, float *mask)
{

//	UpdateSettings();

	if(!phase_elasto_analyzer) return elasto_status::not_initialized;

	try
	{
		const cfm_container_t::value_type::part_type *type_control_ptr = in_cfm_data;
		// убеждаемся, что наш внутренний комплексный массив соответствует
		// входному массиву по типу компоненты комплексного числа
		const cfm_container_t::value_type *in_complex_ptr = reinterpret_cast<const cfm_container_t::value_type*>(type_control_ptr);
		// из указателя на массив действительных чисел делаем указатель на комплексный массив,
		// который идет далее в обработку.
		tp_full_frame_proc->Start();
		tp_arrays_alloc->Start();
		RealFunction2D_F32	elastogram;
// 		elastogram.UseData(out_elastogram, phase_elasto_analyzer->n_cfm_beams, phase_elasto_analyzer->n_cfm_samples);
		elastogram.UseData(out_elastogram, phase_elasto_analyzer->n_cfm_beams(), roi_sample_end-roi_sample_start);
		RealFunction2D_F32	elasto_mask;
		elasto_mask.UseData(mask, phase_elasto_analyzer->n_cfm_beams(), roi_sample_end - roi_sample_start);
		phase_elasto_analyzer->ImportRawData(in_complex_ptr, !unsweep_on_read);
		tp_arrays_alloc->Stop();
		// перед началом обработки подставляем динамические параметры, введенные с интерфейса
		UpdateElastoSettings();
		phase_elasto_analyzer->result_axial_blur = current_elastogram_blur;
		phase_elasto_analyzer->frame_agility_factor = current_elastogram_agility;

		phase_elasto_analyzer->AnalyzeFrame(*full_elastogram_buffer, *full_mask_buffer);
		*offset_value = phase_elasto_analyzer->average_frame_offset();

		for(size_t i = 0; i < phase_elasto_analyzer->n_cfm_beams(); ++i)
		{
			RealFunctionF32::iterator it = full_elastogram_buffer->row(i).begin();
			RealFunctionF32::iterator it_mask = full_mask_buffer->row(i).begin();
			std::copy(it + roi_sample_start, it + roi_sample_end, elastogram.row(i).begin());
			std::copy(it_mask + roi_sample_start, it_mask + roi_sample_end, elasto_mask.row(i).begin());
		}
		tp_full_frame_proc->Stop();
		return elasto_status::ok;
	}
	catch(exception &ex)
	{
		PrintErrorInfo(ex);
		return elasto_status::operation_error;
	}
	catch(...)
	{
		return elasto_status::operation_error;
	}
}


string	ELASTO_DLL_EI GetTimeConsumptionReport()
{ 
	try
	{
		string time_consumption_report;
		physical_time	parts_sum = phase_elasto_analyzer->GetProcessingTime();

		if(tp_full_frame_proc->Count())
		{
			time_consumption_report += ssprintf("Frame processing time %g ms including:\n", tp_full_frame_proc->MeanElapsed().msec());
		}
		if(tp_arrays_alloc->Count())
		{
			parts_sum += tp_arrays_alloc->MeanElapsed();
			time_consumption_report += ssprintf("\tAlloc time %g ms\n", tp_arrays_alloc->MeanElapsed().msec());
		}

		time_consumption_report += phase_elasto_analyzer->GetTimeConsumptionReport();

		time_consumption_report += ssprintf("Parts sum is %g ms\n", parts_sum.msec());
		time_consumption_report += ssprintf("Difference is %g ms\n", (tp_full_frame_proc->MeanElapsed()-parts_sum).msec());

		return time_consumption_report;
	}
	catch(exception &ex)
	{
		PrintErrorInfo(ex);
		return string("Time consumption is unknown\n");
	}
	catch(...)
	{
		return string("Time consumption is unknown\n");
	}
}

bool	ELASTO_DLL_EI DataUnswept()
{
	return unsweep_on_read;
}

};//namespace ElastoGrafica
