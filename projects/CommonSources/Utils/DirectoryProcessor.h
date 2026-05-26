#ifndef XRAD__File_DirectoryProcessor_h
#define XRAD__File_DirectoryProcessor_h
//--------------------------------------------------------------

#include <XRADBasic/Core.h>

XRAD_BEGIN

//--------------------------------------------------------------

//! \brief Параметры для ProcessDirectories()
struct DirectoryProcessorOptions
{
	//! \brief Глубина рекурсии подкаталогов
	size_t depth = 0;

	DirectoryProcessorOptions() = default;
	DirectoryProcessorOptions(size_t depth): depth(depth) {}
};

//--------------------------------------------------------------

/*!
	\brief Обработать подкаталоги заданного каталога

	\param func Семантика вызова: func(absolute_path, relative_path, progress_start, progress_range).
		- absolute_path -- "абсолютный" путь: path + relative_path.
		- relative_path -- путь к подкаталогу относительно path.
		- progress_start, progress_range -- приблизительный прогресс для данного подкаталога в общем
			списке подкаталогов, прогресс должен переместиться от progress_start до
			progress_start + progress_range.

	Если options.depth == 0, то func вызывается непосредственно для path.

	Если в каком-то подкаталоге path нет каталогов нужного уровня вложенности, такой подкаталог
	пропускается, даже если в нем есть файлы.

	Исключение внутри func при обработке подкаталога прерывает выполнение всей функции.

	\note Передача func по значению сделана по аналогии с алгоритмами стандартной библиотеки C++.

	\note Если потребуется прерывать обработку каталогов штатным образом, следует добавить ещё одну
	функцию, у которой func будет возвращать bool (true — продолжитЬ, false — прервать), и
	сама функция будет возвращать bool.

	\note Для обработки каталогов с разным уровнем вложенности можно сделать по аналогии с командой
	find в UNIX два параметра: min_depth и max_depth. Только надо продумать логику работы с непустыми
	каталогами промежуточных уровней вложенности. Например: min_depth = 1, max_depth = 2, каталоги
	1, 1/1, 1/2. Нужно ли вызывать func для 1, если она вызывается для 1/1 и 1/2? Если да, то как
	передать в func информацию о том, что внутри этого каталога есть обрабатываемые подкаталоги?
*/
void ProcessDirectories(const string &path, const DirectoryProcessorOptions &options,
		function<void (const string &, const string &, double, double)> func);

//--------------------------------------------------------------

XRAD_END

//--------------------------------------------------------------
#endif // XRAD__File_DirectoryProcessor_h
