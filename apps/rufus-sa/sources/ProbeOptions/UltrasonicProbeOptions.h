#ifndef	__ultrasonic_probe_options_h
#define	__ultrasonic_probe_options_h

#include <XRADBasic/Sources/Utils/PhysicalUnits.h>
#include <XRADSystem/Sources/IniFile/XRADIniFile.h>

XRAD_BEGIN


enum ultrasonic_probe_type
	{
	unknown_probe = 0,
	// 1D
	single_element_1D_probe,
	annular_1D_probe,
	// 2D
	single_element_scanning_2D_probe,
	annular_scanning_2D_probe,
	phased_2D_probe,
	linear_2D_probe,
	convex_2D_probe,
	
	//3D
	matrix_phased_3D_probe,
	mechanical_convex_3D_probe,
	//unfinished
	
	n_probe_types
	};


// заготовка
struct	UltrasonicProbeOptions
	{
	private:
		string probe_id;
		// совершенно абстрактнаЯ вещь
		
		ultrasonic_probe_type probe_type;
		
		physical_length	scanning_radius; //0 for phased array; infinity for linear also substituted by 0
//		physical_length	scanning_trajectory_length; //0 for phased array
		physical_length	max_scan_length; //0 for phased array
		
		size_t	n_elements_total;
		size_t	n_elements_in_active_aperture;
		
		physical_angle	max_scan_angle; //0 for linear array
//		physical_angle	end_scan_angle; //0 for linear array
		physical_length	element_size;
		physical_length	active_aperture_size;
		physical_frequency carrier_frequency;// номинальная частота датчика	
		
		//-------------------------------------------------------------------
	public:
		//initialization
		UltrasonicProbeOptions() : probe_type(unknown_probe), probe_id(""){}
		
		void	LoadProbeOptions(IniFileReader &ir);
		void	InitProbeConvex(physical_length probe_radius, physical_angle probe_angle, size_t n_elements, size_t n_active_elements);
		void	InitProbeLinear(physical_length probe_length, size_t n_elements, size_t n_active_elements);
		void	SetCarrier(const physical_frequency &in_carrier){carrier_frequency = in_carrier;}
		
		//access
		ultrasonic_probe_type	ProbeType() const{return probe_type;}
		
		physical_length ScanningRadius() const{return scanning_radius;}
		physical_length MaxScanLength() const{return max_scan_length;}
		physical_length	ElementSize() const {return element_size;}
		
		physical_length	ApertureSize() const {return active_aperture_size;}
		physical_frequency CarrierFrequency(){return carrier_frequency;};
		size_t	ApertureElements() const {return n_elements_in_active_aperture;}
		size_t	TotalElements() const {return n_elements_total;}
	};


XRAD_END


#endif //__ultrasonic_probe_options_h
