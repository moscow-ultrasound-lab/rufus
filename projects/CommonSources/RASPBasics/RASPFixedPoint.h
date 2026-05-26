#ifndef __fixed_point_h
#define __fixed_point_h

#error

//-------------------------------------------------------------
//
//	здесь описаны средства, обеспечивающие
//	работу алгоритмов RASP в режимах
//	с плавающей и с фиксированной запятой
//
//	существует три типа данных: "внешний отсчет", "внутренний отсчет"
//	и "всегда плавающий"
//
//	особенности импорта входных значений:
//	плавающие только в плавающие.
//	целочисленные либо в целые большей разрядности (со сдвигом),
//	либо в плавающие (без сдвига).
//	!!!величина сдвига зависит от разрядности
//	!!!обеих величин!
//
//	особенности импорта множителей из плавающей во внутренние отсчеты.
//	во-первых, величины, идущие в масштабе значений данных (например,
//	пороговые значения усиления).
//	во-вторых, масштабные множители (например, border_enhance_factor,
//	peak_enhance_factor). их следует преобразовать, чтобы умножение в дальнейшем
//	осуществлялось между однородными величинами
//




//	ранее это была структура с внутренним полем int32_t,
//	которая отвечала за действия с фиксированной запятой.
//	оказалось, что элементарные действия, например конструктор от void
//	в CodeComposer занимают несоразмерно много времени. явно ошибка
//	оптимизации их компилятора! но пока откажемся от класса и будем
//	использовать просто int32

