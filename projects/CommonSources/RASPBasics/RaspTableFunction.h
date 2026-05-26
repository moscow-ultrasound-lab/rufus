#ifndef __table_function_h
#define __table_function_h

XRAD_BEGIN

template<class T>
class	RaspTableFunction : public DataArray<T>
	{
    PARENT(DataArray<T>);
	double	parameter;

	#ifndef _DEBUG
	T	*base;
	//	экстремальный хулиганский способ быстрого доступа к элементам
	//	таблицы: поскольку мы знаем, что шаг у нее всегда равен 1, вместо
	//	at() обращаемся напрямую к указателю на данные.
	#endif //DEBUG

private:
	int	table_order;

	enum	function_t
		{
		not_initialized_function,
		power_function,
		tresholding_function
		};
	function_t	function_type;

public:
	const int	table_bits;

	RaspTableFunction(int tb);
//	~RaspTableFunction(){}

	inline	void InitPowerFunction(double power);
	inline	void InitTresholdingFunction(double treshold);
		// эта функция, видимо, пока не будет использоваться.
		// ее назначение -- плавное нелинейное ограничение амплитуды

	inline const T &operator()(int x) const;
	inline T &operator()(T& y, int x) const;
	inline T normalizer()const {return T(table_order-1);}
	};




XRAD_END

#include "RaspTableFunction.cc"

#endif //__table_function_h

