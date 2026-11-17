#ifndef	RFDataImport_h
#define	RFDataImport_h

#include "SectorData.h"
#include <RFDataImport/RFDataImporter.h>
#include <Utils/NonDiffraction.h>


XRAD_BEGIN

class	RFDataToSimIOImport : public SectorData
	{
	private:
	//	int	HT_To_Be_Done, n_Old_Samples;
	//	int	rays_Interpolation;
	//	int	n_Scanlines;
		physical_length	convexRadius;
		ComplexFunction2D_F32	downsampled_data;
		RFDataImporter *importer;
		non_diffraction_setup focusing_setup;
		bool	non_diffraction;
		int	downsample_ratio;

	public:	
	virtual	void	InitWork();
	virtual	void	EndWork();
	virtual	void	Batch();

		void	GetNonDiffractionParams();

		RFDataToSimIOImport();
		virtual ~RFDataToSimIOImport();
	};

XRAD_END


#endif