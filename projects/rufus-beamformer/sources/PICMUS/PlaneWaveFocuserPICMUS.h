
//#ifndef PlaneWaveFocuserPICMUS_h__
//#define PlaneWaveFocuserPICMUS_h__


//#include "SignalProcessing/SyntheticApertureFocuserSectorProbe.h"
//#include "PICMUS/RawSFDataSourcePICMUS.h"

//XRAD_BEGIN
/*
class PlaneWaveFocuser : public SyntheticApertureBeamformer
{
	PARENT(SyntheticApertureBeamformer);

public:
	PlaneWaveFocuser();
	virtual ~PlaneWaveFocuser();

	virtual void InitWork() override;
	virtual void Batch() override;
	virtual void EndWork() override;

protected:
	//ComplexFunction2D_F32 PlaneWaveDASFocus(RawSFDataSourcePICMUS& source, int angle_filter);
	ComplexFunction2D_F32 PlaneWaveDASFocus(RawSFDataSource& source, int angle_filter);
	bool IsPlaneWaveSource() const;

	int angle_filter = -1;     
	size_t bf_choice = 0;      
};
*/

//XRAD_END

//#endif
