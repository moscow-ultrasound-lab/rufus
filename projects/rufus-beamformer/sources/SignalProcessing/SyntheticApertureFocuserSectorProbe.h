#ifndef	__focuser_sector_probe_h
#define	__focuser_sector_probe_h

#include "SyntheticFocusingOptions.h"
#include "LUT_8bit_interpolator.h"
#include "SyntheticApertureFocuser.h"
#include "RawSFDataSourceTransEcho.h"
#include "DirectPhaseEstimation.h"
#include "PolynomialNarrowing.h"

XRAD_BEGIN

//#error continue from here

RealFunction2D_F64	RearrangeMatrixCorrection(RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected, int number_of_rays);

void DisplayTrapezoid(ComplexFunction2D_F32& image, string title, physical_length scan_width, physical_length scan_depth, double angle_deg); // полуугол

class	SyntheticApertureBeamformer : public SyntheticApertureFocuser
{
	PARENT(SyntheticFocusingOptions);

protected:
	virtual void					GetAreaForCorrection(CorrectionTraits &traits);
	virtual void					GetAreaForCorrectionSoS(CorrectionTraits& traits);
	virtual void					PreparationforCorrectionSoS(CorrectionTraits& traits, double parametr_min_sos, double parametr_max_sos, double parametr_step_sos);
	virtual void					DisplaySoSMap(RealFunction2D_F64& data);

	virtual	void					Batch();
	virtual	void					DisplayFrameLinearProbe(RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected, string title);
	virtual	void					ChooseOption(RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected);
	virtual	void					InitWork();

	virtual void					PlaneWaveInitWork();
	virtual void					PlaneWaveBatch();
	ComplexFunction2D_F32			PlaneWaveDASFocus(RawSFDataSource& source, int angle_filter);

	virtual void					CorrectionSoS(CorrectionTraits& traits, int j, double parametr_min_sos, double parametr_max_sos);
	virtual void					Narrowing_SoS(CorrectionTraits& traits);
	virtual void					Estimation_SoS(RealFunction2D_F64& matrix_correction, RealFunctionF32& rays_corrected);
	
	virtual	void					Sharpness(RealFunction2D_F64 &matrix_correction, RealFunctionF32 &rays_corrected);
	virtual	void					DisplayFrameSectorProbe(RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected, string title);
	virtual	void					CalculateFrameRice(RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected, string title);
	virtual void					CalibrationSourceMethod(RealFunction2D_F64 &matrix_correction, RealFunction2D_F64 &best_fit_matrix_correction, RealFunctionF32 &rays_corrected, physical_angle half_scanning_sector);

	virtual	ComplexFunction2D_F32	SectorProbeDASFocus(int first_sample, int last_sample, int n_lines, RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected, RawSFDataSource &current_source);
	//virtual ComplexFunction2D_F32	LinearProbeDASFocus(int first_sample, int last_sample, int n_lines, RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected, RawSFDataSource& current_source);
	virtual ComplexFunction2D_F32	PseudoConvexLinearProbeDASFocus(int first_sample, int last_sample, int n_lines, RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected, RawSFDataSource& current_source);
	virtual	RealFunctionF64			GaussFit(double &best_fit_sq_error, RealFunctionF64 line, size_t ray_no);
	virtual	RealFunctionF64			GetLine(GraphSet &GS, int first_sample, int last_sample, int n_lines, RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected);

	virtual	void					Correction(CorrectionTraits &traits, int j, double min);
	
public:
	virtual void					DirectPhaseEstimation(CorrectionTraits &traits);
	virtual	void					DirectPhaseEstimationFullFrame(CorrectionTraits &traits);
	
	virtual void PolynomialNarrowing(CorrectionTraits &traits);

	bool	dynamic_focal_depth_transmit = true;
	bool	dynamic_focal_depth_receive = true;

	bool	dynamic_focal_angle_transmit = true;
	bool	dynamic_focal_angle_receive = true;

	bool	calculate_phase_correction = false;
	bool	apply_phase_correction = false;

	bool	plane_wave;
	bool using_linear_array_probe;

	bool	option;

	physical_length	focal_depth_transmit = cm(1);
	physical_length	focal_depth_receive = cm(10);

	physical_angle	scanning_sector = degrees(90);
	physical_angle	half_scanning_sector = scanning_sector/2;

	physical_angle	fixed_angle_transmit = degrees(0);
	physical_angle	angle_receive = degrees(20);

	physical_angle dfi;
	physical_length	start_depth;
	physical_length	step_depth;

	physical_frequency f0 = MHz(3);
	physical_frequency estimated_f0;
	//double delta_phi = pi() / 2;
	physical_speed sound_speed = cm_sec(1.54e5);//1540000;//mm/s

	int control = 2;

	double iteration_time_step;
	double sector_angle_deg;

	int number_of_receive_elements;
	int number_of_transmit_elements;
	int number_of_frames;
	int current_frame_no;
	int fixed_ray_no;

	physical_length aperture_width;

	physical_length lambda = cm(sound_speed.cm_sec() / f0.Hz());
	physical_length normalized_lambda = lambda / half_scanning_sector.radians();
	
	physical_length pitch;

	RawSFDataSource &source_original = *data_source;
	shared_ptr<RawSFDataSourceTransEcho>	source_buffer;

	int	number_of_rays;
	int	samples;

	ComplexFunction2D_F32	img;

};


XRAD_END

#endif