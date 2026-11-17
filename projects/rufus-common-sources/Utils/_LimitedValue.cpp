#include "pre.h"
#include <RealFunction.h>
#include "LimitedValue.h"

#error obsolete

void	CheckLimited()
	{
	range_float	valf(10,20);
	range_int	vali(10,20);
//	float		f_set[4] = {1.5, 2.3, 0.1, -1};
	float		f_set[4] = {1,2,3,4};
	int		i_set[6] = {1,2,3,4,5,6};
	indexed_float	set_ind_f(4, f_set);
	float_set	set2(4, f_set);
	integer_set	set3(6, i_set);
	int	s = 2000;
	RealFunction	f(s);
	
	float	x0 = -10;
	float	dx = 0.01;
	for(int i = 0; i < s; i++)
		{
		f[i] = valf(float(i)/10);
		//f[i] = set2(i%7-1);
		f[i] = set_ind_f(x0 + dx*i);
		}
	
	f.Display(x0,dx, "result");
	DebugQuit();
	}
