#include "Pre.h"
#include "SignalProcessing/SyntheticApertureFocuserSectorProbe.h"
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
#include "Verasonics\SyntheticApertureFocuserLinearProbe.h"
#include "UlaOp\SyntheticApertureFocuserDebug.h"

XRAD_BEGIN

struct CalibrationResult {
    double speed_of_sound;  // скорость звука (м/с)
    double pitch;           // шаг решетки (м)
    double confidence;      // уверенность (0-1)
};

struct PeakInfo {
    int tx;
    int rx;
    int sample_idx;
    float amp;
    float depth;
};

//template<class T>
void DisplayTrapezoid(ComplexFunction2D_F32& image, string title,
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

/*
ComplexFunction2D_F32	SyntheticApertureBeamformer::LinearProbeDASFocus(int first_sample, int last_sample, int n_lines, RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected, RawSFDataSource& current_source)
{
    
    printf("\n\n=== DAS BEAMFORMING (RECTANGLE for Linear Probe) ===\n");
    //number_of_rays = GetFloating("number_of_rays", MakeGUIValue(number_of_rays, saved_default_value), 1, 500);  
    printf("\n  ax_min = %d, n_samples = %d", first_sample, n_lines);
    printf("\n  n_tx = %d, n_rx = %d", number_of_transmit_elements, number_of_receive_elements);
    printf("\n  pitch = %.4f mm", pitch.cm() * 10);
    //printf("\n  speed = %.0f m/s", speed_cm_per_sec / 100);

    //printf("\n  speed = %.0f m/s", current_source.sound_speed.m_sec());
    printf("\n  speed = %.0f m/s", sound_speed.m_sec());
    
    printf("\n  rays = %d, samples = %d", number_of_rays, n_lines);
    fflush(stdout);
    double central_ray_position(0);
    double fixed_beam_x(0);
    double aperture_centre = 0.5 * (number_of_transmit_elements)*pitch.cm();
    if (dynamic_focal_angle_transmit)
    {
        central_ray_position = 0.5 * number_of_rays * pitch.cm();
        fixed_beam_x = (fixed_ray_no + 0.5) * pitch.cm() - central_ray_position;
        printf("\n=== BEAM FIX ENABLED ===\n");
        printf("  Fixed ray index: %d\n", fixed_ray_no);
        printf("  Fixed beam position: %.3f mm\n", fixed_beam_x * 10);
        printf("  Mode: Transmit beam fixed, receive scans all rays\n");
        printf("  This allows visualizing the beam shape and aberrations\n");
        fflush(stdout);
    }
    
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
                //auto	row = current_source.GetRow(transmit_element, receive_element);
                auto row = current_source.GetRow(transmit_element + current_frame_no * number_of_transmit_elements, receive_element);

                ComplexFunctionF32	buffer(image.hsize(), complexF32(0));
                for (int ray_no = 0; ray_no < number_of_rays; ++ray_no)
                {
                    auto    ray_position = (ray_no - number_of_rays / 2.0) * pitch.cm();
                    ray_position = dynamic_focal_angle_transmit ? ray_position : (ray_position + fixed_beam_x);
                    
                    double dx_tx = transmit_element_position - ray_position;
                    dx_tx = dynamic_focal_angle_transmit ? fixed_beam_x : dx_tx;

                    for (int line_no = 0; line_no < n_lines; ++line_no)
                    {
                        auto	CalculateInfluence = [&]()
                        {
                            double depth = 0.2 + 0.5 * line_no * (sound_speed.cm_sec() / current_source.raw_signal_sample_rate.Hz());
                            double dx_rx = receive_element_position - ray_position;
                            double transmit_path = sqrt(dx_tx * dx_tx + depth * depth);
                            double receive_path = sqrt(dx_rx * dx_rx + depth * depth);                            
                            auto	path = receive_path + transmit_path;
                            double t = path / sound_speed.cm_sec();
                            double sample_no = current_source.raw_signal_sample_rate.Hz() * t;
                            return row->in(sample_no - (int)current_source.first_raw_sample);
                        };
                        auto increment = CalculateInfluence();
                        buffer.at(line_no) = increment;
                    }
                    std::lock_guard<std::mutex>	lock(m);
                    image.row(ray_no) += buffer;
                    //if (YesOrNo("Display ray 0?", MakeGUIValue(false, saved_default_value))) { DisplayMathFunction(image.row(0), 0, 1, "image.row(0)"); }
                }
            }
            ++progress;
        }
    }
    progress.end();
    printf("\n=== DAS test complete ===\n");
    return image;
}
*/

ComplexFunction2D_F32 SyntheticApertureBeamformer::PseudoConvexLinearProbeDASFocus(int first_sample, int last_sample, int n_lines, RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected, RawSFDataSource& current_source)
{
    //number_of_rays = 128;
    printf("\n\n=== PSEUDO-CONVEX DAS BEAMFORMING ===");
    printf("\n  first_sample = %d, last_sample = %d, n_lines = %d", first_sample, last_sample, n_lines);
    printf("\n  number_of_rays = %d", number_of_rays);

    RealFunction2D_F64 matrix_correction_new = RearrangeMatrixCorrection(matrix_correction, rays_corrected, number_of_rays);

    double sector_angle_rad = sector_angle_deg * pi() / 180.0;
    printf("\n  sector_angle_rad = %.3f", sector_angle_rad);

    double aperture_centre = 0.5 * number_of_transmit_elements * pitch.cm();
    printf("\n  aperture_centre = %.3f", aperture_centre);

    ComplexFunction2D_F32 image(number_of_rays, n_lines, complexF32(0));
    double speed_cm_per_sec = source_original.sound_speed.m_sec() * 100;
    if (CapsLock()) 
    {
        speed_cm_per_sec = sound_speed.cm_sec();
    }
    printf("\n  sound_speed = %.0f m/s", speed_cm_per_sec/100);

    double sample_rate_hz = current_source.raw_signal_sample_rate.Hz();

    GUIProgressBar progress;
    std::mutex m;
    progress.start("Pseudo-Convex DAS Beamforming", number_of_transmit_elements);
    for (int transmit_element_no = 0; transmit_element_no < number_of_transmit_elements; ++transmit_element_no) 
    {
        double transmit_element_position = (transmit_element_no + 0.5) * pitch.cm() - aperture_centre;

#pragma omp parallel for schedule (guided)
        for (int receive_element_no = 0; receive_element_no < number_of_receive_elements; ++receive_element_no) 
        {
            double receive_element_position = (receive_element_no + 0.5) * pitch.cm() - aperture_centre;
            auto row = current_source.GetRow(transmit_element_no + current_frame_no * number_of_transmit_elements, receive_element_no);

            ComplexFunctionF32 buffer(n_lines, complexF32(0));

            for (int ray_no = 0; ray_no < number_of_rays; ++ray_no) 
            {
                int tx_ray_no = dynamic_focal_angle_transmit ? ray_no : fixed_ray_no;

                double t_tx = (tx_ray_no - number_of_rays / 2.0) / (number_of_rays / 2.0);
                double t_rx = (ray_no - number_of_rays / 2.0) / (number_of_rays / 2.0);
                

                double transmit_angle = t_tx * sector_angle_rad / 2.0;
                double receive_angle = t_rx * sector_angle_rad / 2.0;
                receive_angle = dynamic_focal_angle_transmit ? receive_angle : (receive_angle + transmit_angle);

                double ray_start = (ray_no - number_of_rays / 2.0) * pitch.cm();

                for (int line_no = 0; line_no < n_lines; ++line_no) 
                {
                    auto CalculateInfluence = [&]() 
                    {                       
                        int		raw_sample_no = first_sample + line_no * ((last_sample - first_sample) / n_lines);
                        auto	depth = start_depth + 0.5 * raw_sample_no * step_depth;

                        double transmit_point_x = ray_start + depth.cm() * tan(transmit_angle);
                        double receive_point_x = ray_start + depth.cm() * tan(receive_angle);

                        double dx_tx = transmit_element_position - transmit_point_x;
                        double dx_rx = receive_element_position - receive_point_x;

                        double transmit_path = sqrt(dx_tx * dx_tx + depth.cm() * depth.cm());
                        double receive_path = sqrt(dx_rx * dx_rx + depth.cm() * depth.cm());

                        double path = receive_path + transmit_path;
                        double t = path / speed_cm_per_sec;

                        t += matrix_correction_new.at(ray_no, transmit_element_no) + matrix_correction_new.at(ray_no, receive_element_no);

                        double sample_no = sample_rate_hz * t;
                        return row->in(sample_no - (int)current_source.first_raw_sample);
                    };

                    buffer.at(line_no) = CalculateInfluence();
                }

                std::lock_guard<std::mutex> lock(m);
                image.row(ray_no) += buffer;
            }
        }
        ++progress;
    }
    progress.end();
    printf("\n=== Pseudo-Convex DAS complete ===\n");
    return image;
}


/*
ComplexFunction2D_F32 SyntheticApertureBeamformer::PseudoConvexDASFocus(int first_sample, int last_sample, int n_lines, RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected, RawSFDataSource& current_source)
{
    printf("\n\n=== PSEUDO-CONVEX DAS BEAMFORMING ===\n");

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

    for (int j = 0; j < n_lines; ++j)
    {
        depth_cm[j] = 0.2 + 0.5 * j * (speed_cm_per_sec / sample_rate_hz);
    }

    for (int transmit_element = 0; transmit_element < number_of_transmit_elements; ++transmit_element)
    {
        double transmit_element_position = (transmit_element + 0.5) * pitch.cm() - aperture_centre;

#pragma omp parallel for schedule (guided)
        for (int receive_element = 0; receive_element < number_of_receive_elements; ++receive_element)
        {
            double receive_element_position = (receive_element + 0.5) * pitch.cm() - aperture_centre;
            auto row = current_source.GetRow(transmit_element + current_frame_no * number_of_transmit_elements, receive_element);

            ComplexFunctionF32 buffer(n_lines, complexF32(0));

            for (int ray_no = 0; ray_no < number_of_rays; ++ray_no)
            {
                double angle = ray_angle_rad[ray_no];
                double tan_angle = tan(angle);
                double ray_start = ray_start_x[ray_no];

                for (int line_no = 0; line_no < n_lines; ++line_no)
                {
                    auto CalculateInfluence = [&]()
                    {
                        double depth = depth_cm[line_no];

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

                    buffer.at(line_no) = CalculateInfluence();
                }

                std::lock_guard<std::mutex> lock(m);
                image.row(ray_no) += buffer;
            }
        }
        ++progress;
    }
    progress.end();

    printf("\n=== Pseudo-Convex DAS complete ===\n");
    return image;
}
*/


CalibrationResult CalibrateFromSA(RawSFDataSource& source, int tx_start, int tx_end, int rx_start, int rx_end, int sample_start, int sample_end)
{
    CalibrationResult result;
    result.speed_of_sound = 1540.0;  // начальное приближение
    result.pitch = source.array_pitch.mm() / 1000.0;
    result.confidence = 0.0;

    // 1. Собираем пики
    std::vector<PeakInfo> peaks;

    for (int tx = tx_start; tx < tx_end; ++tx) 
    {
        for (int rx = rx_start; rx < rx_end; ++rx) 
        {
            auto row = source.GetRow(tx, rx);
            if (!row) continue;

            // Ищем пик в заданном диапазоне
            float max_amp = 0;
            int max_idx = sample_start;
            for (int s = sample_start; s < sample_end; ++s) 
            {
                float amp = cabs(row->in(s));
                if (amp > max_amp) {
                    max_amp = amp;
                    max_idx = s;
                }
            }

            if (max_amp > 0.01) {
                PeakInfo info;
                info.tx = tx;
                info.rx = rx;
                info.sample_idx = max_idx;
                info.amp = max_amp;
                peaks.push_back(info);
            }
        }
    }

    printf("Found %zu peaks\n", peaks.size());
    if (peaks.size() < 10) {
        printf("Not enough peaks for calibration\n");
        return result;
    }

    // 2. По пикам определяем глубину и позицию
    // Берем центральный элемент как референс
    int center_tx = (tx_start + tx_end) / 2;
    int center_rx = (rx_start + rx_end) / 2;

    // Находим пик для центрального элемента
    const PeakInfo* center_peak = nullptr;
    for (const auto& p : peaks) {
        if (p.tx == center_tx && p.rx == center_rx) {
            center_peak = &p;
            break;
        }
    }

    if (!center_peak) {
        printf("No peak found for center element\n");
        return result;
    }

    double center_sample = center_peak->sample_idx;
    double fs = source.raw_signal_sample_rate.Hz();
    double dt = 1.0 / fs;

    // 3. Для каждого элемента вычисляем скорость звука
    std::vector<double> speeds;
    std::vector<double> pitches;

    double aperture_center = 0.5 * (tx_start + tx_end) * result.pitch;

    for (const auto& p : peaks) {
        if (p.tx == p.rx) continue;  // используем только разные элементы

        // Позиция элемента
        double x_tx = (p.tx + 0.5) * result.pitch - aperture_center;
        double x_rx = (p.rx + 0.5) * result.pitch - aperture_center;

        // Глубина из центрального пика (приблизительно)
        double depth = center_sample * dt * result.speed_of_sound / 2.0;

        // Позиция цели (предполагаем на оси)
        double x_target = 0;

        // Расстояния
        double r_tx = sqrt(pow(x_tx - x_target, 2) + depth * depth);
        double r_rx = sqrt(pow(x_rx - x_target, 2) + depth * depth);
        double total_path = r_tx + r_rx;

        // Время и скорость
        double t = p.sample_idx * dt;
        double c = total_path / t;

        if (c > 1000 && c < 3000) {  // разумный диапазон
            speeds.push_back(c);
        }
    }

    if (speeds.empty()) {
        printf("No valid speeds calculated\n");
        return result;
    }

    // 4. Статистика
    double sum = 0;
    for (double s : speeds) sum += s;
    result.speed_of_sound = sum / speeds.size();

    double var = 0;
    for (double s : speeds) var += pow(s - result.speed_of_sound, 2);
    double std_dev = sqrt(var / speeds.size());

    result.confidence = 1.0 - std_dev / result.speed_of_sound;

    printf("\n=== Calibration Results ===\n");
    printf("  Number of measurements: %zu\n", speeds.size());
    printf("  Speed of sound: %.1f m/s\n", result.speed_of_sound);
    printf("  Std deviation: %.1f m/s (%.1f%%)\n", std_dev, std_dev / result.speed_of_sound * 100);
    printf("  Confidence: %.2f\n", result.confidence);

    return result;
}

void SyntheticApertureBeamformer::DisplayFrameLinearProbe(RealFunction2D_F64 matrix_correction, RealFunctionF32 rays_corrected, string title)
{
    physical_length pitch_original(pitch);
    physical_speed sound_speed_original(sound_speed);
    //number_of_rays = number_of_receive_elements;

   // if (YesOrNo("Display frame?", MakeGUIValue(false, saved_default_value)))
    bool cycle_flag(true);
    while(cycle_flag)
    {
        
        if (!YesOrNo("Dynamic focusing for transmit?", MakeGUIValue(true, saved_default_value)))
        {  
            dynamic_focal_angle_transmit = false;
            fixed_ray_no = GetSigned("Enter ray index to fix", MakeGUIValue(number_of_rays/2, saved_default_value), 0, number_of_rays - 1);
        }
        else 
        {
            dynamic_focal_angle_transmit = true;
        }
        

        int first_sample = source_original.first_raw_sample;
        int last_sample = source_original.samples_per_element;
        
        if (CapsLock() || (source_original.samples_per_element > 3000))
        {
            if (YesOrNo("Cut deeper part of the frame?", MakeGUIValue(false, saved_default_value)))
            {
                last_sample = GetFloating("Choose last sample number", MakeGUIValue(1.0, saved_default_value), first_sample, source_original.samples_per_element);
            }
        }

        if (CapsLock())
        {
            if (YesOrNo("Adjust pitch and SoS?", true))
            {
                printf("\n pitch=%g", pitch_original);
                pitch = pitch_original * GetFloating("Choose pitch multiplier (pitch*coef)", MakeGUIValue(1.0, saved_default_value), 0, 2);
                printf("\n New pitch=%g mm", pitch.mm());

                printf("\n SoS=%g", sound_speed_original);
                sound_speed = sound_speed_original * GetFloating("Choose multiplier (SoS*coef)", MakeGUIValue(1.0, saved_default_value), 0, 5);
                printf("\n New SoS=%g m/s", sound_speed.m_sec());
            }
        }

        int n_lines(last_sample - first_sample);

        ComplexFunction2D_F32 image;

        physical_length scan_width = cm(pitch.cm() * number_of_rays);
        physical_length scan_depth = cm(0.5 * source_original.samples_per_element * (source_original.sound_speed.cm_sec() / source_original.raw_signal_sample_rate.Hz()));
        ScanConverterOptions sco(ScanFrameRectangle(scan_width, scan_depth));

        if (CapsLock())
        {
            if (YesOrNo("Enter Wavefront Debug Mode?", MakeGUIValue(false, saved_default_value)))
            {
                SyntheticApertureFocuserDebug::InteractiveDebug(source_original);
            }
        }

      //  if (YesOrNo("Is Pseudo Convex Active?", MakeGUIValue(false, saved_default_value)))
       // {
            sector_angle_deg = GetFloating("Define Pseudo Convex Opening Angle (degrees)", MakeGUIValue(15, saved_default_value), 0, 90);
            image = PseudoConvexLinearProbeDASFocus(first_sample, last_sample, n_lines, matrix_correction, rays_corrected, source_original);
            DisplayMathFunction2D(image, "Frame before Scanconversion");
            DisplayTrapezoid(image, title, scan_width, scan_depth, sector_angle_deg);
       // }
     //   else
      //  {
       //     image = LinearProbeDASFocus(first_sample, last_sample, n_lines, matrix_correction, rays_corrected, source_original);
       //     DisplayMathFunction2D(image, "Rectangle Frame", sco);
      //  }  
            if (CapsLock()) cycle_flag = true;
            else cycle_flag = false;
    }

}

XRAD_END