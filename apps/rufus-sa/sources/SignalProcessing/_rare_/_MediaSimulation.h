#ifndef	__media_simulation_h
#define	__media_simulation_h

#include "SectorData.h"
//#include "volume3D.h"
//#include "SmartEmitter.h"


XRAD_BEGIN

class	MediaSimulator : public SectorData
			//, public smartEmitter
				{

	void	ExportSignalParams();
	smartEmitter	emitter;
	
public:	
virtual	void	InitWork();
virtual	void	EndWork();
virtual	void	Batch();


	MediaSimulator();
	virtual ~MediaSimulator();
	};

XRAD_END


#endif //__media_simulation_h