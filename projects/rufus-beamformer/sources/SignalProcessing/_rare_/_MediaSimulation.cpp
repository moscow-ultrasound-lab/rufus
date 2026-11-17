#include "Pre.h"
#include "SimIOHeaderProcessor.h"
#include <XRADBasic/Sources/Utils/StatisticUtils.h>

#include "SignalParams.h"
#include "GreenFunction.h"
#include "PhasedProbe.h"
#include "ScatteredSignal.h"
#include "Volume3d.h"
#include "MediaSimulation.h"

static	double	radius(double_3 &p)
	{
	return sqrt(p.x*p.x + p.y*p.y + p.z*p.z);
	}

extern	char	this_process_comment[MAXSTRINGSIZE];

XRAD_BEGIN

void	MediaSimulator	:: ExportSignalParams()
	{
//	SetAngleUnits(DEGREES);
	//SetFrequencyUnits(M_HERZ);
//	SetDepthUnits(CENTIMETRES);

	signalParams :: soundSpeed = sound_speed.cm_sec();
	signalParams :: f0 = Options :: omega0.Hz();
	signalParams :: omega0 = Options :: omega0.rad_sec();//
	// выше несоответствие: в старые времена назвали
	// омегой то, что должно было быть ню или эф.
	// теперь, однако, менЯть поздно.
	
	signalParams ::	lambda = sound_speed.cm_sec()/signalParams :: f0;
	signalParams ::	tauI = 1/(PI*band_half_width.Hz()); // sec
	signalParams ::	sampleRate = sample_rate.Hz(); // Hz
	}


MediaSimulator	:: MediaSimulator():SectorData()
	{
//	SetAngleUnits(DEGREES);
//	SetFrequencyUnits(HERZ);
//	SetDepthUnits(CENTIMETRES);

//	задание базовых констант
	sound_speed = cm_sec(1.54e5);
//	Options :: omega0 = 3.e6;

	Options :: omega0 = MHz(GetFloating("Carrier frequency, MHz", 3., 1, 15));

	float	K = Options :: omega0.Hz()/3.;

	band_half_width = Options :: omega0/4;
	sample_rate = Options :: omega0*2;
	double	lambda = sound_speed.cm_sec()/Options :: omega0.Hz();

//	задание формальных комментариев
	strcpy(SIMIO::Process_Name,"Media simulator");
	sprintf(Object_Comment, "Simulated data");

	sprintf(SIMIO::Original_RF_Data_File_Name, "Simulated data. No input file");
	sprintf(SIMIO::Original_RF_Data_Comment, "Simulated data");
	
	sprintf(SIMIO::Original_RF_Data_File_Date, "Unknown date");
	
	sprintf(SIMIO::Data_Species, SUBAPERT_DATA_SPECIES);
	sprintf(SIMIO::Data_Genesis, SIM_GENESIS);
	sprintf(SIMIO::Process_Comment, "Simulating sector data for phased array");
	sprintf(SIMIO::Process_In_File_Name, Signal_IP_File_Name);
	sprintf(SIMIO::Process_Date_Time, SIMIO::Original_RF_Data_File_Date);
	sprintf(this_process_comment, SIMIO::Process_Comment);

//	определение параметров решетки
	n_elements = 100 * K;
	n_frames = 1;
	n_subaperture_elements_active = n_elements;
	n_subaperture_elements_full = n_elements;
	
	switch(GetButtonDecision("Array pitch:", 3,
		"lambda/2",
		"lambda",
		"lambda/10"))
		{
		case 0:
			array_Pitch = cm(lambda/2);
			break;
		case 1:
			array_Pitch = cm(lambda);
			n_elements /= 2;
			break;
		case 2:
			array_Pitch = cm(lambda/10);
			n_elements *= 5;
			break;
			}

//	определение сектора сканированиЯ
	physical_angle	a0, a1;
	physical_length	r0, r1;	

	switch(GetButtonDecision("Sector width", 3, "90", "45", "22.5"))
		{
		case 0:
			n_rays = 128;
			a0 = -degrees(45);
			a1 = degrees(45);
			break;
			
		case 1:
			n_rays = 64;
			a0 = -degrees(22.5);
			a1 = degrees(22.5);
			break;
			
		case 2:
			n_rays = 32;
			a0 = -degrees(11.25);
			a1 = degrees(11.25);
			break;
		default:
			FatalError("Unknown  sector parameter");
			break;	
		};

	if(YesOrNo("Double n_rays?", NO)) n_rays *= 2;

//	определение глубины сканированиЯ
	
//	SetDepthUnits(CENTIMETRES);
	double	dR = sound_speed.cm_sec()/(2*sample_rate.Hz());

	short	start_Sample;
	
	r0 = cm(GetFloating("Start depth", 0, 0, 10));
	r1 = cm(GetFloating("End depth", 10, r0.cm(), HUGE_VAL));
	start_Sample = r0.cm()/dR;
	n_samples = CeilFFTLength((r1-r0).cm()/dR);
	r1 = r0 + cm(n_samples*dR);
	
	r1 = cm(double(n_samples + start_Sample) * dR);
	r0 = cm(double(start_Sample) * dR);


	SetFrameSector(r0*(a1-a0).radians(), (r1-r0), a0, a1);

//	задание фокусного расстоЯниЯ
	TX_Focus.depth = RX_Focus.depth = cm(GetFloating("Focal point", (r_max() + r_min()).cm()/2, -HUGE_VAL, HUGE_VAL));

	printf("\nDepth range from %.1f to %.1f cm;", r_min().cm(), r_max().cm());
	fflush(stdout);
	ExportSignalParams();
	Pause();
	}

