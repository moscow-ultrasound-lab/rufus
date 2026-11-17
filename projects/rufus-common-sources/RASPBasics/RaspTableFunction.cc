//RaspTableFunction.cc


XRAD_BEGIN


//---------------------------------------
//
//	табличная функция
//
//---------------------------------------

template<class T>
RaspTableFunction<T> :: RaspTableFunction(int tb) :
		table_bits(tb)
	{
	table_order = (1<<table_bits);
	realloc(table_order);
	#ifndef _DEBUG
	base = &at(0);
	#endif
	parameter = 0;
    function_type = not_initialized_function;
	}

template<class T>
inline void	RaspTableFunction<T> :: InitPowerFunction(double power)
	{
	if(function_type != power_function || parameter != power)
		{
		if(power==0)
			{
			fill(table_order-1);
			}
		else
			{
			double	normalizer = table_order-1;
			for(int i = 0; i < table_order; i++)
				{
				at(i) = T(pow(double(i)/normalizer, power)*normalizer);
				}
			}

		parameter = power;
        function_type = power_function;
		}
	}


template<class T>
inline T& RaspTableFunction<T> :: operator() (T& y, int x) const
	{
	#ifndef _DEBUG
	return y = base[x];
	#else
	return y = at(x);
	#endif
	}



XRAD_END
