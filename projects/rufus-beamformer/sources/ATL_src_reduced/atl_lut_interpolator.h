/*
File name : interpolators.h		

Created :	6-Feb-91
Modified :	10-Apr-91	Add Stardent version.   The MAC version uses look-up
						to do the multiplication, whereas the Stardent version
						is actually faster doing the multiplication (because
						it is an (8-tap) FIR filter that can be vectorised).
*/




/*
Note : There are 4 possible types of implementations of the algorithm as
specified by the compiler flags LUT_ALGORITHM & SYMMETRY.
LUT_ALGORITHM uses look-up of byte data to perform the multiplications
for the FIR filter.   Typically used on the Mac.
!LUT_ALGORITHM uses actual multiplys to perform the multiplications
for the FIR filter.   Typically used on the Stardent as it can use
floating point arithmetic which allows the loop to vectorise.
SYMMETRY make use of symmetry between left/right sector & TX/RX element
to save geometry calculations.   Typically used on the Mac.
!SYMMETRY does not use of symmetry, which means redundant calculations.
Typically used on the Stardent, However because loops are simplified
helping the Stardent of optimise (vectorise/parallelise).	
*/
#if 1 //defined(THINK_CPLUS) || defined(__powerc)
//2015_05_30 всегда LUT_ALGORITHM. остальные совсем неактуальны
#define		LUT_ALGORITHM
#define		SYMMETRY
#else
#undef		LUT_ALGORITHM
#undef		SYMMETRY
#endif



/*
common values for both LUT & multiplication algorithms
*/
#define	N_Taps_LUT				8L	/* # taps with have non-zero data	*/
#define	LUT_Entries_Per_Tap			256L	/* data spans -128 to +127		*/
#define	Analytic_LUT_size			65536L	/* 256 data * 128 taps * 2 filters	*/
#define	Real_LUT_size				32768L	/* 256 data * 128 taps			*/
#define	Quadrature_LUT_size			32768L	/* 256 data * 128 taps			*/
#define LUT_Interpolation_Factor		16L	/* 128 total taps/8 non-zero taps	*/
#define Centre_Offset				3L	/* Offset of input sample @ filter	*/
							/* centre (for 0 interpolation)		*/
							/* from 1st input sample		*/




#ifdef	LUT_ALGORITHM			/* look-up (presumably on MAC)				*/

/*
The lookup table which implements the multiplication of the analytic FIR 
interpolation filters is organised as follows :
Filters 							2		in-phase & imaginary.
	Interpolation point				16		0 to 15.
		Tap							8		0 to 7.
			Input level				256		-128 to 127.
Each entry represents the multiplication of one of the 256 possible input 
levels (ie: -128 to +127) by a filter coefficient.   There are 8 filter 
coefficients for each of the 16 interplation points.   Therefore, table is 
2*16*8*256 = 64 k (int) entries = 256 kBytes.
The lookup table which implements the multiplication of the Real FIR 
interpolation filter only contains the in-phase filter (with potentially
different scaling) and is half as long as the analytic table.
*/
#define Entries_Per_Interp_Pnt	2048L	/* 256 data values * 8 taps			*/
#define	Entries_Per_Filter		32768L	/* 256 data values*128 total # taps	*/
#define Data_Offset				128L	/* add this offset to pointer into 	*/
										/* table so that when data value is	*/
										/* added, points to correct entry	*/

typedef	char	IN_DATA_TYPE;
typedef	int	INTRP_TABLE_TYPE;

#else							/* multiplication (presumably on Stardent)	*/

/*
The filter coefficients are ordered as follows :
Filters 							2		in-phase & imaginary.
	Interpolation point				16		0 to 15.
		Tap							8		0 to 7.
The coefficient table for the Real FIR interpolation filter only contains 
the in-phase filter (with potentially different scaling) and is half as 
long as the analytic table.
*/
#define	Entries_Per_Interp_Pnt	8L		/* 8 taps							*/
#define	Entries_Per_Filter		128		/* 8*16 = 128 total # taps			*/
#define LUT_to_Table_Size_Ratio	256L	/* ratio of # entries in the LUT	*/
										/* versus the coefficient table		*/
#define Data_Offset				0		/* not applicable for mult algrithm	*/
										/* but include to make things work	*/

/*
typedef	double	IN_DATA_TYPE;
*/
typedef	float	IN_DATA_TYPE;			/* TEST ON MAC						*/
typedef	double	INTRP_TABLE_TYPE;


#endif


#define Analytic_LUT_Filename	"Analytic.LUT"
#define Real_LUT_Filename		"Real.LUT"
#define Quadrature_LUT_Filename	"Quadrature.LUT"


/*
function prototypes
*/
void	analytic_interpolate(IN_DATA_TYPE *, INTRP_TABLE_TYPE *, double *, 
										double *);
double	real_interpolate(IN_DATA_TYPE *, INTRP_TABLE_TYPE *);

void	FFT_real_double(double *, int, int);
void	FFT_complex_double(double *, int, int);

