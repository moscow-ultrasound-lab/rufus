#include "pre.h"
#include <ProbeOptions/UltrasonicProbeOptions.h>
	
XRAD_BEGIN

namespace
	{
	string probe_types[n_probe_types] = 
		{
		string("unknown_probe"),
		// 1D
		string("single_element_1D_probe"),
		string("annular_1D_probe"),
		// 2D
		string("single_element_scanning_2D_probe"),
		string("annular_scanning_2D_probe"),
		string("phased_2D_probe"),
		string("linear_2D_probe"),
		string("convex_2D_probe"),
		
		//3D
		string("matrix_phased_3D_probe"),
		string("mechanical_convex_3D_probe")
		};

	ultrasonic_probe_type	RecognizeProbeType(const string &s)
		{
		for(size_t i = 0; i < n_probe_types; ++i)
			{
			if(s==probe_types[i]) return static_cast<ultrasonic_probe_type>(i);
			}
		return unknown_probe;
		}
}


void	UltrasonicProbeOptions :: LoadProbeOptions(IniFileReader &ir)
	{
	ir.set_section("PROBE_OPTIONS");
	probe_type = RecognizeProbeType(ir.read_string("probe_type"));
	probe_id = ir.read_string("probe_id", "");
	
	switch(probe_type)
		{
		case linear_2D_probe:
			scanning_radius = cm(ir.read_double("scanning_radius", 0));
			max_scan_length = cm(ir.read_double("max_scan_length"));

			max_scan_angle = degrees(ir.read_double("max_scan_angle", 0));			
			
			n_elements_total = ir.read_int("n_elements_total");
			n_elements_in_active_aperture = ir.read_int("n_elements_in_active_aperture");
			element_size = max_scan_length/n_elements_total;
			active_aperture_size = cm(ir.read_double("active_aperture_size", element_size.cm()*n_elements_in_active_aperture));
			
			if(scanning_radius.cm() || max_scan_angle.radians() ||
					active_aperture_size != element_size*n_elements_in_active_aperture)
				{
				throw ini_file_error("Invalid options for current probe type");
				}
			break;
			
		case convex_2D_probe:
			scanning_radius = cm(ir.read_double("scanning_radius"));

			max_scan_angle = degrees(ir.read_double("max_scan_angle"));
			max_scan_length = cm(ir.read_double("max_scan_length", scanning_radius.cm()*max_scan_angle.radians()));
			
			
			n_elements_total = ir.read_int("n_elements_total");
			n_elements_in_active_aperture = ir.read_int("n_elements_in_active_aperture");
			element_size = max_scan_length/n_elements_total;
			active_aperture_size = cm(ir.read_double("active_aperture_size", element_size.cm()*n_elements_in_active_aperture));
			
			if(active_aperture_size != element_size*n_elements_in_active_aperture ||
				max_scan_length != scanning_radius*max_scan_angle.radians() ||
				!max_scan_angle.radians())
				{
				throw ini_file_error("Invalid options for current probe type");
				}
			break;
			
		case phased_2D_probe:
			scanning_radius = cm(ir.read_double("scanning_radius", 0));
			max_scan_length = cm(ir.read_double("max_scan_length", 0));

			max_scan_angle = degrees(ir.read_double("max_scan_angle"));
			
			n_elements_total = ir.read_int("n_elements_total");
			n_elements_in_active_aperture = ir.read_int("n_elements_in_active_aperture");
			active_aperture_size = cm(ir.read_double("active_aperture_size"));
			element_size = active_aperture_size/n_elements_in_active_aperture;
			
			if(scanning_radius.cm() || max_scan_length.cm() || !element_size.cm() ||
				!max_scan_angle.radians())
				{
				throw ini_file_error("Invalid options for current probe type");
				}
			break;
		default:
			throw logic_error(ssprintf("UltrasonicProbeOptions::LoadProbeOptions -- probe type '%s' parameters reading not implemented", probe_types[probe_type].c_str()));
			break;
		}
	}

void	UltrasonicProbeOptions::InitProbeConvex(physical_length probe_radius, physical_angle probe_angle, size_t n_elements, size_t n_active_elements)
	{
	probe_type = convex_2D_probe;
	probe_id = "";
	scanning_radius = probe_radius;

	max_scan_angle = probe_angle;
	max_scan_length = probe_radius * probe_angle.radians();
	
	
	n_elements_total = n_elements;
	n_elements_in_active_aperture = n_active_elements;
	element_size = max_scan_length/n_elements_total;
	active_aperture_size = element_size*n_elements_in_active_aperture;
	
	}

void	UltrasonicProbeOptions::InitProbeLinear(physical_length probe_length, size_t n_elements, size_t n_active_elements)
	{
	probe_type = linear_2D_probe;
	probe_id = "";
	scanning_radius = cm(0);

	max_scan_angle = radians(0);
	max_scan_length = probe_length;
	
	
	n_elements_total = n_elements;
	n_elements_in_active_aperture = n_active_elements;
	element_size = max_scan_length/n_elements_total;
	active_aperture_size = element_size*n_elements_in_active_aperture;
	
	}


XRAD_END


