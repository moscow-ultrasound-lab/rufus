// SyntheticApertureFocuserVerasonics.cpp
#include "Pre.h"
#include "SignalProcessing/SyntheticApertureFocuserAina.h"
#include "SignalProcessing/SyntheticApertureFocuserAnalyzer.h"
#include "SignalProcessing/BeamformerTesting.h"
#include <Attic/CalculateEntropy.h>
#include <XRADBasic/FFT2D.h>
#include <XRADBasic/Sources/Utils/LeastSquares.h>
#include "SignalProcessing/DirectPhaseEstimation.h"
#include "SignalProcessing/PolynomialNarrowing.h"
#include "SignalProcessing/RawSFDataSourceTransEcho.h"
#include "SignalProcessing/RawSFDataSourceSimIO.h"
#include "SignalProcessing/AberrationsCorrector.h"
#include "BBasics\RiceBasics.h"
#include "Verasonics\RawSFDataSourceVerasonics.h"
#include "Verasonics\SyntheticApertureFocuserVerasonics.h"
//#include "ScanConverter\TrapezoidalScanConverter.h"

XRAD_BEGIN



//template<class T>
void DisplayTrapezoid(ComplexFunction2D_F32& image, const char* title,
    physical_length scan_width,
    physical_length scan_depth,
    double angle_deg) // полуугол
{

    image.transpose();

    double start_rad = -angle_deg * pi() / 180.0;
    double end_rad = angle_deg * pi() / 180.0;
    double angle_range = end_rad - start_rad;

    // Используем тот же тип, что и исходное изображение
    size_t rows = (size_t)(scan_depth.cm() * 30);  // pixels_per_cm = 30
    size_t cols = (size_t)(scan_width.cm() * 30 * 1.5);

    // Создаем выходное изображение с комплексными значениями
    ComplexFunction2D_F32 out(rows, cols, complexF32(0, 0));

    double center = cols / 2.0;
    double apex = scan_width.cm() * 30 / (2 * tan(angle_range / 2));

    for (size_t r = 0; r < rows; r++) {
        double y = r / 30.0;
        double y_ratio = y / scan_depth.cm();
        double half_width = (scan_width.cm() / 2) * (1 + y_ratio * tan(angle_range / 2));

        for (size_t c = 0; c < cols; c++) {
            double x = (c - center) / 30.0;
            if (fabs(x) > half_width) continue;

            double angle = atan2(x, y + apex / 30);
            angle = max(start_rad, min(end_rad, angle));

            double src_ray = ((angle - start_rad) / angle_range) * (image.hsize() - 1);
            double src_sample = (y / cos(angle) / scan_depth.cm()) * (image.vsize() - 1);

            if (src_ray >= 0 && src_ray < image.hsize() - 1 &&
               src_sample >= 0 && src_sample < image.vsize() - 1) {



                int r0 = (int)src_sample, r1 = r0 + 1;
                int c0 = (int)src_ray, c1 = c0 + 1;
                double dr = src_sample - r0, dc = src_ray - c0;

                // РАБОТАЕМ С КОМПЛЕКСНЫМИ ЧИСЛАМИ ПРАВИЛЬНО
                complexF32 v00 = image.at(r0, c0);
                complexF32 v10 = image.at(r1, c0);
                complexF32 v01 = image.at(r0, c1);
                complexF32 v11 = image.at(r1, c1);



                // Билинейная интерполяция комплексных чисел
                complexF32 v0 = v00 * (1.0 - dr) + v10 * dr;
                complexF32 v1 = v01 * (1.0 - dr) + v11 * dr;
                complexF32 result = v0 * (1.0 - dc) + v1 * dc;

                out.at(r, c) = result;
            }
        }
    }

    // Отображаем амплитуду (модуль комплексного числа)
    // Или можно отобразить реальную/мнимую часть
    ComplexFunction2D_F32 out_amplitude(out.vsize(), out.hsize());
    for (size_t r = 0; r < out.vsize(); ++r) {
        for (size_t c = 0; c < out.hsize(); ++c) {
            out_amplitude.at(r, c) = cabs(out.at(r, c));  // модуль для отображения
        }
    }

    DisplayMathFunction2D(out_amplitude, title);
}



