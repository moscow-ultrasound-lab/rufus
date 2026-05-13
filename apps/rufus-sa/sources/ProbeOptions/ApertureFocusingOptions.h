#ifndef	__frame_focusing_options_h
#define	__frame_focusing_options_h

#include <XRADBasic/ContainersAlgebra.h>

#include <XRADBasic/Sources/Utils/PhysicalUnits.h>
#include <XRADBasic/Sources/Utils/LeastSquares.h>

	
XRAD_BEGIN


using DistancesVector = MathFunction<physical_length, double, AlgebraicStructures::FieldTagScalar>;
using DistancesMatrix = MathFunction2D<DistancesVector>;

using TimeVector = MathFunction<physical_time, double, AlgebraicStructures::FieldTagScalar>;
using TimeTable = MathFunction2D<TimeVector>;



// typedef MathMatrix<physical_length, double, AlgebraicStructures::FieldTagScalar> DistancesMatrix;
// typedef LinearVector<physical_length, double, AlgebraicStructures::FieldTagScalar> DistancesVector;

// заготовка. данные сканированиЯ могут содержать несколько
// кадров, снЯтых при различной глубине фокусировки на передачу.

struct	MultifocusOptions : public DistancesVector
	{
	const size_t	&n_focuses;
	bool	static_direction;
	MultifocusOptions() : n_focuses(size()), static_direction(false){}
	// пустой контейнер должен соответствовать динамической фокусировке
	bool	dynamical_focusing(){return !n_focuses;}
	
	physical_length	GetFocus(physical_length target_depth)
		{
		if(dynamical_focusing()) return target_depth;
		else
			{
			size_t i(0);
			while(i < n_focuses && target_depth < at(i)) {++i;}
			if(!i) return at(0);
			else if(i>=n_focuses-1) return at(n_focuses-1);
			else
				{
				physical_length l1 = target_depth - at(i);
				physical_length l2 = at(i+1) - target_depth;
				if(l1<l2) return at(i);
				else return at(i+1);
				}
			}
		}
	};

#if 1
typedef MultifocusOptions aperture_focusing;
#else
struct	aperture_focusing
	{
	bool	dynamical;
	bool	static_direction;
	physical_length depth;
	
	aperture_focusing(){dynamical = true; static_direction = false; depth = cm(0);};
	};
#endif

class	focusing_defect
	{
		const int	n_elements;
		chebyshev_LS_polynom polynom;
	public:
		focusing_defect(int in_n_elements) : n_elements(in_n_elements), polynom(0, in_n_elements){}
		
		
		static const physical_speed	standard_sound_speed;
		physical_speed	deviated_sound_speed;
		
		LinearVector<physical_length, double, AlgebraicStructures::FieldTagScalar> polynomial_defect;

		physical_length	ComputeDelay(physical_length element_x, physical_length target_z, physical_length target_r, aperture_focusing focus) const;
	};


	


physical_length ComputeLensDelay(physical_length element_x, physical_length target_z, physical_length target_r, aperture_focusing focus);


XRAD_END


#endif //__frame_focusing_options_h
