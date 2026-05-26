#include "pre.h"
#include "ImageUtils.h"

XRAD_BEGIN



//	назначение этого файла следующее.
//	сюда переносятся из классов те функции обработки, которые ранее
//	были сделаны членами и которые теперь необходимо сделать шаблонами
//	от всех подходящих классов. вначале пусть они лежат в .cpp в своем
//	прежнем виде. когда появляется возможность переделать функцию в шаблон,
//	переносим ее в ImageUtils.hh





void	ComputeDistributionFunction(RealFunctionF32 &distribution_f, const RealFunctionF32 &histogram)
{
	int	s = histogram.size();
	distribution_f.MakeCopy(histogram);

#pragma message ("see comment")
// здесь функция плотности предварительно фильтруется от пиков, это дает лучший результат
// для большинства картинок. однако нужно предусмотреть вычисление и функции распределения
// без каких-либо оговорок, как есть

	distribution_f[0] = distribution_f[s-1] = 0;
	for(int i = 1; i < s-1; ++i)
	{
		if(distribution_f[i] > distribution_f[i-1] && distribution_f[i] > distribution_f[i+1]) distribution_f[i] = distribution_f[i+1];
	}
	if(distribution_f[0] > distribution_f[1]) distribution_f[0] = distribution_f[1];
	if(distribution_f[s-1] > distribution_f[s-2]) distribution_f[s-1] = distribution_f[s-2];

	for(int i = 1; i < s; ++i) distribution_f[i] += distribution_f[i-1];

	if(distribution_f[s-1])
	{
		distribution_f /= distribution_f[s-1];
	}
	else
	{
		for(int i = 0; i < s; ++i) distribution_f[i] = double(i)/s;
	}
}







XRAD_END