ComplexFunction2D_F32	SyntheticApertureFocuserAina::LinearDASFocus(int first_sample, int last_sample, int n_lines, RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected, RawSFDataSource& current_source)
{
    printf("\n\n=== TESTING DAS BEAMFORMING (RECTANGLE for L11-5V Carotid) ===\n");
    printf("\nParameters (Carotid L11-5V):");
    printf("\n  ax_min = %d, n_samples = %d", first_sample, n_lines);
    printf("\n  n_tx = %d, n_rx = %d", number_of_transmit_elements, number_of_receive_elements);
    printf("\n  pitch = %.4f mm", pitch.cm() * 10);
    //printf("\n  speed = %.0f m/s", speed_cm_per_sec / 100);
    printf("\n  speed = %.0f m/s", current_source.sound_speed.m_sec());
    printf("\n  rays = %d, samples = %d", number_of_rays, n_lines);
    fflush(stdout);
    double aperture_centre = 0.5 * (number_of_transmit_elements)*pitch.cm();
    ComplexFunction2D_F32 image(number_of_rays, n_lines, complexF32(0));
    GUIProgressBar progress;
    std::mutex	m;
    progress.start("DAS Beamforming (Rectangle)", number_of_transmit_elements);
    {
        for (int transmit_element = 0; transmit_element < number_of_transmit_elements; ++transmit_element) 
        {
            auto transmit_element_position = (transmit_element + 0.5) * pitch.cm() - aperture_centre;

#pragma omp parallel for schedule (guided)
            for (int receive_element = 0; receive_element < number_of_receive_elements; ++receive_element) 
            {
                auto receive_element_position = (receive_element + 0.5) * pitch.cm() - aperture_centre;
                auto row = current_source.GetRow(transmit_element + current_frame_no * number_of_transmit_elements, receive_element);
            
                ComplexFunctionF32	buffer(image.hsize(), complexF32(0));
                for (int i = 0; i < number_of_rays; ++i)
                {
                    auto    ray_x = (i - number_of_rays / 2.0) * pitch.cm();
                    double dx_tx = transmit_element_position - ray_x;

                    for (int j = 0; j < n_lines; ++j)
                    {
                        auto	CalculateInfluence = [&]()
                        {
                            double depth = 0.2 + 0.5 * j * (current_source.sound_speed.cm_sec() / current_source.raw_signal_sample_rate.Hz());
                            double dx_rx = receive_element_position - ray_x;
                            double transmit_path = sqrt(dx_tx * dx_tx + depth * depth);
                            double receive_path = sqrt(dx_rx * dx_rx + depth * depth);
                            auto	path = receive_path + transmit_path;
                            double t = path / current_source.sound_speed.cm_sec();
                            double sample_no = current_source.raw_signal_sample_rate.Hz() * t;
                            return row->in(sample_no - (int)current_source.first_raw_sample);
                        };
                        auto increment = CalculateInfluence();
                        buffer.at(j) = increment;
                    }
                    std::lock_guard<std::mutex>	lock(m);
                    image.row(i) += buffer;
                }
            }
            ++progress;
        }
    }
    progress.end();
    printf("\n=== DAS test complete ===\n");
    return image;
}


