#include "pre.h"
#include <conio.h>
#include "q:\Projects\ElastoGrafica\ElastoGrafica\ElastoGrafica.h"

using namespace ElastoGrafica;

void	CheckElastoStatus(elasto_status status)
	{
	if(status != elasto_status::ok)
		{
		printf("Elastography error happened, status = %d\n", status);
		}
	else
		{
		printf("Elasto operation OK\n");
		}
	}



int main()
	{
	// размеры кадра
	int	n_cfm_beams = 70;
	int	n_cfm_samples = 400;
	int	n_cfm_shots = 9;
	int	n_beams_in_sweep = 5;
	
//	DebugBreak();
//	ForceDebugBreak();	
	// выделяем место под "данные ЦДК"
	int	in_data_size = n_cfm_beams*n_cfm_samples*n_cfm_shots*2;
	int *in_cfm_data_ptr = new int[in_data_size];
	
	// выделяем место под эластограмму
	int	elastogram_size = n_cfm_beams*n_cfm_samples;
	float *elastogram_ptr = new float[elastogram_size];
	float *mask_ptr = new float[elastogram_size];
		
	// место под переменную, в которую кладется информация
	// о том, как в целом двигался образец в момент получения данных.
	// это информация для графика
	float	frame_offset_for_graph;
	
	// величина для проверки
	elasto_status status;
	
	// инициализируем библиотеку
	status = InitElastographyLib("c:\\temp\\ElastoGrafica.ini");
//								c:\\temp\\ElastoGrafica.ini
	CheckElastoStatus(status);

	// два принципиально важных параметра, которые надо обязательно
	// "дать в руки" оператору
	frame_averaging_t	elastogram_agility = elasto_frame_averaging_2;
	axial_blur_t	axial_blur = elasto_blur_2;
	
	// передаем их в библиотеку (там, конечно, есть значения по умолчанию,
	// они рабочие, но все-таки лучше явно задать руками)
	status = SetElastogramAgility(elastogram_agility);
	CheckElastoStatus(status);
	status = SetElastogramBlur(axial_blur);
	CheckElastoStatus(status);
	
	// сообщаем библиотеке размеры обрабатываемого кадра
	status = SetFrameSizes(n_cfm_shots, n_cfm_beams, n_cfm_samples, n_beams_in_sweep, 0, 0, n_cfm_samples);
	CheckElastoStatus(status);
	// сбрасываем буфер накопления. в принципе, функция SetFrameSizes этот сброс
	// внутри себя автоматически делает, если размеры кадра поменялись по сравнению
	// с ранее бывшими. Но возможно такое: оператор нажал паузу, перенес датчик
	// в другое место, снова запустил исследование. Размер буферов не меняется,
	// и в этом случае, если не сделать сброс, программа подумает, что это
	// продолжение прежних измерений. тогда первый кадр достанется нам в наследство
	// от предыдущего измерения и плавно затухнет. Очень может сбить с толку, поэтому лучше
	// везде, где предполагается резкая смена ракурса, делать сброс.
	status = ResetElastogram();
	CheckElastoStatus(status);
	
	int	n_frames_to_process = 10;
	for(int frame=0;frame < n_frames_to_process;++frame)
		{
		// создаем "данные ЦДК": выделяем память и заполняем
		// случайными числами
		int	*it = in_cfm_data_ptr;
		int	*ie = in_cfm_data_ptr+in_data_size;;
		for(; it < ie; ++it) *it = int(rand()) - RAND_MAX/2;
		status = BuildElastogram(in_cfm_data_ptr, elastogram_ptr, &frame_offset_for_graph, mask_ptr);
		CheckElastoStatus(status);
		}

	// кончив работу, закрываем библиотеку 
	status = FinishElastographyLib();
	
	// на случай проблем с быстродействием,
	// можно получить детальный отчет о времени работы основных частей алгоритма
	string	time_consumption = GetTimeConsumptionReport();
	
	printf("Time consumption report\n%s", time_consumption.c_str());
	
	
	delete[] in_cfm_data_ptr;
	delete[] elastogram_ptr;
	delete[] mask_ptr;
	
	fflush(stdout);
	getch();
	}