#include "pre.h"
#include "roi_vocabulary.h"
#include <XRADBasic/Sources/Utils/crayons.h>

/*!
	\file
	\date 2019/11/22 15:09
	\author kulberg

	\brief 
*/

XRAD_BEGIN

const double	desaturation = 0.25;


roi_type_vocabulary_t	mmg_vocabulary()
{
	return
	{
		{0, make_pair(L"Прочие находки", crayons::yellow()%desaturation) },
		{1, make_pair(L"ЗНО", crayons::red()) },
		{2, make_pair(L"Доброкачественное", crayons::green())},
		{3, make_pair(L"Подозрительные кальцинаты", crayons::yellow())},
		{4, make_pair(L"Патологически измененные лимфоузлы", crayons::magenta())},
		{5, make_pair(L"Утолщение кожи", crayons::cyan())}
	};
}

roi_type_vocabulary_t	lung_vocabulary()
{
	return
	{
		{-2, make_pair(L"unknown", crayons::white()) },
		{-1, make_pair(L"Цель исследования не легкие, а ребра, грудины", crayons::black())},
		{0, make_pair(L"Прочие находки", crayons::yellow()%desaturation) },
		{1, make_pair(L"Образования доброкачественные", crayons::green())},
		{2, make_pair(L"Образования злокачественные", crayons::red())},
		{3, make_pair(L"Образования неясного генеза", crayons::yellow())},
		{4, make_pair(L"Пневмонии", crayons::green()%desaturation)},
		{5, make_pair(L"Саркоидоз / интерстициальные заболевания легких", crayons::cyan()%desaturation)},
		{6, make_pair(L"Туберкулез / постутберкулез", crayons::dark_blue())},
		{7, make_pair(L"Гидроторокс", crayons::blue())},
		{8, make_pair(L"Пневмоторакс", crayons::gray_25())},
		{9, make_pair(L"Абсцесс", crayons::dark_blue())},
		{10, make_pair(L"Ателектатические изменения", crayons::magenta()%desaturation)},
		{11, make_pair(L"Синдром диссеминации", crayons::cyan())},
		{12, make_pair(L"Перелом", crayons::magenta())},
		{13, make_pair(L"Эмфизема", crayons::dark_yellow())},
		{14, make_pair(L"Отек легких", crayons::dark_cyan())},
		{15, make_pair(L"Аневризма", crayons::dark_red())},
		{16, make_pair(L"Атрезия бронха", crayons::dark_green())},
		{17, make_pair(L"спайки", crayons::violet())},
		{18, make_pair(L"пневмосклероз", crayons::orange())},
		{19, make_pair(L"пневмофиброз", crayons::turquoise())},
		{20, make_pair(L"бронхит", crayons::brown())},
		{21, make_pair(L"плеврит", crayons::dark_red())},
		{22, make_pair(L"застой по МКК", crayons::orange()%desaturation)},
		{23, make_pair(L"косвенные признаки ХОБЛ", crayons::turquoise()%desaturation)},
		{24, make_pair(L"образование скелетно-мышечной системы", crayons::violet()%desaturation)},
		{25, make_pair(L"резекция легкого", crayons::blue()%desaturation)},
		{26, make_pair(L"киста", crayons::green()%desaturation)},
		{27, make_pair(L"перисциссурит", crayons::red()%desaturation)}
	};
}


XRAD_END