ComplexFunction2D_F32 SyntheticApertureFocuserAina::PseudoConvexDASFocus(
    int first_sample, int last_sample, int n_lines,
    RealFunction2D_F64 matrix_correction,
    RealFunctionF32 rays_corrected,
    RawSFDataSource& current_source)
{
    printf("\n\n=== PSEUDO-CONVEX DAS BEAMFORMING for L11-5V Carotid ===\n");

    // Угол раскрытия (чем больше угол, тем шире трапеция)
    double sector_angle_rad = sector_angle_deg * pi() / 180.0;

    printf("\nParameters (Carotid L11-5V Pseudo-Convex):");
    printf("\n  Opening angle = %.1f deg", sector_angle_deg);
    printf("\n  first_sample = %d, n_lines = %d", first_sample, n_lines);
    printf("\n  n_tx = %d, n_rx = %d", number_of_transmit_elements, number_of_receive_elements);
    printf("\n  pitch = %.4f mm", pitch.cm() * 10);
    printf("\n  speed = %.0f m/s", current_source.sound_speed.m_sec());
    printf("\n  rays = %d, samples = %d", number_of_rays, n_lines);
    fflush(stdout);

    double aperture_centre = 0.5 * number_of_transmit_elements * pitch.cm();
    ComplexFunction2D_F32 image(number_of_rays, n_lines, complexF32(0));
    GUIProgressBar progress;
    std::mutex m;
    progress.start("Pseudo-Convex DAS Beamforming", number_of_transmit_elements);

    // Предвычисляем углы лучей
    std::vector<double> ray_angle_rad(number_of_rays);
    std::vector<double> ray_start_x(number_of_rays);

    for (int i = 0; i < number_of_rays; ++i) {
        // Начало луча на апертуре
        ray_start_x[i] = (i - number_of_rays / 2.0) * pitch.cm();
        // Угол луча: максимальный у крайних лучей, 0 у центрального
        double t = (i - number_of_rays / 2.0) / (number_of_rays / 2.0);
        ray_angle_rad[i] = t * sector_angle_rad / 2.0;
    }

    // Предвычисляем глубины
    std::vector<double> depth_cm(n_lines);
    double speed_cm_per_sec = current_source.sound_speed.m_sec() * 100;
    double sample_rate_hz = current_source.raw_signal_sample_rate.Hz();

    for (int j = 0; j < n_lines; ++j) {
        depth_cm[j] = 0.2 + 0.5 * j * (speed_cm_per_sec / sample_rate_hz);
    }

    for (int transmit_element = 0; transmit_element < number_of_transmit_elements; ++transmit_element) {
        double transmit_element_position = (transmit_element + 0.5) * pitch.cm() - aperture_centre;

#pragma omp parallel for schedule (guided)
        for (int receive_element = 0; receive_element < number_of_receive_elements; ++receive_element) {
            double receive_element_position = (receive_element + 0.5) * pitch.cm() - aperture_centre;
            auto row = current_source.GetRow(transmit_element + current_frame_no * number_of_transmit_elements, receive_element);

            ComplexFunctionF32 buffer(n_lines, complexF32(0));

            for (int i = 0; i < number_of_rays; ++i) {
                double angle = ray_angle_rad[i];
                double tan_angle = tan(angle);
                double ray_start = ray_start_x[i];

                for (int j = 0; j < n_lines; ++j) {
                    auto CalculateInfluence = [&]() {
                        double depth = depth_cm[j];

                        // КЛЮЧЕВОЕ ОТЛИЧИЕ: точка смещается с глубиной
                        double point_x = ray_start + depth * tan_angle;

                        double dx_tx = transmit_element_position - point_x;
                        double dx_rx = receive_element_position - point_x;

                        double transmit_path = sqrt(dx_tx * dx_tx + depth * depth);
                        double receive_path = sqrt(dx_rx * dx_rx + depth * depth);

                        double path = receive_path + transmit_path;
                        double t = path / speed_cm_per_sec;
                        double sample_no = sample_rate_hz * t;

                        return row->in(sample_no - (int)current_source.first_raw_sample);
                    };

                    buffer.at(j) = CalculateInfluence();
                }

                std::lock_guard<std::mutex> lock(m);
                image.row(i) += buffer;
            }
        }
        ++progress;
    }
    progress.end();

    printf("\n=== Pseudo-Convex DAS complete ===\n");
    return image;
}

void SyntheticApertureFocuserAina::BatchVerasonics(RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected)
{
    int first_sample = source_original.first_raw_sample;
    int last_sample = source_original.samples_per_element;
    int n_lines(last_sample - first_sample);
    

    printf("\n pitch=%g", pitch);
    pitch = pitch * GetFloating("Choose pitch multiplier (pitch*coef)", MakeGUIValue(1.0, saved_default_value), 0, 2);
    printf("\n New pitch=%g", pitch);

    ComplexFunction2D_F32 image;

    physical_length scan_width = cm(pitch.cm() * number_of_rays);
    physical_length scan_depth = cm(0.5 * source_original.samples_per_element * (source_original.sound_speed.cm_sec() / source_original.raw_signal_sample_rate.Hz()));
    ScanConverterOptions sco(ScanFrameRectangle(scan_width, scan_depth));

    if (YesOrNo("Is Pseudo Convex Active?", MakeGUIValue(false, saved_default_value)))
    {
        sector_angle_deg = GetFloating("Opening angle (degrees)", MakeGUIValue(15.0, saved_default_value), 0, 45.0);
        image = PseudoConvexDASFocus(first_sample, last_sample, n_lines, matrix_correction, rays_corrected, source_original);
        DisplayTrapezoid(image, "Trapezoid Frame", scan_width, scan_depth, sector_angle_deg);
    }
    else
    {
        image = LinearDASFocus(first_sample, last_sample, n_lines, matrix_correction, rays_corrected, source_original);
        DisplayMathFunction2D(image, "Rectangle Frame", sco);
    }     

}

XRAD_END