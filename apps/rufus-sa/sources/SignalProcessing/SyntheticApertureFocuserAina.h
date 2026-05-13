#ifndef	__focuser_aina_h
#define	__focuser_aina_h

#include "SyntheticFocusingOptions.h"
#include "LUT_8bit_interpolator.h"
#include "SyntheticApertureFocuser.h"
#include "RawSFDataSourceTransEcho.h"
#include "DirectPhaseEstimation.h"
#include "PolynomialNarrowing.h"
//#include "ApertureFocusingOptionsOld.h"

XRAD_BEGIN

//#error continue from here

double			DisplayLines(GraphSet &GS, ComplexFunction2D_F64 lines, size_t number_of_rays, size_t n_lines, size_t iteration_no, physical_angle scanning_sector);
RealFunctionF64 CalcLine(ComplexFunction2D_F64 lines, size_t number_of_rays, size_t n_lines);
void			SubstractLinearShift(RealFunctionF64 &phase_interpolated);



class	SyntheticApertureFocuserAina : public SyntheticApertureFocuser
{
	PARENT(SyntheticFocusingOptions);
//	void	AllocateRawSFData();
//	void	DisplayRawData();

protected:
	virtual void					GetAreaForCorrection(CorrectionTraits &traits);
	virtual	void					Batch();
	virtual	void					ChooseOption(RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected);
	virtual	void					InitWork();
	virtual	void					Sharpness(RealFunction2D_F64 &matrix_correction, RealFunctionF32 &rays_corrected);
	virtual	void					DisplayFrame(RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected, string title);
	virtual	void					CalculateFrameRice(RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected, string title);
	virtual void					CalibrationSourceMethod(RealFunction2D_F64 &matrix_correction, RealFunction2D_F64 &best_fit_matrix_correction, RealFunctionF32 &rays_corrected, physical_angle half_scanning_sector);

	virtual	ComplexFunction2D_F32	Focus(int first_sample, int last_sample, int n_lines, RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected, RawSFDataSource &current_source);
	virtual	RealFunctionF64			GaussFit(double &best_fit_sq_error, RealFunctionF64 line, size_t ray_no);
	virtual	RealFunctionF64			GetLine(GraphSet &GS, int first_sample, int last_sample, int n_lines, RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected);

	virtual	void					Correction(CorrectionTraits &traits, int j, double min);
	
public:
	virtual void					DirectPhaseEstimation(CorrectionTraits &traits);
	
	virtual	void					DirectPhaseEstimationFullFrame(CorrectionTraits &traits);
	//virtual	void					DirectPhaseEstimationFullFrame(RealFunctionF32 &rays_corrected, physical_angle half_scanning_sector, physical_angle scanning_sector, physical_angle fixed_angle_transmit, size_t number_of_rays, GUIProgressBar &prog, size_t ray_no, RealFunctionF32 &ray_to_correct, GraphSet &errorGS, GraphSet &approximationGS, GraphSet &phaseGS, GraphSet &lineGS, int iteration_number, int first_sample_to_correct, int last_sample_to_correct, int n_lines, RealFunction2D_F64 &matrix_correction, RealFunction2D_F64 &best_fit_matrix_correction, RealFunctionF32 &best_result);
	
	
	virtual void PolynomialNarrowing(CorrectionTraits &traits);
	//virtual void PolynomialNarrowing(RealFunctionF32 &rays_corrected, RealFunction2D_F64 &matrix_correction, RealFunction2D_F64 &best_fit_matrix_correction, GraphSet &errorGS, GraphSet &approximationGS, GraphSet &lineGS,	int first_sample_to_correct, int last_sample_to_correct, int n_lines, RealFunctionF64 &min_error, GUIProgressBar &prog);


	bool	dynamic_focal_depth_transmit = true;
	bool	dynamic_focal_depth_receive = true;

	bool	dynamic_focal_angle_transmit = true;
	bool	dynamic_focal_angle_receive = true;

	bool	calculate_phase_correction = false;
	bool	apply_phase_correction = false;

	bool	plane_wave;

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
	//int iteration_number;
//	int	number_of_iterations;

	double iteration_time_step;

	int number_of_receive_elements;
	int number_of_transmit_elements;
	int number_of_frames;
	int current_frame_no;

	physical_length aperture_width;

	physical_length lambda = cm(sound_speed.cm_sec() / f0.Hz());
	physical_length normalized_lambda = lambda / half_scanning_sector.radians();
	
	physical_length pitch;

	RawSFDataSource &source_original = *data_source;
	shared_ptr<RawSFDataSourceTransEcho>	source_buffer;

	int	number_of_rays;
	int	samples;

	ComplexFunction2D_F32	img;


public:
	
	
//	SyntheticApertureFocuserAina();
//	virtual ~SyntheticApertureFocuserAina();
};


XRAD_END

#endif