#ifndef FOCUS_OPTIONS_H
#define FOCUS_OPTIONS_H

#include "SectorData.h"
//#include "ApertureFocusingOptionsOld.h"
#include "SignalProcessing/RawSFDataSource.h"
//#include "Q:\projects\ATL\Sources\SignalProcessing/RawSFDataSource.h"

XRAD_BEGIN





class	SyntheticFocusingOptions : public SectorData
	{
public:

	size_t	n_subaperture_elements_active;
	size_t	n_subaperture_elements_full;

// 	int	first_raw_sample;		
// 	int	last_raw_sample;		
	
//	int	n_raw_samples;
	

	int	first_sample;	
	int	last_sample;

	bool	cancel_dc_offset_correction;
	size_t	n_merged_elements;
			// если >1, происходит искусственное загрубление элемента апертуры. нужно для оценки артефактов

	shared_ptr<RawSFDataSource>	data_source;

public:
	bool	apodization_tx, apodization_rx;
	physical_angle	sector_centre_angle;
	
	// Aberrations corrector options
	
//	double	sound_speed_defect;
	

	
	SyntheticFocusingOptions();
	virtual ~SyntheticFocusingOptions();
	
	void	Set_Default_Options();
	void	ShowOptions();
	void	UpdateFocusingSectorBounds();
virtual	void	Modify_Options();
	

private:
//	void	UpdateFocusPoints(focusing_dialog_item);
	//void	UpdateNSubapertures(focusing_dialog_item);
//	void	UpdateNRays(focusing_dialog_item);
//	void	UpdateSampleRate(focusing_dialog_item);
	
// virtual	void	CreateComment();
	};


XRAD_END

#endif