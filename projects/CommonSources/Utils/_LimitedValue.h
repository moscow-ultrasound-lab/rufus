#ifndef	__limited_value_h
#define	__limited_value_h

#error obsolete

#include <DataArray.h>

//----------------------------------------
//
//	идеЯ такова: в классе хранитсЯ число T_internal value
//	оператор преобразованиЯ к T_external возвращает value
//	с заданным преобразованием (функциЯ, ограничение по значению и др.).
//	все остальные операторы (=,+-*/) работают с внутренним value.
//
//----------------------------------------

XRAD_BEGIN


template<class T_internal, class T_external>
class	value_transformation{
private:
	virtual	const T_external &transform_function(const T_internal &) const = 0;

protected:
	value_transformation(){}
public:
	T_external operator()(const T_internal &v) const;
        void	correct(T_internal &v) const;
	};





//------------------------------------------------------
//
//	величина, лежащаЯ в определенном диапазоне
//

template <class T>
class range_value : public value_transformation<T, T>{
//	typedef value_transformation<T, T> parent;
const	T	min, max;
	const T	&transform_function(const T &v) const;

public:
	range_value(const T &mn, const T &mx);
	};


//------------------------------------------------------

typedef range_value<int> range_int;
typedef range_value<float> range_float;
typedef range_value<double> range_double;




//------------------------------------------------------
//
//	величина, принимающаЯ фиксированное кол-во N значений
//	поиск осуществлЯетсЯ по индексу в массиве
//	инициализируетсЯ от Явно заданного массива или от DataArray

template<class T>
class	indexed_value : public value_transformation<int, T>,
			private DataArray<T>{
private:
	const T	&transform_function(const int &v) const;
public:
	indexed_value(int n, const T *values);
	indexed_value(const DataArray<T> &);
	int	count() const;
	};

typedef	indexed_value<double> indexed_double;
typedef	indexed_value<float> indexed_float;
typedef	indexed_value<int> indexed_integer;


//------------------------------------------------------
//
//	величина, принимающаЯ фиксированное кол-во N значений
//	поиск по ближайшему значению
//	инициализируетсЯ от Явно заданного массива или от DataArray

template<class T>
class	value_set : public value_transformation<T, T>,
			private DataArray<T>{
private:
	const T	&transform_function(const T &v) const;
public:
	value_set(int n, const T *values);
	value_set(const DataArray<T> &);
	int	count() const;
	};

typedef	value_set<float> float_set;
typedef	value_set<double> double_set;
typedef	value_set<int> integer_set;


XRAD_END

#include "LimitedValue.cc"

#endif	//__limited_value_h