MediaSimulator	:: ~MediaSimulator()
	{
	}	
	
void	MediaSimulator :: InitWork()
	{
	SetOutputFileName("Simulated sector data", "");
	
	ISignal_Write();
	fflush(stdout);
	}

void	MediaSimulator :: EndWork()
	{
	Write_Data();
	Display("Simulated signal");
	}


void	MediaSimulator :: Batch()
	{
//	SetDepthUnits(CENTIMETRES);
	double	z0 = GetFloating("Center point depth", (r_max()+r_min()).cm()/2, r_min().cm(), HUGE_VAL);
	double	dR = depth_range().cm()/n_samples;
	
	emitter.InitProbes2D(n_elements*array_Pitch.cm(), array_Pitch.cm());
	if(Decide2("Scattering media configuration", "Multiple points", "One point", 0)) emitter.SetPointsConfiguration(one_point);
//	else SetPointsConfiguration(pattern_z_10);
	else emitter.SetPointsConfiguration(small_pattern_yz);
//	#pragma message "here used small pattern now"
	emitter.SetPointsZ(z0);
	

	emitter.txProbe.SetFocus(TX_Focus.depth.cm(), TX_Focus.depth.cm());
	emitter.rxProbe.SetFocus(RX_Focus.depth.cm(), RX_Focus.depth.cm());
	
	bool	smart_emitting = YesOrNo("Smart emitting?", YES);

	if(Decide2("Apodization function", "Constant", "Hamming", true))
		{
		/*if(smart_emitting) txProbe.SetApodization(constant_win, constant_win);
		else*/
		
		emitter.txProbe.SetApodization(hamming_win(), hamming_win());
		emitter.rxProbe.SetApodization(hamming_win(), hamming_win());
		}
	
	else
		{
		emitter.txProbe.SetApodization(constant_win(), constant_win());
		emitter.rxProbe.SetApodization(constant_win(), constant_win());
		}
	bool	dynamic_focus = YesOrNo("Dynamic focusing", NO);
	pulse_shape ps = Decide2("Choose pulse shape:", "Gauss", "Filtered rect", false) ? rect_pulse : gauss_pulse;
	
	greenFunctionTX gf_t;
	if(ps) gf_t.add_rect_pulse(signalParams :: omega0, 0, 0.5);
	else gf_t.add_gauss_pulse(signalParams :: omega0, 0, 0.5);
		
		
//	SetAngleUnits(RADIANS);
	double	x = 0;
	double	fi0 = start_angle().radians();
	double	dFi = (end_angle() - start_angle()).radians()/n_rays;
	
	int	i;
	double	relative_bandwidth = 1;
	
	if(smart_emitting)
		{
		int	n_focus_ranges = GetSigned("Number of focusing ranges", 5, 1, 15);
		relative_bandwidth = GetFloating("Relative bandwidth", 2./float(n_focus_ranges), 0.01, 1);

		for(i = 0; i < n_focus_ranges; i++)
			{
//			SetDepthUnits(CENTIMETRES);
			double	dF = depth_range().cm()/(n_focus_ranges+1);
			double	dW = signalParams::omega0/float(n_focus_ranges);
			double	F = r_min().cm() + float(i+1)*dF;
			double	W = signalParams::omega0 - float(i-n_focus_ranges/2)*dW;
			printf("\ni = %lu, F = %g, omega = %g (%g)", i, F, W/(2e6*PI), signalParams::omega0/(2e6*PI));
			fflush(stdout);
			emitter.AddFXPoint(F, W, relative_bandwidth, ps);
			}
		}
			
	StartProgress("Computing 2D image", n_rays);
	ComplexFunction2D_F32	buffer(n_rays, n_samples);

	for(i = 0; i < n_rays; i ++)
		{
		//SetAngleUnits(RADIANS);
		double y_angle = start_angle().radians() + i*dFi;
		double	sinFi = sin(y_angle);
		double	cosFi = cos(y_angle);
		
		//SetDepthUnits(CENTIMETRES);
		double	current_focus = TX_Focus.depth.cm();
		
		//txProbe.SetFocus(current_focus, current_focus);
		//rxProbe.SetFocus(current_focus, current_focus);
		
		if(smart_emitting) emitter.EmitSignalSmart(y_angle);
		else emitter.EmitSignal(y_angle, gf_t);

		emitter.rxProbe.SetAngles(0, y_angle);
		for(int sample = 0; sample < n_samples; sample++)
			{
			NextQMEvent();
			
			double r = dR*sample+r_min().cm();
			double z = r*cosFi;
			double	y = r*sinFi;
			
			double_3 target_point(x,y,z);
			if(dynamic_focus)
				{
				double	x_focus = RX_Focus.depth.cm();
				double y_focus = radius(target_point); // cm
				
				emitter.rxProbe.SetFocus(x_focus, y_focus);
				}
			buffer.at(i,sample) = emitter.ReceiveSignal(target_point);
//			CurrentRay[sample] = ReceiveSignal(target_point);
			}
		emitter.FrequencyDependentAttenuation(buffer[i], r_min().cm(), r_max().cm(), 0.5);
		if(smart_emitting)
			{
			//SetDepthUnits(CENTIMETRES);
			emitter.FilterSmart(buffer[i], r_min().cm(), r_max().cm(), relative_bandwidth);
			}
		NextProgress();
		}
	EndProgress();
	float	max = cabs(MaxValue(buffer));

	float	noise_db = -60;//10 разр. ацп
	float	noise = max*pow(10, noise_db/20);
	
	if(YesOrNo("Fixed noise?", YES)) noise = GetFloating("Fixed noise level", noise, 0, HUGE_VAL);
	
	for(i = 0; i < n_rays; i++)
		{
		for(int sample = 0; sample < n_samples; sample++)
			{
			buffer.at(i,sample) += polar(RandomUniform(noise), RandomUniform(two_pi()));
			}
		//SetDepthUnits(CENTIMETRES);
		emitter.DoAutoGain(buffer[i], r_min().cm(), r_max().cm());
		}
	for(int ray = 0; ray < n_rays; ++ray)
		{
		SetCurrentRay(ray);
		CurrentRay.CopyData(buffer[RayNo()]);
		}
	}
XRAD_END
