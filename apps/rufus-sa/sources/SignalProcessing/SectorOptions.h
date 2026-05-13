#ifndef __sector_options_h
#define __sector_options_h

#include "ProbeOptions/ApertureFocusingOptions.h"
//#include "Q:\projects\ATL\Sources\ProbeOptions/ApertureFocusingOptions.h"
#include <XRADBasic/Sources/ScanConverter/ScanAreaGeometry.h>

XRAD_BEGIN

class	SectorOptions : public PhysicalFrameDimensions
	{
private:

//	physical_length	_r_min;
//	physical_length	_r_max;	

public:
	SectorOptions();	
	virtual ~SectorOptions(){}

//	physical_length	r_min(){return _r_min;};
//	physical_length	r_max(){return _r_min;};
//	physical_length	depth_range(){return _r_max - _r_min;};

//	physical_angle start_angle;
//	physical_angle end_angle;


//	physical_length	TX_Focus;
//	physical_length	RX_Focus;
//	bool	TX_Dynamic_Focus, RX_Dynamic_Focus;

	};

XRAD_END

#endif //__sector_options_h