XRAD_BEGIN

	//-------------------------------------------------
    //
	//	информация о целочисленных типах, нужная
	//	при действиях с фиксированной запятой: явно заданные константы
	//


	enum
		{
		//	ширина поля в битах
		sample_width_int = 32,
		sample_width_short = 16,
		sample_width_char = 8,

		//	положение десятичной точки справа
		//	(ранее было равно width/4 - 1
		//	сейчас все вычисления производятся в контейнере int32_t, поэтому
		//	точность 16-битных данных можно повысить
		point_position_32 = 7,
		point_position_16 = 6,//3
		point_position_8 = 1,

		//	сдвиг при импорте из меньшего целого контейнера
		//  в больший

		offset_8_to_32 = 7,
		offset_16_to_32 = 3,
		offset_32_to_32 = 1,

		offset_8_to_16 = 6,//3,
		offset_16_to_16 = 1,

		offset_8_to_8 = 1, 

		//	дозволенные диапазоны значений
		//	для основных целочисленных типов

		//	можно было бы обойтись без задания констант
		//	(вычислять через сдвиг), но тогда пришлось бы
		//	вычислять этот сдвиг на каждом отсчете
		//	обрабатываемых данных
		min_ui8 = 0,
		max_ui8 = 255,
		min_si8 = -127,
		max_si8 = 127,

		min_ui16 = 0,
		max_ui16 = 65535,
		min_si16 = -32767,
		max_si16 = 32767,

		min_ui32 = 0,
		max_ui32 = 0xFFFFFFFFu,
		min_si32 = -0x7FFFFFFF,
		max_si32 = 0x7FFFFFFF
		};

	//-------------------------------------------------------------------------
    //
	//	следующая функция проверяет, чтобы размер целочисленных типов
	//	соответствовал требуемым в данном случае. если на каком-либо
	//	компиляторе здесь возникнет ошибка, это явится поводом для
	//	использования типов с явным заданием разрядности (__int32 например)
	//

	inline	void	*dummy_type_width_check()
		{
		void	*ptr1 = (sample_width_int - sizeof(int32_t)*8);
		void	*ptr2 = (sample_width_short - sizeof(int16_t)*8);
		void	*ptr3 = (sample_width_char - sizeof(char)*8);

		// во избежание warnings "объявлено, но не используется"
		if (ptr1) return ptr2;
		else return ptr3;
		}

	//-------------------------------------------------------------------------
	//
	//	ширина отсчета в битах
	//

	inline	int32_t	sample_width(int32_t){return sample_width_int;}
	inline	int32_t	sample_width(int16_t){return sample_width_short;}
	inline	int32_t	sample_width(char){return sample_width_char;}

	inline	int32_t	sample_width(uint32_t){return sample_width_int;}
	inline	int32_t	sample_width(uint16_t){return sample_width_short;}
	inline	int32_t	sample_width(uint8_t){return sample_width_char;}

	//-------------------------------------------------------------------------
    //
	//	сдвиг при целочисленном умножении
	//	или (что то же самое) положение фиксированной
	//	точки в числе
	//

	inline	int32_t	fixed_point_position(int32_t){return point_position_32;}
	inline	int32_t	fixed_point_position(uint32_t){return point_position_32;}
	inline	int32_t	fixed_point_position(int16_t){return point_position_16;}
	inline	int32_t	fixed_point_position(uint16_t){return point_position_16;}

    // эти две для совместимости, еще не протестировано, как с ними будет
	inline	int32_t	fixed_point_position(float){return 0;}
	inline	int32_t	fixed_point_position(double){return 0;}


	//--------------------------------------------------------------------------
	//
	//	корректный диапазон для целочисленных типов
	//

	template<class T>
	inline	uint8_t&	proper_range(uint8_t& x, const T& y){return x = range(y, min_ui8, max_ui8);}
	template<class T>
	inline	char&	proper_range(char& x, const T& y){return x = range(y, min_si8, max_si8);}

	template<class T>
	inline	uint16_t&	proper_range(uint16_t& x, const T& y){return x = range(y, min_ui16, max_ui16);}
	template<class T>
	inline	int16_t&	proper_range(int16_t& x, const T& y){return x = range(y, min_si16, max_si16);}

	template<class T>
	inline	uint32_t&	proper_range(uint32_t& x, const T& y){return x = range(y, min_ui32, max_ui32);}
	template<class T>
	inline	int32_t&	proper_range(int32_t &x, const T& y){return x = range(y, min_si32, max_si32);}

	template<class T>
	inline	float&	proper_range(float& x, const T& y){return x = y;}
	template<class T>
	inline	double&	proper_range(double& x, const T& y){return x = y;}


	//--------------------------------------------------------------------------
	//
	//	корректный сдвиг для конкретной пары типов
	//	(выписываю только для тех пар, что будут использоваться)
	//
	//	!!! наличие warning при компиляции этих функций означает, что
	//	!!! они вызываются неправильно! нужно проверять вызывающую процедуру
	//

	inline int32_t	conversion_offset(const int32_t&, const uint8_t&){return offset_8_to_32;}
	inline int32_t	conversion_offset(const int32_t&, const char &){return offset_8_to_32;}
	inline int32_t	conversion_offset(const uint32_t&, const uint8_t&){return offset_8_to_32;}
	inline int32_t	conversion_offset(const uint32_t&, const char &){return offset_8_to_32;}

	inline int32_t	conversion_offset(const int32_t&, const uint16_t&){return offset_16_to_32;}
	inline int32_t	conversion_offset(const int32_t&, const int16_t&){return offset_16_to_32;}
	inline int32_t	conversion_offset(const uint32_t&, const uint16_t&){return offset_16_to_32;}
	inline int32_t	conversion_offset(const uint32_t&, const int16_t&){return offset_16_to_32;}

	inline int32_t	conversion_offset(const int16_t&, const uint8_t&){return offset_8_to_16;}
	inline int32_t	conversion_offset(const int16_t&, const char&){return offset_8_to_16;}
	inline int32_t	conversion_offset(const uint16_t&, const uint8_t&){return offset_8_to_16;}
	inline int32_t	conversion_offset(const uint16_t&, const char&){return offset_8_to_16;}

	//--------------------------------------------------------------------------
    //
	//	нормировочный множитель, который вычисляется в тех случаях, когда требуется
	//	умножить текущий отсчет на число с плавающей запятой. процедура:
	//
	//	float	factor;
	//	DataArray<sample_t>	data;
	//	sample_t	intermediate_factor = factor*multiply_normalizer();
	//	for(int32_t i = 0; i < data.size(); ++i)
	//		{
	//		data[i] = multiply(data[i], intermediate_factor);
	//		}
	//

	inline float	multiply_normalizer(float){return 1;}
	inline double	multiply_normalizer(double){return 1;}

	template<class T>
		inline T	multiply_normalizer(const T&x){return 1<<fixed_point_position(x);}


	//--------------------------------------------------------------------------
    //
	//	алгоритмы умножения с плавающей и фиксированной запятой
	//

	inline float&	multiply(float &z, const float& x, const float& y){return z = x*y;}
	inline double&	multiply(double& z, const double& x, const double& y){return z = x*y;}

	template<class T>
		inline T	&multiply(T& z, const T& x, const T& y){return z=(x*y)>>fixed_point_position(T());}


XRAD_END





//DataArray2D.h

#endif //__fixed_point_h

