#ifndef __signal_processor_h
#define __signal_processor_h

#include "SectorData.h"

XRAD_BEGIN


class	SignalProcessor : public SectorData
	{
		
	void	ShowDataFileInfo();	

	protected:


	virtual	void	Batch();
	virtual	void	InitWork();
	virtual	void	EndWork();

	public:	
	virtual	void	Display(const char *);
		SignalProcessor();	
		virtual ~SignalProcessor();	
		
	};
//-----------------------------------------------------------------------------
//
//	даннаЯ утилита компенсирует движение (в том числе, неравномерное)
//	тканей и датчика, которое может приводить к смазыванию картины при
//	временном накоплении данных.
//
//	пока недоработана!!!
//
//-----------------------------------------------------------------------------

void	StabilizeSignalComponents(SignalProcessor &sp);


XRAD_END

	
#endif //__signal_processor_h
