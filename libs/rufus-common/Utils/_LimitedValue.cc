#error obsolete
XRAD_BEGIN


#define aValue_transformation value_transformation<T_internal, T_external>

template<class T_internal, class T_external>
T_external	aValue_transformation :: operator ()  (const T_internal &v) const{
	return transform_function(v);
	}

template<class T_internal, class T_external>
void	aValue_transformation :: correct(T_internal &v) const{
	v = transform_function(v);
	}

//-------------------------------------------------------------------------------------

#undef aValue_transformation




//---------------------------------------------------
//
//	range_value

#define aRange_value range_value<T>

template <class T>
const T &aRange_value :: transform_function(const T &v) const{
	return v<min ? min : v>max ? max : v;
	}

template <class T>
aRange_value :: range_value(const T &mn, const T &mx) : min(mn), max(mx)
	{
	}


//---------------------------------------------------
//
//	indexed_value
//	индексированный набор n значений.
//	инициализируетсЯ от Явно заданного массива или от DataArray



#define aIndexedValue indexed_value<T>


template <class T>
aIndexedValue :: indexed_value(int n, const T* values) : DataArray<T>(n)
	{
	if(values)for(int i = 0; i < n; i++) at(i) = values[i];
	}

template <class T>
aIndexedValue :: indexed_value(const DataArray<T> &original) : DataArray<T>(original)
	{
	}

template <class T>
const T	&aIndexedValue :: transform_function(const int &v) const{
	return at_fast(range(v, 0, size()-1));
	}

template <class T>
int	aIndexedValue :: count() const{
	return size();
	}


#undef aIndexedValue


//------------------------------------------------------
//
//	величина, принимающаЯ фиксированное кол-во N значений
//	инициализируетсЯ от Явно заданного массива или от DataArray
//

#define	aValueSet value_set<T>

template <class T>
aValueSet :: value_set(int n, const T* values) : DataArray<T>(n)
	{
	for(int i = 0; i < n; i++) at(i) = values[i];
	}

template <class T>
aValueSet :: value_set(const DataArray<T> &original) : DataArray<T>(original)
	{
	}


template <class T>
const T	&aValueSet :: transform_function(const T &v) const{
	int	result_index = 0;
	T	delta;
	if(at_fast(0) < v) delta = v-at_fast(0);
	else delta = at_fast(0) - v;
	for(int i = 0; i < size(); i++)
		{
		T	v1 = at_fast(i);
		if(v1 == v) return v;
		if(v1 < v)
			{
			if(v-v1 < delta) result_index = i, delta = v-v1;
			}
		else{
			if(v1-v < delta) result_index = i, delta = v1-v;
			}
		}
	return at_fast(result_index);
	}

template <class T>
int	aValueSet :: count() const{
	return size();
	}



//---------------------------------------------------



XRAD_END