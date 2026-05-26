#ifndef __rasp_functors_h
#define __rasp_functors_h

#include <RASPFixedPoint.h>


//------------------------------------------------------------------------------
//
//	функторы для вызова алгоритмов
//
//------------------------------------------------------------------------------

namespace	RASPFunctors
{


	//-----------------------------------------------------------------------------
	//
	//	функтор "умножение со с двигом"
	//
	//-----------------------------------------------------------------------------

	class	shift_multiplier_base
		{
		public:
		int	correction_shift;
		double	factor;
		static const double	inv_log2;
		};

	template<class sample_t>
	class shift_multiplier : public shift_multiplier_base
		{
		public:

			shift_multiplier()
				{
				correction_shift = fixed_point_position(sample_t(0));
				factor = 1./double(1<<correction_shift);
				}
			shift_multiplier(int cs)
				{
				correction_shift = cs;
				factor = 1./double(1<<correction_shift);
				}

			shift_multiplier(int cs, double f)
				{
				correction_shift = cs;
				factor = 1./f;
				}

			inline double	&operator() (double &x, const double &y) const
				{
				return ((x*=y)*=factor);
				}

			inline float	&operator() (float &x, const float &y) const
				{
				return ((x*=y)*=factor);
				}

			inline int &operator() (int &x, const int &y) const
				{
				return (x*=y)>>=correction_shift;
				}

			inline short &operator() (short &x, const short &y) const
				{
				// очень важное изменение (см. ниже). потеря точности
				// была фатальная, разницы в быстродействии никакой
				//return (x*=y)>>=correction_shift;
				return x=(int(x)*int(y))>>correction_shift;
				}

		static int	calculate_correction_shift(double x)
			{
			if(!x) return fixed_point_position(sample_t(0));
			return range(fixed_point_position(sample_t(0)) - log(fabs(x))*inv_log2, 0, sample_width(sample_t(0))-2);
			// поправочный сдвиг при целочисленном умножении
			// не должен превышать (width-2) бит, поскольку, помимо знакового,
			// должен остаться хотя бы один значащий бит
			};

		};


template<class T1, class T2>
struct import_sample_functor : public binary_function<T1,T2,T1>
	{
	T1& operator()(T1& x, const T2& y) const {return ImportOriginalSample(x,y);}
	};

template<class T1, class T2>
struct export_sample_functor : public binary_function<T1,T2,T1>
	{
	T1& operator()(T1& x, const T2& y) const {return ExportProcessedSample(x,y);}
	};

 } //namespace RASPFunctors


#endif //__rasp_functors_h

