#include "pre.h"
#pragma hdrstop

//#include "A4000File.h"
#include <XRADBasic/ContainersAlgebra.h>

XRAD_BEGIN


/*
iot_to_str
str_to_io
*/

typedef	DataArray2D<DataArray<int> > arr_t;
//typedef	RealFunction2D_F32 arr_t;

//---------------------------------------------------------------------------
/*
void	WriteA4000File(const char *filename)
	{
	arr_t	m;
	ScanParams sp;
        PatientInfo pi;

	A4000DataWriter<int, arr_t> aw(sp, 10);

	aw.SetDataFormat(ioPCInt16);
	aw.SetDestination("c:\\temp\\TestData");
	aw.BeginWriting();
        aw.WriteFrame(m);
	aw.EndWriting();
	}
*/



XRAD_END


